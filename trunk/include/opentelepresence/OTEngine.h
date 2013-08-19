#ifndef OPENTELEPRESENCE_ENGINE_H
#define OPENTELEPRESENCE_ENGINE_H

#include "OpenTelepresenceConfig.h"

#include "opentelepresence/OTObject.h"
#include "opentelepresence/OTBridge.h"
#include "opentelepresence/OTWrap.h"
#include "opentelepresence/OTMutex.h"
#include "opentelepresence/OTProcess.h"
#include "opentelepresence/cfg/OTCfg.h"
#include "opentelepresence/nettransport/OTHttpTransport.h"

#include <map>
#include <list>
#include <string>

class OTEngine;


class OTEngineInfo : public OTObject
{
	friend class OTSipCallback;
	friend class OTEngine;
	friend class OTBridgeInfo;
public:
	OTEngineInfo(uint64_t uId)
		: m_uId(uId)
		, m_bAcceptIncomingSipReg(OPENTELEPRESENCE_ACCEPT_INCOMING_SIPREG)
		, m_bRecordEnabled(OPENTELEPRESENCE_RECORD_ENABLED)
		, m_strRecordFileExt(OPENTELEPRESENCE_RECORD_FILE_EXT)
		, m_nMixedVideoWidth(OPENTELEPRESENCE_VIDEO_WIDTH_DEFAULT)
		, m_nMixedVideoHeight(OPENTELEPRESENCE_VIDEO_HEIGHT_DEFAULT)
		, m_nVideoMotionRank(OPENTELEPRESENCE_VIDEO_MOTION_RANK_DEFAULT)
		, m_nVideoFps(OPENTELEPRESENCE_VIDEO_FPS_DEFAULT)

		, m_nAudioChannels(OPENTELEPRESENCE_AUDIO_CHANNELS_DEFAULT)
		, m_nAudioBitsPerSample(OPENTELEPRESENCE_AUDIO_BITS_PER_SAMPLE_DEFAULT)
		, m_nAudioSampleRate(OPENTELEPRESENCE_AUDIO_RATE_DEFAULT)
		, m_nAudioPtime(OPENTELEPRESENCE_AUDIO_PTIME_DEFAULT)
		, m_nAudioMaxLatency(OPENTELEPRESENCE_AUDIO_MAX_LATENCY)
		, m_fAudioVolume(OPENTELEPRESENCE_AUDIO_MIXER_VOL)
		, m_eAudioDim(OPENTELEPRESENCE_AUDIO_MIXER_DIM)
		, m_bAudioLoopback(OPENTELEPRESENCE_AUDIO_LOOPBACK_DEFAULT)

		, m_strOverlayFontsFolderPath(OPENTELEPRESENCE_VIDEO_OVERLAY_PATH_FONTFOLDER)
		, m_strOverlayCopyrightText(OPENTELEPRESENCE_VIDEO_OVERLAY_TEXT_COPYRIGTH)
		, m_strOverlayWatermarkImagePath(OPENTELEPRESENCE_VIDEO_OVERLAY_PATH_WATERMARK)
		, m_strOverlayCopyrightFontFileName(OPENTELEPRESENCE_VIDEO_OVERLAY_FILENAME_FONT_COPYRIGHT)
		, m_strOverlaySpeakerNameFontFileName(OPENTELEPRESENCE_VIDEO_OVERLAY_FILENAME_FONT_SPEAKER_NAME)
		, m_nOverlayCopyrightFontSize(OPENTELEPRESENCE_VIDEO_OVERLAY_FONTSIZE_COPYRIGTH)
		, m_nOverlaySpeakerNameFontSize(OPENTELEPRESENCE_VIDEO_OVERLAY_FONTSIZE_NAME_SPEAKER)
		, m_bOverlayDisplaySpeakerName(OPENTELEPRESENCE_VIDEO_OVERLAY_DISPLAY_SPEAKER_NAME)
		, m_bOverlayDisplaySpeakerJobTitle(OPENTELEPRESENCE_VIDEO_OVERLAY_DISPLAY_SPEAKER_JOBTITLE)

		, m_bPresentationSharingEnabled(OPENTELEPRESENCE_PRESENTATION_SHARING_ENABLED)
		, m_strPresentationSharingBaseFolder(OPENTELEPRESENCE_PRESENTATION_SHARING_BASE_FOLDER)
		, m_nPresentationSharingLocalPort(OPENTELEPRESENCE_PRESENTATION_SHARING_PROCESS_LOCAL_PORT)
		, m_strPresentationSharingAppPath(OPENTELEPRESENCE_PRESENTATION_SHARING_APP_PATH)

		, m_bSSLMutualAuth(false)
	{
		m_parSpeaker.nNumerator = OPENTELEPRESENCE_VIDEO_PAR_SPEAKER_NUM;
		m_parSpeaker.nDenominator = OPENTELEPRESENCE_VIDEO_PAR_SPEAKER_DEN;
		m_parListener.nNumerator = OPENTELEPRESENCE_VIDEO_PAR_LISTENER_NUM;
		m_parListener.nDenominator = OPENTELEPRESENCE_VIDEO_PAR_LISTENER_DEN;
	}
	virtual ~OTEngineInfo(){}
	virtual OT_INLINE const char* getObjectId() { return "OTEngineInfo"; }

	virtual OT_INLINE uint64_t getId() { return m_uId; }

private:
	uint64_t m_uId;
	bool m_bAcceptIncomingSipReg, m_bRecordEnabled;
	size_t m_nMixedVideoWidth, m_nMixedVideoHeight;
	OTRatio_t m_parSpeaker, m_parListener;
	size_t m_nVideoMotionRank, m_nVideoFps;
	std::string m_strRecordFileExt;
	std::string m_strOverlayFontsFolderPath, m_strOverlayCopyrightText, m_strOverlayWatermarkImagePath, m_strOverlayCopyrightFontFileName, m_strOverlaySpeakerNameFontFileName;
	size_t m_nOverlayCopyrightFontSize, m_nOverlaySpeakerNameFontSize;
	bool m_bOverlayDisplaySpeakerName, m_bOverlayDisplaySpeakerJobTitle;

	uint16_t m_nAudioChannels, m_nAudioBitsPerSample, m_nAudioSampleRate, m_nAudioPtime;
	uint32_t m_nAudioMaxLatency;
	float m_fAudioVolume;
	OTDimension_t m_eAudioDim;
	bool m_bAudioLoopback;

	bool m_bPresentationSharingEnabled;
	std::string m_strPresentationSharingBaseFolder;
	unsigned short m_nPresentationSharingLocalPort;
	std::string m_strPresentationSharingAppPath;

	std::string m_strSSLPrivateKey, m_strSSLPublicKey, m_strSSLCA;
	bool m_bSSLMutualAuth;
};

class OTEngine : public OTObject
{
	friend class OTSipCallback;
protected:
	OTEngine();
public:
	virtual ~OTEngine();
	virtual OT_INLINE const char* getObjectId() { return "OTEngine"; }
	virtual OT_INLINE bool isStarted(){ return m_bStarted; }
	virtual OT_INLINE OTObjectWrapper<OTEngineInfo*> getInfo(){ return m_oInfo; }

	virtual bool start();
	virtual bool stop();

	OTObjectWrapper<OTBridge*> getBridge(std::string strId);

	virtual bool setDebugLevel(const char* pcLevel);
	virtual bool addTransport(const char* pcTransport, uint16_t nLocalPort, const char* pcLocalIP = tsk_null, const char* pcIPVersion = tsk_null);
	virtual bool setRtpSymmetricEnabled(bool bEnabled);
	virtual bool setRtcpMuxEnabled(bool bEnabled);
	virtual bool setIceEnabled(bool bEnabled);
	virtual bool setIceStunEnabled(bool bEnabled);
	virtual bool setStunServer(const char* pcServerHost, unsigned short nServerPort, const char* pcUserName = NULL, const char* pcUserPwd = NULL);
	virtual bool setRtpBufferSize(size_t nBufferSize);
	virtual bool setAvpfTailLength(size_t nMin, size_t nMax);
	virtual bool setCongestionCtrlEnabled(bool bEnabled);
	virtual bool setBandwidthVideoUploadMax(int32_t nBandwidthVideoUploadMax);
	virtual bool setBandwidthVideoDownloadMax(int32_t nBandwidthVideoDownloadMax);
	virtual bool setVideoMotionRank(int32_t nVideoMotionRank);
	virtual bool setVideoFps(int32_t nVideoFps);
	virtual bool setVideoJbEnabled(bool bEnabled);
	virtual bool setVideoZeroArtifactsEnabled(bool bEnabled);
	virtual bool setVideoMixedSize(std::string strPrefVideoSize);
	virtual bool setVideoSpeakerPAR(size_t nNum, size_t nDen);
	virtual bool setVideoListenerPAR(size_t nNum, size_t nDen);
	virtual bool setAcceptIncomingSipReg(bool bAcceptIncomingSipReg);
	virtual bool setAudioChannels(int32_t nChannels);
	virtual bool setAudioBitsPerSample(int32_t nBitsPerSample);
	virtual bool setAudioSampleRate(int32_t nSampleRate);
	virtual bool setAudioPtime(int32_t nPtime);
	virtual bool setAudioVolume(float fVolume);
	virtual bool setAudioDim(std::string strDim);
	virtual bool setAudioMaxLatency(int32_t nMaxLatency);
	virtual bool setAudioLoopback(bool bLoopback);
	virtual bool setRecordEnabled(bool bEnabled);
	virtual bool setRecordFileExt(std::string strRecordFileExt);
	virtual bool setCodecs(const char* pcCodecs);
	virtual bool setCodecOpusMaxRates(int32_t nPlaybackMaxRate, int32_t nCaptureMaxRate);
	virtual bool setOverlayFontsFolderPath(std::string strPath);
	virtual bool setOverlayCopyrightText(std::string strText);
	virtual bool setOverlayWatermarkImagePath(std::string strPath);
	virtual bool setOverlayCopyrightFontSize(size_t nFontSize);
	virtual bool setOverlayCopyrightFontFileName(std::string strFileName);
	virtual bool setOverlaySpeakerNameFontSize(size_t nFontSize);
	virtual bool setOverlaySpeakerNameFontFileName(std::string strFileName);
	virtual bool setOverlayDisplaySpeakerName(bool bDisplay);
	virtual bool setOverlayDisplaySpeakerJobTitle(bool bDisplay);
	virtual bool setSSLCertificates(std::string strPrivateKey, std::string strPublicKey, std::string strCA, bool bMutualAuth = false);
	virtual bool setSRTPMode(std::string strMode);
	virtual bool setSRTPType(std::string strTypesCommaSep);
	virtual bool setPresentationSharingEnabled(bool bPresentationSharingEnabled);
	virtual bool setPresentationSharingBaseFolder(std::string strPresentationSharingBaseFolder);
	virtual bool setPresentationSharingLocalPort(unsigned short nPresentationSharingLocalPort);
	virtual bool setPresentationSharingAppPath(std::string strPresentationSharingAppPath);
	virtual bool setConfFile(const char* pcConfFileFullPath);
	
	static OTObjectWrapper<OTEngine*> getEngine(uint64_t uId);
	static OTObjectWrapper<OTBridge*> getBridge(uint64_t uEngineId, std::string strBridgeId);
	static OTObjectWrapper<OTEngine*> New();

protected:
	virtual bool isValid() = 0;
	virtual OT_INLINE void setStarted(bool bStarted){ m_bStarted = bStarted; }

private:
	static bool OTCfgParserOnNewCfg(OTObjectWrapper<OTCfg*> oCfg, const void* pcCallbackData);

private:
	static bool g_bInitialized;
	static uint64_t g_uId;
	static std::map<uint64_t, OTEngine* > g_oEngines; // must not hold "OTObjectWrapper<OTEngine*>" because of circular ref()
	uint64_t m_uId;
	std::map<std::string, OTObjectWrapper<OTBridge*> > m_oBridges; /* < bridge-id, bridge-object > */
	std::map<std::string, OTMapOfCfgParams> m_oBridgeCfgs; /* < bridge-id, OTMapOfCfgParams > */
	OTObjectWrapper<OTSipCallback*> m_oSipCallback;
	OTObjectWrapper<OTSipStack*> m_oSipStack;
	OTObjectWrapper<OTMutex*> m_oMutex;

	OTObjectWrapper<OTCfgSection*> m_oCurrCfgSection;
	std::list<OTObjectWrapper<OTCfgSection *> > m_oCfgSections;

	std::list<OTObjectWrapper<OTHttpTransport*> > m_oHttpTransports;

	OTObjectWrapper<OTProcess*> m_oProcessPresShare3rdApp;

	OTObjectWrapper<OTEngineInfo*> m_oInfo;

	bool m_bStarted;
};

#if defined(_MSC_VER)
template class OTObjectWrapper<OTEngine*>;
#endif

#endif /* OPENTELEPRESENCE_ENGINE_H */

