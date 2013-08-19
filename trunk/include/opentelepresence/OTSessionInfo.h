/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_SESSIONINFO_H
#define OPENTELEPRESENCE_SESSIONINFO_H

#include "OpenTelepresenceConfig.h"

#include "opentelepresence/OTObject.h"
#include "opentelepresence/OTCommon.h"
#include "opentelepresence/OTRole.h"
#include "opentelepresence/OTCodec.h"
#include "opentelepresence/OTBridgeInfo.h"

#include <string>

class CallSession;

//
//	OTSessionInfo
//
class OTSessionInfo : public OTObject
{
public:
	OTSessionInfo(OTMediaType_t eMediaType, OTObjectWrapper<OTBridgeInfo*> oBridgeInfo);
	virtual ~OTSessionInfo();
	OT_INLINE const char* getObjectId() { return "OTSessionInfo"; }

	OT_INLINE  OTMediaType_t getMediaType() { return m_eMediaType; }
	OT_INLINE std::string getBridgeId() { return m_oBridgeInfo->getId(); }
	OT_INLINE OTObjectWrapper<OTBridgeInfo*> getBridgeInfo() { return m_oBridgeInfo; }
	OT_INLINE bool haveAudio() { return (m_eMediaType & OTMediaType_Audio) != 0; }
	OT_INLINE bool haveVideo() { return (m_eMediaType & OTMediaType_Video) != 0; }
	OT_INLINE bool haveAudioVideo() { return haveAudio() && haveVideo(); }
	OT_INLINE OTRole_t getRole() { return m_eRole; }
	OT_INLINE void setRole(OTRole_t eRole) { m_eRole = eRole; }
	OT_INLINE bool isSpeaker() { return m_bSpeaker; }
	OT_INLINE void setSpeaker(bool bSpeaker) { m_bSpeaker = bSpeaker; }
	OT_INLINE bool isSpeaking() { return m_bSpeaking; }
	OT_INLINE void setSpeaking(bool bSpeaking) { m_bSpeaking = bSpeaking; }
	OT_INLINE void setDisplayName(std::string strDisplayName){ m_strDisplayName = strDisplayName; }
	OT_INLINE std::string getDisplayName(){ return m_strDisplayName; }
	OT_INLINE void setUserId(std::string strUserId){ m_strUserId = strUserId; }
	OT_INLINE std::string getUserId(){ return m_strUserId; }
	OT_INLINE void setJobTitle(std::string strJobTitle){ m_strJobTitle = strJobTitle; }
	OT_INLINE std::string getJobTitle(){ return m_strJobTitle; }
	OT_INLINE void setAudioVelocity(float x, float y, float z){ m_AudioVelocity.x = x; m_AudioVelocity.y = y; m_AudioVelocity.z = z;}
	OT_INLINE const OT3f_t* getAudioVelocity(){ return &m_AudioVelocity; }
	OT_INLINE void setAudioPosition(float x, float y, float z){ m_AudioPosition.x = x; m_AudioPosition.y = y; m_AudioPosition.z = z;}
	OT_INLINE const OT3f_t* getAudioPosition(){ return &m_AudioPosition; }

	virtual uint64_t getSessionId(OTMediaType_t eMediaType)=0;
	virtual uint64_t getSipSessionId()=0;
	virtual uint64_t getConsumerId(OTMediaType_t eMediaType)=0;
	virtual uint64_t getProducerId(OTMediaType_t eMediaType)=0;
	virtual OTCodec_Type_t getNegotiatedCodecs()=0;

protected:
	OTMediaType_t m_eMediaType;
	OTObjectWrapper<OTBridgeInfo*> m_oBridgeInfo;
	OTRole_t m_eRole;
	std::string m_strDisplayName;
	std::string m_strUserId;
	std::string m_strJobTitle;
	bool m_bSpeaker;
	bool m_bSpeaking;
	OT3f_t m_AudioVelocity, m_AudioPosition;
};

//
//	OTSessionInfoAV
//
class OTSessionInfoAV : public OTSessionInfo
{
public:
	OTSessionInfoAV(OTMediaType_t eMediaType, OTObjectWrapper<OTBridgeInfo*> oBridgeInfo);
	virtual ~OTSessionInfoAV();
	OT_INLINE const char* getObjectId() { return "OTSessionInfoAV"; }

	virtual uint64_t getSessionId(OTMediaType_t eMediaType);
	virtual uint64_t getConsumerId(OTMediaType_t eMediaType);
	virtual uint64_t getProducerId(OTMediaType_t eMediaType);

	OT_INLINE uint64_t getSipSessionId() { return m_nSipSessionId; }
	OT_INLINE void setSipSessionId(uint64_t nId) { m_nSipSessionId = nId; }

	OT_INLINE uint64_t getAudioSessionId() { return m_nAudioSessionId; }
	OT_INLINE void setAudioSessionId(uint64_t nId) { m_nAudioSessionId = nId; }
	OT_INLINE uint64_t getAudioConsumerId() { return m_nAudioConsumerId; }
	OT_INLINE void setAudioConsumerId(uint64_t nId) { m_nAudioConsumerId = nId; }
	OT_INLINE uint64_t getAudioProducerId() { return m_nAudioProducerId; }
	OT_INLINE void setAudioProducerId(uint64_t nId) { m_nAudioProducerId = nId; }

	OT_INLINE uint64_t getVideoSessionId() { return m_nVideoSessionId; }
	OT_INLINE void setVideoSessionId(uint64_t nId) { m_nVideoSessionId = nId; }
	OT_INLINE uint64_t getVideoConsumerId() { return m_nVideoConsumerId; }
	OT_INLINE void setVideoConsumerId(uint64_t nId) { m_nVideoConsumerId = nId; }
	OT_INLINE uint64_t getVideoProducerId() { return m_nVideoProducerId; }
	OT_INLINE void setVideoProducerId(uint64_t nId) { m_nVideoProducerId = nId; }

	OT_INLINE uint64_t getDocStreamerId() { return m_nDocStreamerId; }
	OT_INLINE void setDocStreamerId(uint64_t nId) { m_nDocStreamerId = nId; }

	OT_INLINE OTCodec_Type_t getNegotiatedCodecs() { return m_eNegotiatedCodecs; }
	OT_INLINE void setNegotiatedCodecs(OTCodec_Type_t eNegotiatedCodecs) { m_eNegotiatedCodecs = eNegotiatedCodecs; }

	OT_INLINE bool isMuteRemote() { return m_bMuteRemote; }
	OT_INLINE void setMuteRemote(bool bMute) { m_bMuteRemote = bMute; }

	OT_INLINE const struct tmedia_session_mgr_s* getWrappedMediaSessionMgr(){ return m_pcWrappedMediaSessionMgr; }

	bool attach(const CallSession *pcCallSession);
	bool detach();

	static OTObjectWrapper<OTSessionInfoAV *> New(OTObjectWrapper<OTBridgeInfo*> oBridgeInfo, const CallSession *pcCallSession);

private:
	uint64_t m_nSipSessionId;

	uint64_t m_nAudioSessionId;
	uint64_t m_nAudioConsumerId;
	uint64_t m_nAudioProducerId;

	uint64_t m_nVideoSessionId;
	uint64_t m_nVideoConsumerId;
	uint64_t m_nVideoProducerId;

	uint64_t m_nDocStreamerId;

	OTCodec_Type_t m_eNegotiatedCodecs;

	bool m_bMuteRemote; // put on mute by remote party

	const struct tmedia_session_mgr_s* m_pcWrappedMediaSessionMgr;
};

#endif /* OPENTELEPRESENCE_SESSIONINFO_H */
