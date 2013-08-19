/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_HTTP_TRANSPORT_H
#define OPENTELEPRESENCE_HTTP_TRANSPORT_H

#include "OpenTelepresenceConfig.h"
#include "OTNetTransport.h"


class OTHttpTransport;

class OTNetFileUpload : public OTObject
{
public:
	OTNetFileUpload(OTNetFd nNetFd, std::string strFileName, std::string strFileType, size_t nFileSize, std::string strBridgeId, std::string strBridgePin, std::string strUserId);
	~OTNetFileUpload();
	virtual OT_INLINE const char* getObjectId() { return "OTNetFileUpload"; }
	
	virtual OT_INLINE bool isValid(){ return (m_pFile != NULL); }
	virtual OT_INLINE OTNetFd getNetFd(){ return  m_nNetFd; }
	virtual OT_INLINE size_t getFileSize(){ return  m_nFileSize; }
	virtual OT_INLINE void setFileSize(size_t nFileSize){  m_nFileSize = nFileSize; }
	virtual OT_INLINE std::string getFileName(){ return  m_strFileName; }
	virtual OT_INLINE std::string getFilePath(){ return  m_strFilePath; }
	virtual OT_INLINE std::string getFileType(){ return  m_strFileType; }
	virtual OT_INLINE std::string getBridgeId(){ return  m_strBridgeId; }
	virtual OT_INLINE std::string getBridgePin(){ return  m_strBridgePin; }
	virtual OT_INLINE std::string getUserId(){ return  m_strUserId; }

	virtual bool close();
	virtual size_t writeData(const void* pcDataPtr, size_t nDataSize);
	virtual OT_INLINE bool isAllDataWritten() { return (m_nWrittenSize == m_nFileSize); }
	
private:
	OTNetFd m_nNetFd;
	FILE* m_pFile;
	size_t m_nFileSize, m_nWrittenSize;
	std::string m_strFileName, m_strFilePath, m_strFileType, m_strBridgeId, m_strBridgePin, m_strUserId;
};

//
//	OTHttpResult
// 
class OTHttpResult : public OTObject
{
public:
	OTHttpResult(unsigned short nCode, const char* pcPhrase, const void* pcDataPtr = NULL, size_t nDataSize = 0, OTHttpActionType_t eActionType = OTHttpActionType_None);
	virtual ~OTHttpResult();
	virtual OT_INLINE const char* getObjectId() { return "OTHttpResult"; }

	virtual OT_INLINE unsigned short getCode() { return m_nCode; }
	virtual OT_INLINE const char* getPhrase() { return m_pPhrase; }
	virtual OT_INLINE const void* getDataPtr() { return m_pDataPtr; }
	virtual OT_INLINE size_t getDataSize() { return m_nDataSize; }
	virtual OT_INLINE OTHttpActionType_t getActionType() { return m_eActionType; }

private:
	unsigned short m_nCode;
	char* m_pPhrase;
	void* m_pDataPtr;
	size_t m_nDataSize;
	OTHttpActionType_t m_eActionType;
};

//
//	OTHttpTransportCallback
//
class OTHttpTransportCallback : public OTNetTransportCallback
{
public:
	OTHttpTransportCallback(const OTHttpTransport* pcTransport);
	virtual ~OTHttpTransportCallback();
	virtual OT_INLINE const char* getObjectId() { return "OTHttpTransportCallback"; }
	virtual bool onData(OTObjectWrapper<OTNetPeer*> oPeer, size_t &nConsumedBytes);
	virtual bool onConnectionStateChanged(OTObjectWrapper<OTNetPeer*> oPeer);
private:
	const OTHttpTransport* m_pcTransport;
};

//
//	OTHttpTransport
//
class OTHttpTransport : public OTNetTransport
{
	friend class OTHttpTransportCallback;
public:
	OTHttpTransport(uint64_t uEngineId, bool isSecure, const char* pcLocalIP, unsigned short nLocalPort);
	virtual ~OTHttpTransport();
	virtual OT_INLINE const char* getObjectId() { return "OTHttpTransport"; }

	virtual OT_INLINE uint64_t getEngineId() { return m_uEngineId; }

private:
	OTObjectWrapper<OTHttpResult*> handleJsonContent(OTObjectWrapper<OTNetPeer*> oPeer, const void* pcDataPtr, size_t nDataSize)const;
	bool sendResult(OTObjectWrapper<OTNetPeer*> oPeer, OTObjectWrapper<OTHttpResult*> oResult)const;

private:
	OTObjectWrapper<OTHttpTransportCallback*> m_oCallback;
	uint64_t m_uEngineId;	
	std::map<OTNetFd, OTObjectWrapper<OTNetFileUpload*> > m_oFileUploads;
};

#endif /* OPENTELEPRESENCE_HTTP_TRANSPORT_H */
