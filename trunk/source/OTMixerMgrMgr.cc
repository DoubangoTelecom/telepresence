/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#include "opentelepresence/OTMixerMgrMgr.h"
#include "opentelepresence/OTMixerMgrAudio.h"
#include "opentelepresence/OTMixerMgrVideo.h"

#include <assert.h>

OTMixerMgrMgr::OTMixerMgrMgr(OTMediaType_t eMediaType, OTObjectWrapper<OTBridgeInfo*> oBridgeInfo)
:OTObject()
{
	m_pOTMixerMgrAudio = NULL;
	m_bValid = false;
	m_eMediaType = eMediaType;

	// create recorder if enabled
	if(oBridgeInfo->isRecordEnabled())
	{
		std::string strRecordFile(oBridgeInfo->getId() + "." + oBridgeInfo->getRecordFileExt());
		m_oRecorder = OTRecorder::New(strRecordFile, eMediaType);
	}

	if(m_eMediaType & OTMediaType_Audio)
	{
		OTObjectWrapper<OTMixerMgr*> pOTMixerMgr = OTMixerMgr::New(OTMediaType_Audio, oBridgeInfo);
		m_pOTMixerMgrAudio = dynamic_cast<OTMixerMgrAudio*>(*pOTMixerMgr);
		if(!m_pOTMixerMgrAudio)
		{
			OT_DEBUG_ERROR("Failed to create audio mixer...die()");
			OT_ASSERT(false);
		}
		if(m_oRecorder)
		{
			m_oRecorder->setAudioParams(oBridgeInfo->getAudioPtime(), oBridgeInfo->getAudioSampleRate(), oBridgeInfo->getAudioChannels());
			m_pOTMixerMgrAudio->setRecorder(m_oRecorder); // set recorder
		}
	}
	if(m_eMediaType & OTMediaType_Video)
	{
		OTObjectWrapper<OTMixerMgr*> pOTMixerMgr = OTMixerMgr::New(OTMediaType_Video, oBridgeInfo);
		m_pOTMixerMgrVideo = dynamic_cast<OTMixerMgrVideo*>(*pOTMixerMgr);
		OT_ASSERT(*m_pOTMixerMgrVideo);
		if(m_oRecorder)
		{
			m_oRecorder->setVideoParams(oBridgeInfo->getVideoWidth(), oBridgeInfo->getVideoHeight(), oBridgeInfo->getVideoMotionRank(), oBridgeInfo->getVideoGopSizeInSec(), oBridgeInfo->getVideoFps());
			m_pOTMixerMgrVideo->setRecorder(m_oRecorder); // set recorder
		}
	}

	m_bValid = (m_pOTMixerMgrAudio || m_pOTMixerMgrVideo);

	if(m_bValid)
	{
		if(m_oRecorder)
		{
			m_oRecorder->open(m_eMediaType);
		}
	}
}

OTMixerMgrMgr::~OTMixerMgrMgr()
{
	OT_DEBUG_INFO("*** OTMixerMgrMgr destroyed ***");
}

bool OTMixerMgrMgr::isValid()
{
	return m_bValid;
}

int OTMixerMgrMgr::start(OTMediaType_t eMediaType)
{
	int ret = 0;

	if((eMediaType & OTMediaType_Audio) && m_pOTMixerMgrAudio)
	{
		if((ret = m_pOTMixerMgrAudio->start()))
		{
			return ret;
		}
	}
	if((eMediaType & OTMediaType_Video) && m_pOTMixerMgrVideo)
	{
		if((ret = m_pOTMixerMgrVideo->start()))
		{
			return ret;
		}
	}
	return 0;
}

bool OTMixerMgrMgr::isStarted(OTMediaType_t eMediaType)
{
	return false;
}

int OTMixerMgrMgr::pause(OTMediaType_t eMediaType)
{
	int ret = 0;

	if((eMediaType & OTMediaType_Audio) && m_pOTMixerMgrAudio)
	{
		if((ret = m_pOTMixerMgrAudio->pause()))
		{
			return ret;
		}
	}
	if((eMediaType & OTMediaType_Video) && m_pOTMixerMgrVideo)
	{
		if((ret = m_pOTMixerMgrVideo->pause()))
		{
			return ret;
		}
	}
	return 0;
}

bool OTMixerMgrMgr::isPaused(OTMediaType_t eMediaType)
{
	return false;
}

int OTMixerMgrMgr::flush(OTMediaType_t eMediaType)
{
	int ret = 0;

	if((eMediaType & OTMediaType_Audio) && m_pOTMixerMgrAudio)
	{
		if((ret = m_pOTMixerMgrAudio->flush()))
		{
			return ret;
		}
	}
	if((eMediaType & OTMediaType_Video) && m_pOTMixerMgrVideo)
	{
		if((ret = m_pOTMixerMgrVideo->flush()))
		{
			return ret;
		}
	}
	return 0;
}

int OTMixerMgrMgr::stop(OTMediaType_t eMediaType)
{
	int ret = 0;

	if((eMediaType & OTMediaType_Audio) && m_pOTMixerMgrAudio)
	{
		if((ret = m_pOTMixerMgrAudio->stop()))
		{
			return ret;
		}
	}
	if((eMediaType & OTMediaType_Video) && m_pOTMixerMgrVideo)
	{
		if((ret = m_pOTMixerMgrVideo->stop()))
		{
			return ret;
		}
	}
	return 0;
}

int OTMixerMgrMgr::attachMediaPlugins(OTObjectWrapper<OTSessionInfo*> pOTSessionInfo)
{
	int ret = 0;
	if(m_pOTMixerMgrAudio && pOTSessionInfo->haveAudio())
	{
		OT_ASSERT(dynamic_cast<OTSessionInfoAV*>(*pOTSessionInfo));
		if((ret = m_pOTMixerMgrAudio->attachMediaPlugins(pOTSessionInfo)))
		{
			return ret;
		}
	}
	if(m_pOTMixerMgrVideo && pOTSessionInfo->haveVideo())
	{
		OT_ASSERT(dynamic_cast<OTSessionInfoAV*>(*pOTSessionInfo));
		if((ret = m_pOTMixerMgrVideo->attachMediaPlugins(pOTSessionInfo)))
		{
			return ret;
		}
	}

	return ret;
}

int OTMixerMgrMgr::deAttachMediaPlugins(OTObjectWrapper<OTSessionInfo*> pOTSessionInfo)
{
	int ret = 0;
	if(m_pOTMixerMgrAudio && pOTSessionInfo->haveAudio())
	{
		OT_ASSERT(dynamic_cast<OTSessionInfoAV*>(*pOTSessionInfo));
		if((ret = m_pOTMixerMgrAudio->deAttachMediaPlugins(pOTSessionInfo)))
		{
			return ret;
		}
	}
	if(m_pOTMixerMgrVideo && pOTSessionInfo->haveVideo())
	{
		OT_ASSERT(dynamic_cast<OTSessionInfoAV*>(*pOTSessionInfo));
		if((ret = m_pOTMixerMgrVideo->deAttachMediaPlugins(pOTSessionInfo)))
		{
			return ret;
		}
	}

	return 0;
}

OTObjectWrapper<OTMixerMgrMgr*> OTMixerMgrMgr::New(OTMediaType_t eMediaType, OTObjectWrapper<OTBridgeInfo*> oBridgeInfo)
{
	OTObjectWrapper<OTMixerMgrMgr*> pOTMixerMgr = new OTMixerMgrMgr(eMediaType, oBridgeInfo);
	if(pOTMixerMgr && !pOTMixerMgr->isValid())
	{
		OTObjectSafeRelease(pOTMixerMgr);
	}
	return pOTMixerMgr;
}
