/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#include "opentelepresence/OTBridgeInfo.h"
#include "opentelepresence/OTEngine.h"
#include "opentelepresence/OTBridge.h"

#include "tsk_debug.h"

OTBridgeInfo::OTBridgeInfo(std::string strId, OTObjectWrapper<OTEngineInfo*> oEngineInfo)
: m_uEngineId(oEngineInfo->getId())
, m_strBridgeId(strId)
, m_nSpeakerSipSessionId(0)
, m_bRecord(oEngineInfo->m_bRecordEnabled)
, m_strRecordFileExt(oEngineInfo->m_strRecordFileExt)
, m_nVideoWidth(oEngineInfo->m_nMixedVideoWidth)
, m_nVideoHeight(oEngineInfo->m_nMixedVideoHeight)
, m_parSpeaker(oEngineInfo->m_parSpeaker)
, m_parListener(oEngineInfo->m_parListener)
, m_nVideoFps(oEngineInfo->m_nVideoFps)
, m_nVideoGopSizeInSec(OPENTELEPRESENCE_VIDEO_GOP_SIZE_DEFAULT)
, m_nVideoMotionRank(oEngineInfo->m_nVideoMotionRank)
, m_eVideoDim(OPENTELEPRESENCE_VIDEO_MIXER_DIM)

, m_nAudioChannels(oEngineInfo->m_nAudioChannels)
, m_nAudioBitsPerSample(oEngineInfo->m_nAudioBitsPerSample)
, m_nAudioSampleRate(oEngineInfo->m_nAudioSampleRate)
, m_nAudioPtime(oEngineInfo->m_nAudioPtime)
, m_nAudioMaxLatency(oEngineInfo->m_nAudioMaxLatency)
, m_fAudioVolume(oEngineInfo->m_fAudioVolume)
, m_eAudioDim(oEngineInfo->m_eAudioDim)
, m_bAudioLoopback(oEngineInfo->m_bAudioLoopback)

, m_bPresentationSharingEnabled(oEngineInfo->m_bPresentationSharingEnabled)
, m_strPresentationSharingBaseFolder(oEngineInfo->m_strPresentationSharingBaseFolder)
, m_nPresentationSharingLocalPort(oEngineInfo->m_nPresentationSharingLocalPort)
, m_strPresentationSharingAppPath(oEngineInfo->m_strPresentationSharingAppPath)

, m_strOverlayFontsFolderPath(oEngineInfo->m_strOverlayFontsFolderPath)
, m_strOverlayCopyrightText(oEngineInfo->m_strOverlayCopyrightText)
, m_strOverlayWatermarkImagePath(oEngineInfo->m_strOverlayWatermarkImagePath)
, m_strOverlayCopyrightFontFileName(oEngineInfo->m_strOverlayCopyrightFontFileName)
, m_strOverlaySpeakerNameFontFileName(oEngineInfo->m_strOverlaySpeakerNameFontFileName)
, m_nOverlayCopyrightFontSize(oEngineInfo->m_nOverlayCopyrightFontSize)
, m_nOverlaySpeakerNameFontSize(oEngineInfo->m_nOverlaySpeakerNameFontSize)
, m_bOverlayDisplaySpeakerName(oEngineInfo->m_bOverlayDisplaySpeakerName)
, m_bOverlayDisplaySpeakerJobTitle(oEngineInfo->m_bOverlayDisplaySpeakerJobTitle)
{
	
}

OTBridgeInfo::~OTBridgeInfo()
{
	OT_DEBUG_INFO("*** OTBridgeInfo destroyed ***");
}

OTObjectWrapper<OTBridge*> OTBridgeInfo::getBridge()
{
	OTObjectWrapper<OTEngine*> oEngine = OTEngine::getEngine(m_uEngineId);
	if(oEngine)
	{
		return oEngine->getBridge(m_strBridgeId);
	}
	return NULL;
}
