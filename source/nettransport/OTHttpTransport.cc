/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#include "opentelepresence/nettransport/OTHttpTransport.h"
#include "opentelepresence/OTEngine.h"
#include "opentelepresence/OTWrap.h"
#include "opentelepresence/jsoncpp/json.h"

#include "tsk_string.h"
#include "tsk_memory.h"

#include "tinyhttp.h"
#if OPENTELEPRESENCE_UNDER_WINDOWS
#include <direct.h> /* mkdir() */
#endif
#include <sys/stat.h> /* stat() */
#include <assert.h>

/* min size of a stream chunck to form a valid HTTP message */
#define kStreamChunckMinSize 0x32
#define kHttpMethodOptions "OPTIONS"
#define kHttpMethodPost "POST"

#define kOTHttpResultCode_Provisional		100
#define kOTHttpResultCode_Success			200
#define kOTHttpResultCode_Unauthorized		403
#define kOTHttpResultCode_NotFound			404
#define kOTHttpResultCode_ParsingFailed		420
#define kOTHttpResultCode_InvalidDataType	483
#define kOTHttpResultCode_InvalidData		450
#define kOTHttpResultCode_InternalError		603

#define kOTHttpResultPhrase_Success					"OK"
#define kOTHttpResultPhrase_Unauthorized			"Unauthorized"
#define kOTHttpResultPhrase_NotFound				"Not Found"
#define kOTHttpResultPhrase_ParsingFailed			"Parsing failed"
#define kOTHttpResultPhrase_InvalidDataType			"Invalid data type"
#define kOTHttpResultPhrase_FailedToCreateLocalFile	"Failed to create local file"
#define kOTHttpResultPhrase_FailedTransferPending	"Failed transfer pending"
#define kOTHttpResultPhrase_InternalError			"Internal Error"

#define kJsonField_Action "action"
#define kJsonField_Name "name"
#define kJsonField_Type "type"
#define kJsonField_Size "size"
#define kJsonField_BridgeId "bridge_id"
#define kJsonField_BridgePin "bridge_pin"
#define kJsonField_UserId "user_id"

#define kJsonValue_ActionReq_UploadPresentation "req_presentation_upload"

#ifdef _MSC_VER
#	define mkdir _mkdir
#endif
#if OPENTELEPRESENCE_UNDER_WINDOWS
#	define OTMkdir(path) mkdir((path))
#else
#	define OTMkdir(path) mkdir((path), 777)
#endif


// Access-Control-Allow-Headers, wildcard not allowed: http://www.w3.org/TR/cors/#access-control-allow-headers-response-header
// Connection should not be "Keep-Alive" to avoid ghost sockets (do not let the browser control when the socket have to be closed)
#define kHttpOptionsResponse \
	"HTTP/1.1 200 OK\r\n" \
	"Server: TelePresence Server " OT_VERSION_STRING "\r\n" \
	"Access-Control-Allow-Origin: *\r\n" \
	"Access-Control-Allow-Headers: Content-Type,Connection,File-Description\r\n" \
	"Access-Control-Allow-Methods: POST, GET, OPTIONS\r\n" \
	"Content-Length: 0\r\n" \
	"Content-Type: application/json\r\n" \
	"Connection: keep-alive\r\n" \
	"\r\n"
#define kHttpResponse(code, phrase) \
	"HTTP/1.1 " #code " " phrase "\r\n" \
	"Server: TelePresence Server " OT_VERSION_STRING "\r\n" \
	"Access-Control-Allow-Origin: *\r\n" \
	"Content-Length: 0\r\n" \
	"Connection: Close\r\n" \
	"\r\n"

static OT_INLINE bool folderExists(const char* folder);
static OT_INLINE bool isJsonContentType(const thttp_message_t *pcHttpMessage);
static OT_INLINE bool isFileContentType(const thttp_message_t *pcHttpMessage);
static const char* getHttpContentType(const thttp_message_t *pcHttpMessage);

#define OT_JSON_GET(fieldParent, fieldVarName, fieldName, typeTestFun, couldBeNull) \
	if(!fieldParent.isObject()){ \
		OT_DEBUG_ERROR_EX(kOTMobuleNameHttpTransport, "JSON '%s' not an object", (fieldName)); \
		return new OTHttpResult(kOTHttpResultCode_InvalidDataType, kOTHttpResultPhrase_InvalidDataType); \
	} \
	const Json::Value fieldVarName = (fieldParent)[(fieldName)]; \
	if((fieldVarName).isNull()) \
	{ \
		if(!(couldBeNull)){ \
				OT_DEBUG_ERROR_EX(kOTMobuleNameHttpTransport, "JSON '%s' is null", (fieldName)); \
				return new OTHttpResult(kOTHttpResultCode_InvalidDataType, "Required field is missing"); \
		}\
	} \
	if(!(fieldVarName).typeTestFun()) \
	{ \
		OT_DEBUG_ERROR_EX(kOTMobuleNameHttpTransport, "JSON '%s' has invalid type", (fieldName)); \
		return new OTHttpResult(kOTHttpResultCode_InvalidDataType, kOTHttpResultPhrase_InvalidDataType); \
	}

//
//	OTNetFileUpload
// 

OTNetFileUpload::OTNetFileUpload(OTNetFd nNetFd, std::string strFileName, std::string strFileType, size_t nFileSize, std::string strBridgeId, std::string strBridgePin, std::string strUserId)
		: m_nNetFd(nNetFd)
		, m_strFileName(strFileName)
		, m_strFileType(strFileType)
		, m_nFileSize(nFileSize)
		, m_strBridgeId(strBridgeId)
		, m_strBridgePin(strBridgePin)
		, m_strUserId(strUserId)
		, m_nWrittenSize(0)
		, m_pFile(NULL)
{
	int ret;

	std::string strFolder = (std::string("./") + strBridgeId);
	
	if(!folderExists(strFolder.c_str()) && (ret = OTMkdir(strFolder.c_str())) != 0)
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameHttpTransport, "mkdir(%s) failed with error code = %d", strFolder.c_str(),ret);
		return;
	}
	strFolder += (std::string("/") + strUserId);
	if(!folderExists(strFolder.c_str()) && (ret = OTMkdir(strFolder.c_str())) != 0)
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameHttpTransport, "mkdir(%s) failed with error code = %d", strFolder.c_str(), ret);
		return;
	}
	m_strFilePath = strFolder + std::string("/") + strFileName;
	if(!(m_pFile = fopen(m_strFilePath.c_str(), "wb")))
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameHttpTransport, "Failed to create/open file with path = %s", m_strFilePath.c_str());
		return;
	}
}

OTNetFileUpload::~OTNetFileUpload()
{ 
	close();
	OT_DEBUG_INFO("*** OTNetFileUpload destroyed ***"); 
}

bool OTNetFileUpload::close()
{
	if(m_pFile)
	{
		fclose(m_pFile);
		m_pFile = NULL;
	}
	return true;
}

size_t OTNetFileUpload::writeData(const void* pcDataPtr, size_t nDataSize)
{
	if(!pcDataPtr || !nDataSize)
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameHttpTransport, "Invalid parameter");
		return 0;
	}
	if(!isValid())
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameHttpTransport, "Trying to write data to invalid destination");
		return 0;
	}
	size_t nSize = fwrite(pcDataPtr, 1, nDataSize, m_pFile);
	m_nWrittenSize += nSize;
	return nSize;
}

//
//	OTHttpResult
// 

OTHttpResult::OTHttpResult(unsigned short nCode, const char* pcPhrase, const void* pcDataPtr, size_t nDataSize, OTHttpActionType_t eActionType)
: m_pDataPtr(NULL)
, m_nDataSize(0)
, m_eActionType(eActionType)
{
	m_nCode = nCode;
	m_pPhrase = tsk_strdup(pcPhrase);
	if(pcDataPtr && nDataSize)
	{
		if((m_pDataPtr = tsk_malloc(nDataSize)))
		{
			memcpy(m_pDataPtr, pcDataPtr, nDataSize);
			m_nDataSize = nDataSize;
		}
	}
}

OTHttpResult::~OTHttpResult()
{
	TSK_FREE(m_pPhrase);
	TSK_FREE(m_pDataPtr);
}

//
//	OTHttpTransport
//

OTHttpTransport::OTHttpTransport(uint64_t uEngineId, bool isSecure, const char* pcLocalIP, unsigned short nLocalPort)
: OTNetTransport(isSecure ? OTNetTransporType_TLS : OTNetTransporType_TCP, pcLocalIP, nLocalPort)
, m_uEngineId(uEngineId)
{
	m_oCallback = new OTHttpTransportCallback(this);
	setCallback(*m_oCallback);
}

OTHttpTransport::~OTHttpTransport()
{
	setCallback(NULL);
}


OTObjectWrapper<OTHttpResult*> OTHttpTransport::handleJsonContent(OTObjectWrapper<OTNetPeer*> oPeer, const void* pcDataPtr, size_t nDataSize)const
{
	if(!pcDataPtr || !nDataSize)
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameHttpTransport, "Invalid parameter");
		return new OTHttpResult(kOTHttpResultCode_InternalError, "Invalid parameter");
	}

	Json::Value root;
	Json::Reader reader;

	// Parse JSON content
	bool parsingSuccessful = reader.parse((const char*)pcDataPtr, (((const char*)pcDataPtr) + nDataSize), root);
	if (!parsingSuccessful)
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameHttpTransport, "Failed to parse JSON content: %.*s", nDataSize, pcDataPtr);
		return new OTHttpResult(kOTHttpResultCode_ParsingFailed, kOTHttpResultPhrase_ParsingFailed);
	}

	// JSON::action
	OT_JSON_GET(root, action, kJsonField_Action, isString, false);

	/* JSON::action=='req_presentation_upload' */
	if(tsk_striequals(action.asString().c_str(), kJsonValue_ActionReq_UploadPresentation))
	{
		// JSON::name
		OT_JSON_GET(root, name, kJsonField_Name, isString, false);
		// JSON::type
		OT_JSON_GET(root, type, kJsonField_Type, isString, false);
		// JSON::size
		OT_JSON_GET(root, size, kJsonField_Size, isIntegral, true);
		// JSON::bridge_id
		OT_JSON_GET(root, bridge_id, kJsonField_BridgeId, isString, false);
		// JSON::bridge_pin
		OT_JSON_GET(root, bridge_pin, kJsonField_BridgePin, isString, true);
		// JSON::user_id
		OT_JSON_GET(root, user_id, kJsonField_UserId, isString, false);
		
		// FIXME: make sure the bridge exist and is running (hard-coded bridges should allow uploading files)

		// create the file info
		OTObjectWrapper<OTNetFileUpload* > oFileInfo = new OTNetFileUpload(
			oPeer->getFd(),
			name.asString(),
			type.asString(),
			size.asUInt(),
			bridge_id.asString(),
			bridge_pin.asString(),
			user_id.asString());

		if(!oFileInfo->isValid())
		{
			return new OTHttpResult(kOTHttpResultCode_InternalError, kOTHttpResultPhrase_FailedToCreateLocalFile);
		}
		if(m_oFileUploads.find(oFileInfo->getNetFd()) != m_oFileUploads.end())
		{
			return new OTHttpResult(kOTHttpResultCode_InternalError, kOTHttpResultPhrase_FailedTransferPending);
		}
		// store the file info
		const_cast<OTHttpTransport*>(this)->m_oFileUploads[oFileInfo->getNetFd()] = oFileInfo;
		return new OTHttpResult(kOTHttpResultCode_Success, "File created", NULL, 0, OTHttpActionType_UploadPresensation);
	}	

	OT_DEBUG_ERROR_EX(kOTMobuleNameHttpTransport, "'%s' not valid JSON action", action.asString().c_str());
	return new OTHttpResult(kOTHttpResultCode_InvalidDataType, "Invalid action type");
}

bool OTHttpTransport::sendResult(OTObjectWrapper<OTNetPeer*> oPeer, OTObjectWrapper<OTHttpResult*> oResult)const
{
	if(!oResult || !oPeer)
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameHttpTransport, "Invalid parameter");
		return new OTHttpResult(kOTHttpResultCode_InternalError, "Invalid parameter");
	}

	bool bRet = false;
	void* pResult = NULL;

	int len = tsk_sprintf(
		(char**)&pResult, 
		"HTTP/1.1 %u %s\r\n"
		"Server: TelePresence Server " OT_VERSION_STRING "\r\n"
		"Access-Control-Allow-Origin: *\r\n"
		"Content-Length: %u\r\n"
		"Content-Type: " kJsonContentType "\r\n"
		"Connection: %s\r\n"
		"\r\n", 
		oResult->getCode(), 
		oResult->getPhrase(), 
		oResult->getDataSize(),
		(oResult->getActionType() == OTHttpActionType_UploadPresensation) ? "keep-alive" : "Close"
		);

	if(len <= 0 || !pResult)
	{
		goto bail;
	}
	if(oResult->getDataPtr() && oResult->getDataSize())
	{
		if(!(pResult = tsk_realloc(pResult, (len + oResult->getDataSize()))))
		{
			goto bail;
		}
		memcpy(&((uint8_t*)pResult)[len], oResult->getDataPtr(), oResult->getDataSize());
		len += oResult->getDataSize();
	}

	// send data
	bRet = const_cast<OTHttpTransport*>(this)->sendData(oPeer->getFd(), pResult, len);
	
bail:
	TSK_FREE(pResult);
	return bRet;
}

//
//	OTHttpTransportCallback
//
OTHttpTransportCallback::OTHttpTransportCallback(const OTHttpTransport* pcTransport)
{
	m_pcTransport = pcTransport;
}

OTHttpTransportCallback::~OTHttpTransportCallback()
{

}

bool OTHttpTransportCallback::onData(OTObjectWrapper<OTNetPeer*> oPeer, size_t &nConsumedBytes)
{
	size_t nDataSize;
	const int8_t* pcDataPtr;
	int32_t endOfheaders;
	bool haveAllContent = false;
	thttp_message_t *httpMessage = tsk_null;
	tsk_ragel_state_t ragelState;
	static const tsk_bool_t bExtractContentFalse = tsk_false;
	static const size_t kHttpOptionsResponseSize = tsk_strlen(kHttpOptionsResponse);
	int ret;

#if 0
	OT_DEBUG_INFO_EX(kOTMobuleNameHttpTransport, "Incoming data = %.*s", oPeer->getDataSize(), oPeer->getDataPtr());
#endif

	nConsumedBytes = 0;

	// https://developer.mozilla.org/en-US/docs/HTTP/Access_control_CORS


	/* Check if we have all HTTP headers. */
parse_buffer:
	pcDataPtr = ((const int8_t*)oPeer->getDataPtr()) + nConsumedBytes;
	nDataSize = (oPeer->getDataSize() - nConsumedBytes);
	
	if(oPeer->isRawContent())
	{
		endOfheaders = 0;
		goto parse_rawcontent;
	}
	if((endOfheaders = tsk_strindexOf((const char*)pcDataPtr, nDataSize, "\r\n\r\n"/*2CRLF*/)) < 0)
	{
		OT_DEBUG_INFO_EX(kOTMobuleNameHttpTransport, "No all HTTP headers in the TCP buffer");
		goto bail;
	}

	/* If we are here this mean that we have all HTTP headers.
	*	==> Parse the HTTP message without the content.
	*/
	tsk_ragel_state_init(&ragelState, (const char*)pcDataPtr, endOfheaders + 4/*2CRLF*/);
	if((ret = thttp_message_parse(&ragelState, &httpMessage, bExtractContentFalse)) == 0)
	{
		const thttp_header_Transfer_Encoding_t* transfer_Encoding;

		/* chunked? */
		if((transfer_Encoding = (const thttp_header_Transfer_Encoding_t*)thttp_message_get_header(httpMessage, thttp_htype_Transfer_Encoding)) && tsk_striequals(transfer_Encoding->encoding, "chunked"))
		{
			const char* start = (const char*)(pcDataPtr + (endOfheaders + 4/*2CRLF*/));
			const char* end = (const char*)(pcDataPtr + nDataSize);
			int index;

			OT_DEBUG_INFO_EX(kOTMobuleNameHttpTransport, "HTTP CHUNKED transfer");

			while(start < end)
			{
				/* RFC 2616 - 19.4.6 Introduction of Transfer-Encoding */
				// read chunk-size, chunk-extension (if any) and CRLF
				tsk_size_t chunk_size = (tsk_size_t)tsk_atox(start);
				if((index = tsk_strindexOf(start, (end-start), "\r\n")) >=0)
				{
					start += index + 2/*CRLF*/;
				}
				else
				{
					OT_DEBUG_INFO_EX(kOTMobuleNameHttpTransport, "Parsing chunked data has failed.");
					break;
				}

				if(chunk_size == 0 && ((start + 2) <= end) && *start == '\r' && *(start+ 1) == '\n')
				{
					int parsed_len = (start - (const char*)(pcDataPtr)) + 2/*CRLF*/;
#if 0
					tsk_buffer_remove(dialog->buf, 0, parsed_len);
#else
					nConsumedBytes += 
#endif
					haveAllContent = true;
					break;
				}
					
				thttp_message_append_content(httpMessage, start, chunk_size);
				start += chunk_size + 2/*CRLF*/;
			}
		}
		else
		{
			tsk_size_t clen = THTTP_MESSAGE_CONTENT_LENGTH(httpMessage); /* MUST have content-length header. */
			if(clen == 0)
			{ /* No content */
				nConsumedBytes += (endOfheaders + 4/*2CRLF*/);/* Remove HTTP headers and CRLF ==> must never happen */
				haveAllContent = true;
			}
			else
			{ 
				/* There is a content */
				if(isFileContentType(httpMessage))// Is it for file-upload?
				{
					oPeer->setRawContent(true);
					endOfheaders += 4/*2CRLF*/;
					goto parse_rawcontent;
				}
				
				if((endOfheaders + 4/*2CRLF*/ + clen) > nDataSize)
				{ /* There is content but not all the content. */
					OT_DEBUG_INFO_EX(kOTMobuleNameHttpTransport, "No all HTTP content in the TCP buffer.");
					goto bail;
				}
				else
				{
					/* Add the content to the message. */
					thttp_message_add_content(httpMessage, tsk_null, pcDataPtr + endOfheaders + 4/*2CRLF*/, clen);
					/* Remove HTTP headers, CRLF and the content. */
					nConsumedBytes += (endOfheaders + 4/*2CRLF*/ + clen);
					haveAllContent = true;
				}
			}
		}
	}
	else
	{
		// fails to parse an HTTP message with all headers
		nConsumedBytes += endOfheaders + 4/*2CRLF*/;
	}
	
	
	if(httpMessage && haveAllContent)
	{
		/* Analyze HTTP message */
		if(THTTP_MESSAGE_IS_REQUEST(httpMessage))
		{
			/* OPTIONS */
			if(tsk_striequals(httpMessage->line.request.method, kHttpMethodOptions))
			{
				// https://developer.mozilla.org/en-US/docs/HTTP/Access_control_CORS
				if(!const_cast<OTHttpTransport*>(m_pcTransport)->sendData(oPeer->getFd(), kHttpOptionsResponse, kHttpOptionsResponseSize))
				{
					OT_DEBUG_ERROR_EX(kOTMobuleNameHttpTransport, "Failed to send response to HTTP OPTIONS");
				}
			}
			/* POST */
			else if(tsk_striequals(httpMessage->line.request.method, kHttpMethodPost))
			{
				if(!isJsonContentType(httpMessage))
				{
					m_pcTransport->sendResult(oPeer, new OTHttpResult(kOTHttpResultCode_InvalidData, "Invalid content-type"));
				}
				else if(!THTTP_MESSAGE_HAS_CONTENT(httpMessage))
				{
					m_pcTransport->sendResult(oPeer, new OTHttpResult(kOTHttpResultCode_InvalidData, "Bodiless POST request not allowed"));
				}
				else
				{
					OTObjectWrapper<OTHttpResult*> oResult = m_pcTransport->handleJsonContent(oPeer, THTTP_MESSAGE_CONTENT(httpMessage), THTTP_MESSAGE_CONTENT_LENGTH(httpMessage));
					m_pcTransport->sendResult(oPeer, oResult);
				}
			}
		}

		/* Parse next chunck */
		if((nDataSize - nConsumedBytes) >= kStreamChunckMinSize)
		{
			TSK_OBJECT_SAFE_FREE(httpMessage);
			goto parse_buffer;
		}
	}

parse_rawcontent:
	if(oPeer->isRawContent())
	{
		if(m_pcTransport->m_oFileUploads.find(oPeer->getFd()) == m_pcTransport->m_oFileUploads.end())
		{
			OT_DEBUG_ERROR_EX(kOTMobuleNameHttpTransport, "Failed to find local file");
			m_pcTransport->sendResult(oPeer, new OTHttpResult(kOTHttpResultCode_InternalError, "Failed to find local file"));
			nConsumedBytes = nDataSize;
			goto bail;
		}
		OTObjectWrapper<OTNetFileUpload*> oFileUpload = const_cast<OTHttpTransport*>(m_pcTransport)->m_oFileUploads[oPeer->getFd()];

		if(httpMessage)
		{
			const size_t nMessageContentLength = THTTP_MESSAGE_CONTENT_LENGTH(httpMessage);
			if(nMessageContentLength > 0 && !oFileUpload->getFileSize())
			{
				oFileUpload->setFileSize(nMessageContentLength);
			}
		}
		
		nConsumedBytes += endOfheaders;
		if(endOfheaders < (int32_t)nDataSize)
		{
			const void* _pcDataPtr = pcDataPtr + endOfheaders;
			const size_t _nDataSize = nDataSize - endOfheaders;
			nConsumedBytes += oFileUpload->writeData(_pcDataPtr, _nDataSize);
		}

		if(oFileUpload->isAllDataWritten())
		{
			OT_DEBUG_INFO_EX(kOTMobuleNameHttpTransport, "We got the complete file :)");
			oFileUpload->close(); // close the file to be able to use it

			short nCode = kOTHttpResultCode_Success;
			const char* pcPhrase = "We got the file";
			
			OTObjectWrapper<OTBridge*>oBridge = OTEngine::getBridge(const_cast<OTHttpTransport*>(m_pcTransport)->getEngineId(), oFileUpload->getBridgeId());
			if(oBridge)
			{
				OTObjectWrapper<OTSipSessionAV*> oAVCall = oBridge->findCallByUserId(oFileUpload->getUserId());
				if(oAVCall)
				{
					if(!oAVCall->presentationShare(oFileUpload->getFilePath()))
					{
						nCode = kOTHttpResultCode_InternalError;
						pcPhrase = "Failed to share presentation";
					}
				}
				else
				{
					nCode = kOTHttpResultCode_InternalError;
					pcPhrase = "Failed to find call by user id";
				}
			}
			else
			{
				nCode = kOTHttpResultCode_InternalError;
				pcPhrase = "Failed to find bridge";
			}
			m_pcTransport->sendResult(oPeer, new OTHttpResult(nCode, pcPhrase)); // close the connection

			// start sharing
		}
	}

bail:
	TSK_OBJECT_SAFE_FREE(httpMessage);
	
	return true;
}

bool OTHttpTransportCallback::onConnectionStateChanged(OTObjectWrapper<OTNetPeer*> oPeer)
{
	if(!oPeer->isConnected())
	{
		const_cast<OTHttpTransport*>(m_pcTransport)->m_oFileUploads.erase(oPeer->getFd());
	}
	return true;
}

static OT_INLINE bool folderExists(const char* folder)
{
	if(!folder)
	{
		return false;
	}
    struct stat st;
    return (stat(folder, &st) == 0) && (st.st_mode & S_IFDIR);
}

static const char* getHttpContentType(const thttp_message_t *pcHttpMessage)
{
	const thttp_header_Content_Type_t* contentType;

	if(!pcHttpMessage)
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameHttpTransport, "Invalid parameter");
		return NULL;
	}

	if((contentType = (const thttp_header_Content_Type_t*)thttp_message_get_header(pcHttpMessage, thttp_htype_Content_Type)))
	{
		return contentType->type;
	}
	return NULL;
}

static OT_INLINE bool isContentType(const thttp_message_t *pcHttpMessage, const char* pcContentTypeToCompare)
{
	// content-type without parameters
	const char* pcContentType = getHttpContentType(pcHttpMessage);
	if(pcContentType)
	{
		return tsk_striequals(pcContentTypeToCompare, pcContentType);
	}
	return false;
}

static OT_INLINE bool isJsonContentType(const thttp_message_t *pcHttpMessage)
{
	return isContentType(pcHttpMessage, kJsonContentType);
}

static OT_INLINE bool isFileContentType(const thttp_message_t *pcHttpMessage)
{
	return isContentType(pcHttpMessage, kFileContentType);
}