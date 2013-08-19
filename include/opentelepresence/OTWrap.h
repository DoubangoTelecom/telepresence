/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_WRAP_H
#define OPENTELEPRESENCE_WRAP_H

#include "OpenTelepresenceConfig.h"

#include "opentelepresence/OTObject.h"
#include "opentelepresence/OTCommon.h"
#include "opentelepresence/OTSessionInfo.h"
#include "opentelepresence/docstreamer/OTDocStreamer.h"

#include "SipSession.h"
#include "SipStack.h"

#include <map>
#include <string>

class RegistrationEvent;
class InviteEvent;
class DialogEvent;

class OTEngine;

//
//	OTSipSession
//
class OTSipSession : public OTObject
{
public:
	OTSipSession(SipSession** ppSipSessionToWrap, OTObjectWrapper<OTBridgeInfo*> oBridgeInfo);
	virtual ~OTSipSession();
	virtual OT_INLINE const char* getObjectId() { return "OTSipSession"; }
	virtual OT_INLINE const SipSession* getWrappedSession(){ return m_pWrappedSession; }
	virtual OT_INLINE void setState(OTSessionState_t eState) { m_eState = eState; }
	virtual OT_INLINE OTSessionState_t getState(){ return m_eState; }
	virtual OT_INLINE uint64_t getId(){ return m_pWrappedSession->getId(); }
	virtual OTObjectWrapper<OTSessionInfo*> getInfo() = 0;

private:
	SipSession* m_pWrappedSession;
	OTSessionState_t m_eState;
};

//
//	OTSipSessionAV
//
class OTSipSessionAV : public OTSipSession, public OTDocStreamerCallback
{
public:
	OTSipSessionAV(CallSession** ppCallSessionToWrap, OTMediaType_t eMediaType, OTObjectWrapper<OTBridgeInfo*> oBridgeInfo, std::string strUserId, std::string strDisplayName, std::string strJobTitle);
	virtual ~OTSipSessionAV();
	virtual OT_INLINE const char* getObjectId() { return "OTSipSessionAV"; }


public:
	virtual OT_INLINE const CallSession* getWrappedCallSession(){ return dynamic_cast<const CallSession*>(getWrappedSession()); }
	virtual OT_INLINE const InviteSession* getWrappedInviteSession(){ return dynamic_cast<const InviteSession*>(getWrappedSession()); }
	virtual void setState(OTSessionState_t eState); // @Override
	virtual OT_INLINE OTObjectWrapper<OTSessionInfo*> getInfo() { return dynamic_cast<OTSessionInfo*>(*m_oInfo); }
	virtual OT_INLINE OTObjectWrapper<OTDocStreamer*> getDocStreamer() { return m_oDocStreamer; }
	virtual bool presentationShare(std::string strFilePath);
	virtual bool handleIncomingData(const char* pcContentType, const void* pcDataPtr, const size_t nDataSize);

	// @Override (OTDocStreamerCallback)
	virtual void onStateChanged(OTDocStreamerState_t eState, OTDocStreamer* oStreamer) const;
	virtual void onError(bool bIsFatal, OTDocStreamer* oStreamer) const;

private:
	bool presentationSendInfo(const char* pcState) const;

private:
	OTMediaType_t m_eMediaType;
	OTObjectWrapper<OTSessionInfoAV*> m_oInfo;
	OTObjectWrapper<OTDocStreamer*> m_oDocStreamer;
};


// 
//	OTSipCallback
//
class OTSipCallback : public OTObject, public SipCallback
{	
public:
	OTSipCallback(OTObjectWrapper<OTEngine*> oEngine = NULL);
	virtual ~OTSipCallback();
	virtual OT_INLINE const char* getObjectId() { return "OTSipCallback"; }
	virtual OT_INLINE void setEngine(OTObjectWrapper<OTEngine*> oEngine) { m_oEngine = oEngine; }

	// SipCallback override
	virtual int OnRegistrationEvent(const RegistrationEvent* e);
	virtual int OnInviteEvent(const InviteEvent* e);
	virtual int OnDialogEvent(const DialogEvent* e);

private:
	bool processSessionTerminated(uint64_t u64SessionId);
	bool rejectCall(const CallSession* pcCallSession, short nCode, const char* pcPhrase);

private:
	OTObjectWrapper<OTEngine*> m_oEngine;
	// mapping <session, bridge>
	std::map<uint64_t, std::string> m_oMapSession2Bridge;
};


//
//	OTSipStack
//
class OTSipStack : public OTObject
{
public:
	OTSipStack(OTObjectWrapper<OTSipCallback*> oCallback, const char* pcRealmUri, const char* pcPrivateId, const char* pcPublicId);
	virtual ~OTSipStack();
	virtual OT_INLINE const char* getObjectId() { return "OTSipStack"; }
	virtual OT_INLINE const SipStack* getWrappedStack(){ return m_pStack; }
	bool isValid();
	static OTObjectWrapper<OTSipStack*> New(OTObjectWrapper<OTSipCallback*> oCallback, const char* pcRealmUri, const char* pcPrivateId, const char* pcPublicId);

private:
	SipStack* m_pStack;
};

#endif /* OPENTELEPRESENCE_WRAP_H */
