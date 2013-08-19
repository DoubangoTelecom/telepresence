/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#include "opentelepresence/OTSessionInfo.h"

#include "ProxyPluginMgr.h"
#include "MediaSessionMgr.h"
#include "SipSession.h"

#include "tsk_debug.h"

#include <assert.h>

//
//	OTSessionInfo
//
OTSessionInfo::OTSessionInfo(OTMediaType_t eMediaType, OTObjectWrapper<OTBridgeInfo*> oBridgeInfo)
{
	m_eMediaType = eMediaType;
	m_oBridgeInfo = oBridgeInfo;
	m_eRole = OTRole_Participant;
	m_bSpeaker = false;
}

OTSessionInfo::~OTSessionInfo()
{
	OT_DEBUG_INFO("*** OTSessionInfo destroyed ***");
}


//
//	OTSessionInfoAV
//
OTSessionInfoAV::OTSessionInfoAV(OTMediaType_t eMediaType, OTObjectWrapper<OTBridgeInfo*> oBridgeInfo)
: OTSessionInfo(eMediaType, oBridgeInfo)

, m_eNegotiatedCodecs(OTCodecType_None)

, m_nSipSessionId(OPENTELEPRESENCE_INVALID_ID)

, m_nAudioSessionId(OPENTELEPRESENCE_INVALID_ID)
, m_nAudioConsumerId(OPENTELEPRESENCE_INVALID_ID)
, m_nAudioProducerId(OPENTELEPRESENCE_INVALID_ID)

, m_nVideoSessionId(OPENTELEPRESENCE_INVALID_ID)
, m_nVideoConsumerId(OPENTELEPRESENCE_INVALID_ID)
, m_nVideoProducerId(OPENTELEPRESENCE_INVALID_ID)

, m_nDocStreamerId(OPENTELEPRESENCE_INVALID_ID)

, m_bMuteRemote(false)

, m_pcWrappedMediaSessionMgr(NULL)
{
	OT_ASSERT((eMediaType & OTMediaType_AudioVideo));
}


OTSessionInfoAV::~OTSessionInfoAV()
{
	OT_DEBUG_INFO("*** OTSessionInfoAV destroyed ***");

	detach();
}

bool OTSessionInfoAV::attach(const CallSession *pcCallSession)
{
	if(!pcCallSession)
	{
		OT_DEBUG_ERROR("Invalid parameter");
		return false;
	}

	CallSession *pCallSession = const_cast<CallSession*>(pcCallSession);
	const MediaSessionMgr* pcMediaSessionMgr;
	const tmedia_session_mgr_t* pcWrappedMediaSessionMgr;

	if(!(pcMediaSessionMgr = pCallSession->getMediaMgr()) || !(pcWrappedMediaSessionMgr = pcMediaSessionMgr->getWrappedMgr()))
	{
		OT_DEBUG_ERROR("Wrapped media session manager is NULL");
		return false;
	}

	const ProxyPlugin* pcProxyPlugin;

	OTMediaType_t eMediaType = OTMediaType_None;
	if(pcWrappedMediaSessionMgr->type & tmedia_audio)
	{
		eMediaType = (OTMediaType_t)(eMediaType|OTMediaType_Audio);
	}
	if(pcWrappedMediaSessionMgr->type & tmedia_video)
	{
		eMediaType = (OTMediaType_t)(eMediaType | OTMediaType_Video);
	}

	OT_ASSERT((eMediaType & OTMediaType_AudioVideo));

	// detach(): will destroy wrapped media session manager
	detach();

	m_eMediaType = eMediaType;
	m_pcWrappedMediaSessionMgr = pcWrappedMediaSessionMgr; // MUST not take ownership
	setSipSessionId(pcCallSession->getId());
	setNegotiatedCodecs((OTCodec_Type_t)pCallSession->getNegotiatedCodecs());

	// audio
	if(haveAudio())
	{
		setAudioSessionId(pcMediaSessionMgr->getSessionId(twrap_media_audio));
		if((pcProxyPlugin = pcMediaSessionMgr->findProxyPluginConsumer(twrap_media_audio)))
		{
			setAudioConsumerId(pcProxyPlugin->getId());
		}
		if((pcProxyPlugin = pcMediaSessionMgr->findProxyPluginProducer(twrap_media_audio)))
		{
			setAudioProducerId(pcProxyPlugin->getId());
		}
	}

	// video
	if(haveVideo())
	{
		setVideoSessionId(pcMediaSessionMgr->getSessionId(twrap_media_video));
		if((pcProxyPlugin = pcMediaSessionMgr->findProxyPluginConsumer(twrap_media_video)))
		{
			setVideoConsumerId(pcProxyPlugin->getId());
		}
		if((pcProxyPlugin = pcMediaSessionMgr->findProxyPluginProducer(twrap_media_video)))
		{
			setVideoProducerId(pcProxyPlugin->getId());
		}
	}

	return true;
}

bool OTSessionInfoAV::detach()
{
	m_pcWrappedMediaSessionMgr = NULL; // not owner

	return true;
}

uint64_t OTSessionInfoAV::getSessionId(OTMediaType_t eMediaType)
{
	switch(eMediaType)
	{
		case OTMediaType_Audio:
			{
				return getAudioSessionId();
			}
		case OTMediaType_Video:
			{
				return getVideoSessionId();
			}
		default:
			{
				OT_DEBUG_ERROR("%d is invalid as media type for a session", eMediaType);
				return OPENTELEPRESENCE_INVALID_ID;
			}
	}
}

uint64_t OTSessionInfoAV::getConsumerId(OTMediaType_t eMediaType)
{
	switch(eMediaType)
	{
		case OTMediaType_Audio:
			{
				return getAudioConsumerId();
			}
		case OTMediaType_Video:
			{
				return getVideoConsumerId();
			}
		default:
			{
				OT_DEBUG_ERROR("%d is invalid as media type for a consumer", eMediaType);
				return OPENTELEPRESENCE_INVALID_ID;
			}
	}
}

uint64_t OTSessionInfoAV::getProducerId(OTMediaType_t eMediaType)
{
	switch(eMediaType)
	{
		case OTMediaType_Audio:
			{
				return getAudioProducerId();
			}
		case OTMediaType_Video:
			{
				return getVideoProducerId();
			}
		default:
			{
				OT_DEBUG_ERROR("%d is invalid as media type for a producer", eMediaType);
				return OPENTELEPRESENCE_INVALID_ID;
			}
	}
}

OTObjectWrapper<OTSessionInfoAV *> OTSessionInfoAV::New(OTObjectWrapper<OTBridgeInfo*> oBridgeInfo, const CallSession *pcCallSession)
{
	const MediaSessionMgr* pcMediaSessionMgr = NULL;
	const tmedia_session_mgr_t* pcWrappedMediaSessionMgr = NULL;
	OT_ASSERT(pcCallSession);
	OT_ASSERT((pcMediaSessionMgr = const_cast<CallSession*>(pcCallSession)->getMediaMgr()) && (pcWrappedMediaSessionMgr = pcMediaSessionMgr->getWrappedMgr()));

	OTMediaType_t eMediaType = OTMediaType_None;
	if(pcWrappedMediaSessionMgr->type & tmedia_audio)
	{
		eMediaType = (OTMediaType_t)(eMediaType|OTMediaType_Audio);
	}
	if(pcWrappedMediaSessionMgr->type & tmedia_video)
	{
		eMediaType = (OTMediaType_t)(eMediaType | OTMediaType_Video);
	}
	
	OTObjectWrapper<OTSessionInfoAV *> oSessionInfo = new OTSessionInfoAV(eMediaType, oBridgeInfo);
	OT_ASSERT(oSessionInfo && oSessionInfo->attach(pcCallSession));	

	return oSessionInfo;
}
