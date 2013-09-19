/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#include "opentelepresence/OTWrap.h"
#include "opentelepresence/OTEngine.h"
#include "opentelepresence/jsoncpp/json.h"

#include "SipEvent.h"
#include "SipMessage.h"

#include "tsk_debug.h"

#include <assert.h>

#define kJsonPresShareResponseState(id, state, pageCount) \
	"{\"action\": \"res_presentation_state\", \"id\": " #id ", \"state\": \"" state "\", \"page-count\":" #pageCount "}"

#define OT_JSON_GET(fieldParent, fieldVarName, fieldName, typeTestFun, couldBeNull) \
	if(!fieldParent.isObject()){ \
		OT_DEBUG_ERROR("JSON '%s' not an object", (fieldName)); \
		return false; \
	} \
	const Json::Value fieldVarName = (fieldParent)[(fieldName)]; \
	if((fieldVarName).isNull()) \
	{ \
		if(!(couldBeNull)){ \
			OT_DEBUG_ERROR("JSON '%s' is null", (fieldName)); \
			return false; \
		}\
	} \
	if(!(fieldVarName).typeTestFun()) \
	{ \
		OT_DEBUG_ERROR("JSON '%s' has invalid type", (fieldName)); \
		return false; \
	}

//
//	OTSipSession
//
OTSipSession::OTSipSession(SipSession** ppSipSessionToWrap, OTObjectWrapper<OTBridgeInfo*> oBridgeInfo)
: m_eState(OTSessionState_None)
{
	OT_ASSERT(ppSipSessionToWrap && *ppSipSessionToWrap);

	m_pWrappedSession = *ppSipSessionToWrap;
	*ppSipSessionToWrap = NULL;
}

OTSipSession::~OTSipSession()
{
	OT_DEBUG_INFO("*** OTSipSession object destroyed ***");

	if(m_pWrappedSession)
	{
		delete m_pWrappedSession, m_pWrappedSession = NULL;
	}
}


//
//	OTSipSessionAV
//
OTSipSessionAV::OTSipSessionAV(CallSession** ppCallSessionToWrap, OTMediaType_t eMediaType, OTObjectWrapper<OTBridgeInfo*> oBridgeInfo, std::string strUserId, std::string strDisplayName, std::string strJobTitle)
: OTSipSession((SipSession **)ppCallSessionToWrap, oBridgeInfo)
, m_eMediaType(eMediaType)
{
	m_oInfo = OTSessionInfoAV::New(oBridgeInfo, getWrappedCallSession());
	m_oInfo->setDisplayName(strDisplayName);
	m_oInfo->setUserId(strUserId);
	m_oInfo->setJobTitle(strJobTitle);
}

OTSipSessionAV::~OTSipSessionAV()
{
	OT_DEBUG_INFO("*** OTSipSessionAV object destroyed ***");
	OTObjectSafeRelease(m_oDocStreamer);
}

void OTSipSessionAV::setState(OTSessionState_t eState)
{
	// call base
	OTSipSession::setState(eState);

	switch(eState)
	{
		case OTSessionState_None:
		default:
			{
				break;
			}
	}
}

bool OTSipSessionAV::presentationShare(std::string strFilePath)
{
	if(m_oDocStreamer && m_oDocStreamer->isOpened())
	{
		OT_DEBUG_ERROR("Already have active doc streamer");
		return false;
	}

	m_oDocStreamer = OTDocStreamer::New(m_oInfo->getBridgeInfo(), this);
	if(!m_oDocStreamer)
	{
		OT_DEBUG_ERROR("Failed to create a doc streamer");
		return false;
	}
	return m_oDocStreamer->open(strFilePath);
}

bool OTSipSessionAV::handleIncomingData(const char* pcContentType, const void* pcDataPtr, const size_t nDataSize)
{
	if(tsk_strnullORempty(pcContentType) || !pcDataPtr || !nDataSize)
	{
		return true;
	}

	if(tsk_striequals(pcContentType, kJsonContentType))
	{
		Json::Value root;
		Json::Reader reader;
		bool parsingSuccessful = reader.parse((const char*)pcDataPtr, (((const char*)pcDataPtr) + nDataSize), root);
		if (!parsingSuccessful)
		{
			OT_DEBUG_ERROR("Failed to parse JSON content: %.*s", nDataSize, pcDataPtr);
			return false;
		}
		// JSON::action
		OT_JSON_GET(root, action, "action", isString, false);
		if(tsk_striequals(action.asString().c_str(), "req_presentation_goto") || tsk_striequals(action.asString().c_str(), "req_presentation_close"))
		{
			OTObjectWrapper<OTDocStreamer*> oDocStreamer = m_oDocStreamer; // copy()
			if(!oDocStreamer)
			{
				OT_DEBUG_ERROR("No active doc streamer");
				return false;
			}
			if(tsk_striequals(action.asString().c_str(), "req_presentation_goto"))
			{
				// JSON::page
				OT_JSON_GET(root, page_index, "page_index", isIntegral, false);
				if(page_index.asUInt() > oDocStreamer->getPagesCount())
				{
					OT_DEBUG_ERROR("Export page failed because of wrong index: %u>=%u", page_index.asUInt(), oDocStreamer->getPagesCount());
					return false;
				}
				return oDocStreamer->exportPage(page_index.asInt());
			}
			else /* if(tsk_striequals(action.asString().c_str(), "req_presentation_close")) */
			{
				return oDocStreamer->close();
			}
		}
		else if(tsk_striequals(action.asString().c_str(), "req_call_mute"))
		{
			// JSON::enabled
			OT_JSON_GET(root, enabled, "enabled", isBool, false);
			m_oInfo->setMuteRemote(enabled.asBool());
			return true;
		}
		else
		{
			OT_DEBUG_ERROR("'%s' not valid JSON action", action.asString().c_str());
			return false;
		}
	}
	else
	{
		OT_DEBUG_ERROR("%s not supported content type", pcContentType);
		return false;
	}
}

// @Override (OTDocStreamerCallback)
void OTSipSessionAV::onStateChanged(OTDocStreamerState_t eState, OTDocStreamer* oStreamer) const
{
	switch(eState)
	{
		case OTDocStreamerState_Opening:
			{
				presentationSendInfo("opening");
				break;
			}
		case OTDocStreamerState_Opened:
			{
				presentationSendInfo("opened");
				//if(oStreamer->getPagesCount() > 0)
				//{
				//	oStreamer->exportPage(0);
				//}
				break;
			}
		case OTDocStreamerState_Exported:
			{
				presentationSendInfo("exported");
				m_oInfo->setDocStreamerId(oStreamer->getId());
				break;
			}
		case OTDocStreamerState_Closed:
			{
				presentationSendInfo("closed");
				OTObjectSafeRelease(const_cast<OTSipSessionAV*>(this)->m_oDocStreamer);
				m_oInfo->setDocStreamerId(OPENTELEPRESENCE_INVALID_ID);
				break;
			}
	}
}

// @Override (OTDocStreamerCallback)
void OTSipSessionAV::onError(bool bIsFatal, OTDocStreamer* oStreamer) const
{
	if(bIsFatal)
	{
		presentationSendInfo("error");
		OTObjectSafeRelease(const_cast<OTSipSessionAV*>(this)->m_oDocStreamer);
		m_oInfo->setDocStreamerId(OPENTELEPRESENCE_INVALID_ID);
	}
}

bool OTSipSessionAV::presentationSendInfo(const char* pcState) const
{
	bool bRet = false;
	char* pJsonContent = tsk_null;
	ActionConfig* pActionConfig = tsk_null;

	if(!m_oDocStreamer)
	{
		OT_DEBUG_ERROR("No active doc streamer");
		goto bail;
	}
	if(!pcState)
	{
		OT_DEBUG_ERROR("Invalid parameter");
		goto bail;
	}
	
	tsk_sprintf(&pJsonContent,
		"{\"action\": \"res_presentation_state\", \"id\": %llu, \"state\": \"%s\", \"page_index\":%u, \"page_count\":%u}",
		m_oDocStreamer->getId(),
		pcState,
		TSK_CLAMP(0, m_oDocStreamer->getPageIndex(), m_oDocStreamer->getPagesCount()),
		m_oDocStreamer->getPagesCount());

	if((pActionConfig = new ActionConfig()))
	{
		pActionConfig->addHeader("Content-Type", kJsonContentType);					
	}
	if(!(bRet = const_cast<InviteSession*>(const_cast<OTSipSessionAV*>(this)->getWrappedInviteSession())->sendInfo(pJsonContent, tsk_strlen(pJsonContent), pActionConfig)))
	{
		OT_DEBUG_ERROR("Failed to send INFO message");
	}
	OT_SAFE_DELETE_CPP(pActionConfig);

bail:
	TSK_FREE(pJsonContent);
	return bRet;
}


// 
//	OTSipCallback
//
OTSipCallback::OTSipCallback(OTObjectWrapper<OTEngine*> oEngine /*= NULL*/)
: SipCallback()
, m_oEngine(oEngine)
{
	
}

OTSipCallback::~OTSipCallback() 
{
	
}

int OTSipCallback::OnRegistrationEvent(const RegistrationEvent* e)
{
	OTObjectWrapper<OTEngine*> oEngine = m_oEngine; // copy() to avoid locking

	if(!oEngine)
	{
		OT_DEBUG_INFO("Null engine...ignoring OnRegistrationEvent callback");
		return 0;
	}

	switch(e->getType())
	{
		case tsip_i_newreg:
			{
				OT_ASSERT(e->getSession() == NULL);

				RegistrationSession *pRegSession;					
				if((pRegSession = e->takeSessionOwnership()))
				{
					if(oEngine->m_oInfo->m_bAcceptIncomingSipReg)
					{
						pRegSession->accept();
					}
					else
					{
						ActionConfig* pConfig = new ActionConfig();
						if(pConfig)
						{
							pConfig->setResponseLine(600, "Registration not enabled on Telepresence system");
							pConfig->addHeader("Server", kOTSipHeaderServer);
						}
						pRegSession->reject(pConfig);
						if(pConfig) delete pConfig, pConfig = NULL;
					}
					delete pRegSession, pRegSession = NULL;
				}

				break;
			}
	}

	return 0;
}

int OTSipCallback::OnInviteEvent(const InviteEvent* e)
{
	OTObjectWrapper<OTEngine*> oEngine = m_oEngine; // copy() to avoid locking

	if(!oEngine)
	{
		OT_DEBUG_INFO("Null engine...ignoring OnInviteEvent callback");
		return 0;
	}

	switch(e->getType())
	{
		case tsip_i_newcall:
			{
				OT_ASSERT(e->getSession() == NULL);

				// at this we are sure that there is wrapped message with a valid From header (checked by Doubango). The test here is to be sure there is
				// no bug/regression in Doubango instead of cheking the validity of the message from the client.
				const SipMessage* pcMessage = e->getSipMessage();
				OT_ASSERT(pcMessage != NULL);
				
				const tsip_message_t* pcWrappedMessage = pcMessage->getWrappedSipMessage();
				OT_ASSERT(pcWrappedMessage != NULL);
				OT_ASSERT(pcWrappedMessage->To && pcWrappedMessage->To->uri);
				OT_ASSERT(pcWrappedMessage->From && pcWrappedMessage->From->uri);
				
				CallSession *pCallSession;
				if((pCallSession = e->takeCallSessionOwnership()))
				{
					// 'user_name' in 'To' (Huawei) and 'From' (Vidyo) could be null as per https://groups.google.com/forum/#!topic/opentelepresence/VHbELRtdQbQ and https://groups.google.com/forum/#!topic/opentelepresence/YLeDe-DPv-Q
					// The 'To' contains the bridge id and 'From' the user id. Both must not be null.
					if(tsk_strnullORempty(pcWrappedMessage->To->uri->user_name) || tsk_strnullORempty(pcWrappedMessage->From->uri->user_name))
					{
						rejectCall(pCallSession, 484, tsk_strnullORempty(pcWrappedMessage->To->uri->user_name) ? "Incomplete destination address" : "Incomplete source address");
						delete pCallSession, pCallSession = NULL;
						return 0;
					}
					
					// create bridge identifier from the destination name
					std::string strBridgeId(pcWrappedMessage->To->uri->user_name);

					// find a bridge with this id
					std::map<std::string, OTObjectWrapper<OTBridge*> >::iterator iter = oEngine->m_oBridges.find(strBridgeId);
					OTObjectWrapper<OTBridge*> oBridge;
					if((iter == oEngine->m_oBridges.end()) || !(oBridge = iter->second))
					{
						OT_DEBUG_INFO("No bridge with id = %s...create new one", strBridgeId.c_str());
						if((oBridge = new OTBridge(strBridgeId, oEngine->m_oInfo)))
						{
							oBridge->m_oMapOfCfgParams = oEngine->m_oBridgeCfgs[strBridgeId];
							oEngine->m_oBridges[strBridgeId] = oBridge;
							OT_DEBUG_INFO("Engine contains %u bridges(insert)", oEngine->m_oBridges.size());
						}
					}
					OT_ASSERT(oBridge);

					// map() session <-> bridge
					m_oMapSession2Bridge[pCallSession->getId()] = strBridgeId;
					
					// Before accepting the call "link it with the bridge"
					const MediaSessionMgr* pcMediaSessionMgr = pCallSession->getMediaMgr();
					OTMediaType_t eMediaType = OTMediaType_None;
					if(e->getMediaType() & twrap_media_audio)eMediaType=(OTMediaType_t)(eMediaType|OTMediaType_Audio);
					if(e->getMediaType() & twrap_media_video)eMediaType=(OTMediaType_t)(eMediaType|OTMediaType_Video);
					
					if(pcMediaSessionMgr && (eMediaType != OTMediaType_None))
					{
						// before accepting check authentication info
						OTObjectWrapper<OTCfgParam*> oParamBridgePin = oBridge->m_oMapOfCfgParams["pin-code"];
						if(oParamBridgePin)
						{
							// No support for DTMF in this beta version: only TP-BridgePin
							char* pPinCode = const_cast<SipMessage*>(pcMessage)->getSipHeaderValue("TP-BridgePin");
							bool bAuthOk = false;
							const char* pcErrorPhrase = "Forbidden(22)";
							if(!pPinCode)
							{
								OT_DEBUG_ERROR("TP-BridgePin header is missing and bridge with id='%s' is protected",strBridgeId.c_str());
								pcErrorPhrase = "Forbidden(TP-BridgePin header missing)";
							}
							else
							{
								if(!(bAuthOk = tsk_striequals(oParamBridgePin->getValue(), pPinCode)))
								{
									OT_DEBUG_ERROR("BridgePin mismatch: %s<>%s", pPinCode, oParamBridgePin->getValue());
									pcErrorPhrase = "Forbidden(Pin mismatch)";
								}
								TSK_FREE(pPinCode);
							}

							if(!bAuthOk)
							{
								rejectCall(pCallSession, 403, pcErrorPhrase);
								delete pCallSession, pCallSession = NULL;
								return 0;
							}
						}

						// send 200 OK
						{
							// get information from the INVITE request
							const char* pcDisplayName = tsk_strnullORempty(pcWrappedMessage->From->uri->display_name) ? pcWrappedMessage->From->uri->user_name : pcWrappedMessage->From->uri->display_name;
							const char* pcUserId = pcWrappedMessage->From->uri->user_name;
							char* pJobTitle = const_cast<SipMessage*>(pcMessage)->getSipHeaderValue("TP-JobTitle");
							std::string strDisplayName(pcDisplayName);
							std::string strUserId(pcUserId);
							std::string strJobTitle(pJobTitle ? pJobTitle : "");

							OTObjectWrapper<OTSipSessionAV*> oAVCall = new OTSipSessionAV(&pCallSession, eMediaType, oBridge->getInfo(), pcUserId, strDisplayName, strJobTitle);
							
							char* pAudioPosition = const_cast<SipMessage*>(pcMessage)->getSipHeaderValue("TP-AudioPosition");
							char* pAudioVelocity = const_cast<SipMessage*>(pcMessage)->getSipHeaderValue("TP-AudioVelocity");
							if(pAudioPosition)
							{
								tsk_strunquote_2(&pAudioPosition, '[', ']');
								tsk_params_L_t* pParams = tsk_params_fromstring(pAudioPosition, ",", tsk_true);
								if(pParams)
								{
									if(tsk_list_count(pParams, tsk_null, tsk_null) == 3){
										oAVCall->getInfo()->setAudioPosition(
											(float)atof(((const tsk_param_t*)pParams->head->data)->name),
											(float)atof(((const tsk_param_t*)pParams->head->next->data)->name),
											(float)atof(((const tsk_param_t*)pParams->head->next->next->data)->name));
									}
									TSK_OBJECT_SAFE_FREE(pParams);
								}
							}
							if(pAudioVelocity)
							{
								tsk_strunquote_2(&pAudioVelocity, '[', ']');
								tsk_params_L_t* pParams = tsk_params_fromstring(pAudioVelocity, ",", tsk_true);
								if(pParams)
								{
									if(tsk_list_count(pParams, tsk_null, tsk_null) == 3){
										oAVCall->getInfo()->setAudioVelocity(
											(float)atof(((const tsk_param_t*)pParams->head->data)->name),
											(float)atof(((const tsk_param_t*)pParams->head->next->data)->name),
											(float)atof(((const tsk_param_t*)pParams->head->next->next->data)->name));
									}
									TSK_OBJECT_SAFE_FREE(pParams);
								}
							}

							TSK_FREE(pJobTitle);
							TSK_FREE(pAudioPosition);
							TSK_FREE(pAudioVelocity);

							// add call to the bridge
							oBridge->addAVCall(oAVCall);

							// send 200 OK
							if(const_cast<CallSession*>(oAVCall->getWrappedCallSession())->accept())
							{
								// start bridge if not already done
								oBridge->start();
							}
							else
							{
								processSessionTerminated(oAVCall->getId()); // FORCE
								const_cast<CallSession*>(oAVCall->getWrappedCallSession())->reject();
							}
						}
					}
					else
					{
						rejectCall(pCallSession, 600, "Internal error(23)");
						delete pCallSession, pCallSession = NULL;
					}
					
					if(pCallSession)
					{
						delete pCallSession, pCallSession = NULL;
					}

				}//en-of if(take())

				break;
			}//end-of case(tsip_i_newcall)

		case tsip_i_request:
			{
				if(!e->getSession())
				{
					break;
				}
				
				const SipMessage* pcMessage = e->getSipMessage();
				OT_ASSERT(pcMessage != NULL);
				const tsip_message_t* pcWrappedMessage = pcMessage->getWrappedSipMessage();
				OT_ASSERT(pcMessage != NULL);

				if(const_cast<SipMessage*>(pcMessage)->getRequestType() == tsip_INFO)
				{
					std::string strBridgeId(pcWrappedMessage->To->uri->user_name);
					std::map<std::string, OTObjectWrapper<OTBridge*> >::iterator iter = oEngine->m_oBridges.find(strBridgeId);
					OTObjectWrapper<OTBridge*> oBridge;
					if(iter != oEngine->m_oBridges.end() && (oBridge = iter->second))
					{
						OTObjectWrapper<OTSipSessionAV*> oCall = oBridge->findCallBySessionId(e->getSession()->getId());
						if(oCall)
						{
							oCall->handleIncomingData(
								TSIP_MESSAGE_CONTENT_TYPE(pcWrappedMessage), 
								TSIP_MESSAGE_CONTENT_DATA(pcWrappedMessage), 
								TSIP_MESSAGE_CONTENT_DATA_LENGTH(pcWrappedMessage));
						}
						else
						{
							OT_DEBUG_ERROR("Failed to find call with id = %u on bridge with id = %s", e->getSession()->getId(), strBridgeId.c_str());
						}
					}
					else
					{
						OT_DEBUG_ERROR("Failed to find bridge with id = %s", strBridgeId.c_str());
					}
				}

				break;
			}//end-of case(tsip_i_request)
	}//end-of switch(e->getType())

	return 0;
}

int OTSipCallback::OnDialogEvent(const DialogEvent* e)
{
	OTObjectWrapper<OTEngine*> oEngine = m_oEngine; // copy() to avoid locking

	if(!oEngine)
	{
		OT_DEBUG_INFO("Null engine...ignoring OnDialogEvent callback");
		return 0;
	}

	const SipSession *pcSession = e->getBaseSession();
	if(!pcSession)
	{
		return 0;
	}

	uint64_t u64SessionId = pcSession->getId();
	short u16Code = e->getCode();

	switch(u16Code)
	{
		case tsip_event_code_dialog_connected:
			{
				// FIXME: only if INVITE session
				// restart the mixer manager
				// will be paused if no active session could be found
				// m_pOTBridge->m_pOTMixerMgrMgr->Start(OTMediaType_All);
				break;
			}

		case tsip_event_code_dialog_terminated:
			{
				processSessionTerminated(u64SessionId);
				break;
			}
	}

	return 0; 
}


bool OTSipCallback::processSessionTerminated(uint64_t u64SessionId)
{
	OTObjectWrapper<OTEngine*> oEngine = m_oEngine; // copy() to avoid locking

	std::string strBridgeId = m_oMapSession2Bridge[u64SessionId];
	if(!strBridgeId.empty())
	{
		OTObjectWrapper<OTBridge*> oBridge = oEngine->m_oBridges[strBridgeId];
		if(oBridge)
		{
			oBridge->removeAVCall(u64SessionId);
			OT_DEBUG_INFO("Engine contains %u bridges(erase)", oEngine->m_oBridges.size());
			if(oBridge->getNumberOfActiveAVCalls() == 0)
			{
				oEngine->m_oBridges.erase(strBridgeId);
			}
		}
	}
	m_oMapSession2Bridge.erase(u64SessionId);

	return true;
}

bool OTSipCallback::rejectCall(const CallSession* pcCallSession, short nCode, const char* pcPhrase)
{
	bool bRet = false;

	if(pcCallSession)
	{
		processSessionTerminated(pcCallSession->getId()); // FORCE
		ActionConfig* config = new ActionConfig();
		if(config)
		{
			config->setResponseLine(nCode, pcPhrase);
		}
		bRet = const_cast<CallSession*>(pcCallSession)->reject(config);
		if(config)
		{
			delete config;
		}
	}
	else
	{
		OT_DEBUG_ERROR("Invalid parameter");
		return false;
	}
	return bRet;
}


//
//	OTSipStack
//

OTSipStack::OTSipStack(OTObjectWrapper<OTSipCallback*> oCallback, const char* pcRealmUri, const char* pcPrivateId, const char* pcPublicId)
{
	m_pStack = new SipStack(*oCallback, pcRealmUri, pcPrivateId, pcPublicId);
}

OTSipStack::~OTSipStack()
{
	if(m_pStack)
	{
		delete m_pStack, m_pStack = NULL;
	}
}

bool OTSipStack::isValid()
{
	return (m_pStack && m_pStack->isValid());
}
