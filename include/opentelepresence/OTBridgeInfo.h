/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_BRIDGE_INFO_H
#define OPENTELEPRESENCE_BRIDGE_INFO_H

#include "OpenTelepresenceConfig.h"
#include "opentelepresence/OTObject.h"
#include "opentelepresence/OTCommon.h"

#include <string>

class OTEngineInfo;
class OTBridge;

class OTBridgeInfo : public OTObject
{
public:
	OTBridgeInfo(std::string strId, OTObjectWrapper<OTEngineInfo*> oEngineInfo);
	virtual ~OTBridgeInfo();
	virtual OT_INLINE const char* getObjectId() { return "OTBridge"; }

	OT_INLINE std::string getId() { return m_strBridgeId; }

	OTObjectWrapper<OTBridge*> getBridge();

	OT_INLINE uint64_t getSpeakerSipSessionId() { return m_nSpeakerSipSessionId; }
	OT_INLINE void setSpeakerSipSessionId(uint64_t nId) { m_nSpeakerSipSessionId = nId; }

	OT_INLINE void setRecordEnabled(bool bRecord){  m_bRecord = bRecord; }
	OT_INLINE bool isRecordEnabled(){ return  m_bRecord; }

	OT_INLINE uint32_t getVideoWidth() { return m_nVideoWidth; }
	OT_INLINE void setVideoWidth(uint32_t nWidth) { m_nVideoWidth = nWidth; }

	OT_INLINE uint32_t getVideoHeight() { return m_nVideoHeight; }
	OT_INLINE void setVideoHeight(uint32_t nHeight) { m_nVideoHeight = nHeight; }

	OT_INLINE void getVideoSpeakerPAR(size_t &nNum, size_t &nDen) { nNum = m_parSpeaker.nNumerator; nDen = m_parSpeaker.nDenominator; }

	OT_INLINE void getVideoListenerPAR(size_t &nNum, size_t &nDen) { nNum = m_parListener.nNumerator; nDen = m_parListener.nDenominator; }

	OT_INLINE uint32_t getVideoFps() { return m_nVideoFps; }
	OT_INLINE void setVideoFps(uint32_t nFps) { m_nVideoFps = nFps; }
	
	OT_INLINE uint32_t getVideoGopSizeInSec(){ return m_nVideoGopSizeInSec; }
	OT_INLINE void setVideoGopSizeInSec(uint32_t nGopSizeInSec){ m_nVideoGopSizeInSec = nGopSizeInSec; }

	OT_INLINE uint32_t getVideoMotionRank(){ return m_nVideoMotionRank; }
	OT_INLINE void setVideoMotionRank(uint32_t nVideoMotionRank){ m_nVideoMotionRank = nVideoMotionRank; }

	OT_INLINE OTDimension_t getVideoDimension(){ return m_eVideoDim; }

	OT_INLINE std::string getRecordFileExt(){ return m_strRecordFileExt; }

	OT_INLINE uint16_t getAudioBitsPerSample(){ return m_nAudioBitsPerSample; }
	OT_INLINE uint16_t getAudioChannels(){ return m_nAudioChannels; }
	OT_INLINE uint16_t getAudioSampleRate(){ return m_nAudioSampleRate; }
	OT_INLINE uint16_t getAudioPtime(){ return m_nAudioPtime; }
	OT_INLINE uint32_t getAudioMaxLatency(){ return m_nAudioMaxLatency; }
	OT_INLINE float getAudioVolume(){ return m_fAudioVolume; }
	OT_INLINE OTDimension_t getAudioDimension(){ return m_eAudioDim; }
	OT_INLINE bool isAudioLoopback(){ return m_bAudioLoopback; }

	OT_INLINE std::string getOverlayFontsFolderPath(){ return m_strOverlayFontsFolderPath; }
	OT_INLINE std::string getOverlayCopyrightText(){ return m_strOverlayCopyrightText; }
	OT_INLINE std::string getOverlayWatermarkImagePath(){ return m_strOverlayWatermarkImagePath; }
	OT_INLINE std::string getOverlayCopyrightFontFileName(){ return m_strOverlayCopyrightFontFileName; }
	OT_INLINE std::string getOverlaySpeakerNameFontFileName(){ return m_strOverlaySpeakerNameFontFileName; }
	OT_INLINE size_t getOverlayCopyrightFontSize(){ return m_nOverlayCopyrightFontSize; }
	OT_INLINE size_t getOverlaySpeakerNameFontSize(){ return m_nOverlaySpeakerNameFontSize; }
	OT_INLINE bool isOverlayDisplaySpeakerName(){ return m_bOverlayDisplaySpeakerName; }
	OT_INLINE bool isOverlayDisplaySpeakerJobTitle(){ return m_bOverlayDisplaySpeakerJobTitle; }

	OT_INLINE bool isPresentationSharingEnabled() { return m_bPresentationSharingEnabled; }
	OT_INLINE std::string getPresentationSharingBaseFolder() { return m_strPresentationSharingBaseFolder; }
	OT_INLINE unsigned short getPresentationSharingLocalPort() { return m_nPresentationSharingLocalPort; }
	OT_INLINE std::string getPresentationSharingAppPath() { return m_strPresentationSharingAppPath; }

private:
	uint64_t m_uEngineId;
	std::string m_strBridgeId, m_strRecordFileExt;
	uint64_t m_nSpeakerSipSessionId;
	bool m_bRecord;
	uint32_t m_nVideoWidth;
	uint32_t m_nVideoHeight;
	uint32_t m_nVideoFps;
	uint32_t m_nVideoGopSizeInSec;
	OTRatio_t m_parSpeaker;
	OTRatio_t m_parListener;
	OTDimension_t m_eVideoDim;
	uint32_t m_nVideoMotionRank;
	uint16_t m_nAudioBitsPerSample;
	uint16_t m_nAudioChannels;
	uint16_t m_nAudioSampleRate;
	uint16_t m_nAudioPtime;
	uint32_t m_nAudioMaxLatency;
	float m_fAudioVolume;
	bool m_bAudioLoopback;
	OTDimension_t m_eAudioDim;
	std::string m_strOverlayFontsFolderPath;
	std::string m_strOverlayCopyrightText;
	std::string m_strOverlayWatermarkImagePath;
	std::string m_strOverlayCopyrightFontFileName;
	std::string m_strOverlaySpeakerNameFontFileName;
	size_t m_nOverlayCopyrightFontSize;
	size_t m_nOverlaySpeakerNameFontSize;
	bool m_bOverlayDisplaySpeakerName;
	bool m_bOverlayDisplaySpeakerJobTitle;
	bool m_bPresentationSharingEnabled;
	std::string m_strPresentationSharingBaseFolder;
	unsigned short m_nPresentationSharingLocalPort;
	std::string m_strPresentationSharingAppPath;
};
#endif /* OPENTELEPRESENCE_BRIDGE_INFO_H */
