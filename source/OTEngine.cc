/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#include "opentelepresence/OTEngine.h"

#include "opentelepresence/platforms/win32/OTEngineWin32.h"
#include "opentelepresence/platforms/osx/OTEngineOSX.h"
#include "opentelepresence/platforms/generic/OTEngineGen.h"
#include "opentelepresence/cfg/OTCfgParser.h"
#include "opentelepresence/docstreamer/OTDocStreamer.h"

#include "opentelepresence/OTProxyPluginMgr.h"

#include "SipStack.h"
#include "MediaSessionMgr.h"

#include "tsk_debug.h"

#include <assert.h>
#include <functional>
#include <algorithm>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
}

bool OTEngine::g_bInitialized = false;
uint64_t OTEngine::g_uId = 0;
std::map<uint64_t, OTEngine* > OTEngine::g_oEngines;

#if !defined(OT_SIP_REALM)
#	define OT_SIP_REALM "conf-call.org"
#endif
#if !defined(OT_SIP_IMPI)
#	define OT_SIP_IMPI "engine"
#endif
#if !defined(OT_SIP_IMPU)
#	define OT_SIP_IMPU "sip" OT_SIP_IMPI "@" OT_SIP_REALM
#endif

#define ot_list_count(list)					tsk_list_count((list), tsk_null, tsk_null)
#define ot_str_is(str, val)					tsk_striequals((const char*)(str), val)
#define ot_str_is_star(str)					ot_str_is((str), "*")
#define ot_str_is_yes(str)					ot_str_is((str), "yes")
#define ot_str_is_null_or_empty(str)		tsk_strnullORempty((str))

struct FindParamByName: public std::binary_function< OTObjectWrapper<OTCfgParam *>, const char* , bool > {
	  bool operator () ( const OTObjectWrapper<OTCfgParam *> &oParam, const char* pcName ) const {
		  return  tsk_striequals(oParam->getName(), pcName);
		}
	  };

OTEngine::OTEngine()
: OTObject()
, m_bStarted(false)
, m_uId(++g_uId)
{
	if(!OTEngine::g_bInitialized)
	{
		// 'av_register_all()' not called by Doubango as there is no dependncy with libavformat
		av_register_all(); // formats
		avfilter_register_all(); // filters
		
		SipStack::initialize();
		OTProxyPluginMgr::initialize();
		OTEngine::g_bInitialized = true;
	}

	// create Info
	m_oInfo = new OTEngineInfo(m_uId);
	OT_ASSERT(m_oInfo);

	// create mutex
	m_oMutex = new OTMutex();
	OT_ASSERT(m_oMutex);

	// create SIP callback without binding it to the engine
	m_oSipCallback = new OTSipCallback();
	OT_ASSERT(m_oSipCallback);

	// create SIP stack to be used by all bridges
	m_oSipStack = new OTSipStack(m_oSipCallback, OT_SIP_REALM, OT_SIP_IMPI, OT_SIP_IMPU);
	OT_ASSERT(m_oSipStack && m_oSipStack->isValid());

	OT_ASSERT(const_cast<SipStack*>(m_oSipStack->getWrappedStack())->setMode(tsip_stack_mode_mcu));	
	const_cast<SipStack*>(m_oSipStack->getWrappedStack())->addHeader("Server", kOTSipHeaderServer);

	g_oEngines[m_uId] = this;
}

OTEngine::~OTEngine()
{
	stop();

	g_oEngines.erase(m_uId);

	OT_DEBUG_INFO("*** OTEngine destroyed ***");
}

bool OTEngine::start()
{
	// bind the callback to this engine
	int ret = 0;
	std::list<OTObjectWrapper<OTHttpTransport*> >::iterator iter;

	m_oMutex->lock();

	m_oSipCallback->setEngine(this);

	if(isStarted())
	{
		goto bail;
	}

	if(!isValid())
	{
		OT_DEBUG_ERROR("Engine not valid");
		ret = -1;
		goto bail;
	}
	
	// start presentation sharing 3rd process
	if(m_oInfo->m_bPresentationSharingEnabled)
	{
		if(OTDocStreamer::isSupported())
		{
			if(!m_oProcessPresShare3rdApp)
			{
				m_oProcessPresShare3rdApp = OTProcess::New(m_oInfo->m_strPresentationSharingAppPath, OTDocStreamer::buildCommandArgs(m_oInfo->m_nPresentationSharingLocalPort));
				if(!m_oProcessPresShare3rdApp)
				{
					OT_DEBUG_ERROR("Failed to create background process for presentation sharing app");
				}
			}
			if(m_oProcessPresShare3rdApp && !m_oProcessPresShare3rdApp->start())
			{
				ret = -2;
				goto bail;
			}
		}
		else
		{
			OT_DEBUG_INFO("No doc streamer implementation");
		}
	}
	else
	{
		OT_DEBUG_INFO("Presentation sharing not enabled");
	}

	// start all HTTP transports
	iter = m_oHttpTransports.begin();
	for(; iter != m_oHttpTransports.end(); ++iter)
	{
		(*iter)->setSSLCertificates(m_oInfo->m_strSSLPrivateKey.c_str(), m_oInfo->m_strSSLPublicKey.c_str(), m_oInfo->m_strSSLCA.c_str(), m_oInfo->m_bSSLMutualAuth);
		if((*iter)->start() != true)
		{
			ret = -3;
			goto bail;
		}
	}

	// start SIP stack
	if(const_cast<SipStack*>(m_oSipStack->getWrappedStack())->start())
	{
		setStarted(true);
	}
	else
	{
		OT_DEBUG_ERROR("Failed to start SIP stack");
		ret = -2;
		goto bail;
	}

bail:
	
	if(ret != 0)
	{
		m_oSipCallback->setEngine(NULL);
	}

	m_oMutex->unlock();

	return (ret == 0);
}

bool OTEngine::stop()
{
	int ret = 0;
	std::list<OTObjectWrapper<OTHttpTransport*> >::iterator iter;

	m_oMutex->lock();

	if(!isStarted())
	{
		goto bail;
	}

	if(!isValid())
	{
		OT_DEBUG_ERROR("Engine not valid");
		ret = -1;
		goto bail;
	}

	// stop all click2call transports
	iter = m_oHttpTransports.begin();
	for(; iter != m_oHttpTransports.end(); ++iter)
	{
		ret = (*iter)->stop();
	}

	// stop presentation sharing 3rd process
	if(m_oProcessPresShare3rdApp)
	{
		m_oProcessPresShare3rdApp->stop();
	}

	// stop SIP stack
	if(const_cast<SipStack*>(m_oSipStack->getWrappedStack())->stop())
	{
		setStarted(false);
	}
	else
	{
		OT_DEBUG_ERROR("Failed to stop SIP stack");
		ret = -2;
		goto bail;
	}
	
bail:
	// unbind the callback from this engine
	m_oSipCallback->setEngine(NULL);

	m_oMutex->unlock();

	return (ret == 0);
}

OTObjectWrapper<OTBridge*> OTEngine::getBridge(std::string strId)
{
	std::map<std::string, OTObjectWrapper<OTBridge*> >::iterator iter;
	if((iter = m_oBridges.find(strId)) != m_oBridges.end())
	{
		return iter->second;
	}
	return NULL;
}

bool OTEngine::setDebugLevel(const char* pcLevel)
{
	struct debug_level { const char* name; int level; };
	static const debug_level debug_levels[] =
	{
		{"INFO", DEBUG_LEVEL_INFO},
		{"WARN", DEBUG_LEVEL_WARN},
		{"ERROR", DEBUG_LEVEL_ERROR},
		{"FATAL", DEBUG_LEVEL_FATAL},
	};
	static const int debug_levels_count = sizeof(debug_levels)/sizeof(debug_levels[0]);
	int i;
	for(i = 0; i < debug_levels_count; ++i){
		if(tsk_striequals(debug_levels[i].name, pcLevel)){
			tsk_debug_set_level(debug_levels[i].level);
			return true;
		}
	}
	return false;
}

bool OTEngine::addTransport(const char* pcTransport, uint16_t nLocalPort, const char* pcLocalIP /*= tsk_null*/, const char* pcIPVersion /*= tsk_null*/)
{
	if(tsk_striequals(pcTransport, "https") || tsk_striequals(pcTransport, "http"))
	{
		bool isSecure = tsk_striequals(pcTransport, "https");
		OTObjectWrapper<OTHttpTransport*>oHttpTransport = new OTHttpTransport(m_uId, isSecure, pcLocalIP, nLocalPort);
		if(!oHttpTransport)
		{
			return false;
		}
		m_oHttpTransports.push_back(oHttpTransport);
		return true;
	}
	else if(tsk_striequals(pcTransport, "udp") || tsk_striequals(pcTransport, "ws") || tsk_striequals(pcTransport, "wss") || tsk_striequals(pcTransport, "tcp") || tsk_striequals(pcTransport, "tls"))
	{
		bool bRet = const_cast<SipStack*>(m_oSipStack->getWrappedStack())->setLocalIP(pcLocalIP, pcTransport);
		if(bRet)
		{
			bRet &= const_cast<SipStack*>(m_oSipStack->getWrappedStack())->setLocalPort(nLocalPort, pcTransport);
		}
		return bRet;
	}
	
	return false;
}

bool OTEngine::setRtpSymmetricEnabled(bool bEnabled)
{
	return MediaSessionMgr::defaultsSetRtpSymetricEnabled(bEnabled);
}

bool OTEngine::setRtcpMuxEnabled(bool bEnabled)
{
	return MediaSessionMgr::defaultsSetRtcpMuxEnabled(bEnabled);
}

bool OTEngine::setIceEnabled(bool bEnabled)
{
	return MediaSessionMgr::defaultsSetIceEnabled(bEnabled);
}

bool OTEngine::setIceStunEnabled(bool bEnabled)
{
	if(isValid())
	{
		const_cast<SipStack*>(m_oSipStack->getWrappedStack())->setSTUNEnabledForICE(bEnabled);
	}
	return MediaSessionMgr::defaultsSetIceStunEnabled(bEnabled);
}

bool OTEngine::setStunServer(const char* pcServerHost, unsigned short nServerPort, const char* pcUserName /*= NULL*/, const char* pcUserPwd /*= NULL*/)
{
	if(m_oSipStack)
	{
		if(isValid())
		{
			const_cast<SipStack*>(m_oSipStack->getWrappedStack())->setSTUNServer(pcServerHost, nServerPort);
			const_cast<SipStack*>(m_oSipStack->getWrappedStack())->setSTUNCred(pcUserName, pcUserPwd);
		}
	}
	return MediaSessionMgr::defaultsSetStunServer(pcServerHost, nServerPort) && MediaSessionMgr::defaultsSetStunCred(pcUserName, pcUserPwd);
}

bool OTEngine::setRtpBufferSize(size_t nBufferSize)
{
	return MediaSessionMgr::defaultsSetRtpBuffSize(nBufferSize);
}

bool OTEngine::setAvpfTailLength(size_t nMin, size_t nMax)
{
	return MediaSessionMgr::defaultsSetAvpfTail(nMin, nMax);
}

bool OTEngine::setCongestionCtrlEnabled(bool bEnabled)
{
	return MediaSessionMgr::defaultsSetCongestionCtrlEnabled(bEnabled);
}

bool OTEngine::setBandwidthVideoUploadMax(int32_t nBandwidthVideoUploadMax)
{
	return MediaSessionMgr::defaultsSetBandwidthVideoUploadMax(nBandwidthVideoUploadMax);
}

bool OTEngine::setBandwidthVideoDownloadMax(int32_t nBandwidthVideoDownloadMax)
{
	return MediaSessionMgr::defaultsSetBandwidthVideoDownloadMax(nBandwidthVideoDownloadMax);
}

bool OTEngine::setVideoMotionRank(int32_t nVideoMotionRank)
{
	m_oInfo->m_nVideoMotionRank = nVideoMotionRank;
	return MediaSessionMgr::defaultsSetVideoMotionRank(nVideoMotionRank);
}

bool OTEngine::setVideoFps(int32_t nVideoFps)
{
	if(nVideoFps <=0 || nVideoFps >120)
	{
		OT_DEBUG_ERROR("%d not valid Fps. Fps must be in [1- 120]", nVideoFps);
		return false;
	}
	m_oInfo->m_nVideoFps = nVideoFps;
	return MediaSessionMgr::defaultsSetVideoFps(nVideoFps);
}

bool OTEngine::setVideoJbEnabled(bool bEnabled)
{
	return MediaSessionMgr::defaultsSetVideoJbEnabled(bEnabled);
}

bool OTEngine::setVideoZeroArtifactsEnabled(bool bEnabled)
{
	return MediaSessionMgr::defaultsSetVideoZeroArtifactsEnabled(bEnabled);
}

bool OTEngine::setVideoMixedSize(std::string strPrefVideoSize)
{
	if(!isValid())
	{
		TSK_DEBUG_ERROR("Engine not valid");
		return false;
	}

	int i;
	struct pref_video_size { const char* name; tmedia_pref_video_size_t size; size_t w; size_t h;};
	static const pref_video_size pref_video_sizes[] =
	{
		{"sqcif", tmedia_pref_video_size_sqcif, 128, 98}, // 128 x 98
		{"qcif", tmedia_pref_video_size_qcif, 176, 144}, // 176 x 144
		{"qvga", tmedia_pref_video_size_qvga, 320, 240}, // 320 x 240
		{"cif", tmedia_pref_video_size_cif, 352, 288}, // 352 x 288
		{"hvga", tmedia_pref_video_size_hvga, 480, 320}, // 480 x 320
		{"vga", tmedia_pref_video_size_vga, 640, 480}, // 640 x 480
		{"4cif", tmedia_pref_video_size_4cif, 704, 576}, // 704 x 576
		{"svga", tmedia_pref_video_size_svga, 800, 600}, // 800 x 600
		{"480p", tmedia_pref_video_size_480p, 852, 480}, // 852 x 480
		{"720p", tmedia_pref_video_size_720p, 1280, 720}, // 1280 x 720
		{"16cif", tmedia_pref_video_size_16cif, 1408, 1152}, // 1408 x 1152
		{"1080p", tmedia_pref_video_size_1080p, 1920, 1080}, // 1920 x 1080
		{"2160p", tmedia_pref_video_size_2160p, 3840, 2160}, // 3840 x 2160
	};
	static const int pref_video_sizes_count = sizeof(pref_video_sizes)/sizeof(pref_video_sizes[0]);
	
	for(i = 0; i < pref_video_sizes_count; ++i)
	{
		if(tsk_striequals(pref_video_sizes[i].name, strPrefVideoSize.c_str()))
		{
			if(MediaSessionMgr::defaultsSetPrefVideoSize(pref_video_sizes[i].size))
			{
				m_oInfo->m_nMixedVideoWidth = pref_video_sizes[i].w;
				m_oInfo->m_nMixedVideoHeight = pref_video_sizes[i].h;
				return true;
			}
		}
	}
	OT_DEBUG_ERROR("%s not valid as video size. Valid values: ...", strPrefVideoSize.c_str());
	return false;
}

bool OTEngine::setVideoSpeakerPAR(size_t nNum, size_t nDen)
{
	m_oInfo->m_parSpeaker.nNumerator = nNum;
	m_oInfo->m_parSpeaker.nDenominator = nDen;
	return true;
}

bool OTEngine::setVideoListenerPAR(size_t nNum, size_t nDen)
{
	m_oInfo->m_parListener.nNumerator = nNum;
	m_oInfo->m_parListener.nDenominator = nDen;
	return true;
}

bool OTEngine::setAcceptIncomingSipReg(bool bAcceptIncomingSipReg)
{
	m_oInfo->m_bAcceptIncomingSipReg = bAcceptIncomingSipReg;
	return true;
}

bool OTEngine::setAudioChannels(int32_t nChannels)
{
	switch(nChannels)
	{
		case 1: case 2:  m_oInfo->m_nAudioChannels = nChannels; return true;
		default: OT_DEBUG_ERROR("%d not valid as audio channels.", nChannels); return false;
	}
}

bool OTEngine::setAudioBitsPerSample(int32_t nBitsPerSample)
{
	switch(nBitsPerSample)
	{
		// only "16" is supported in this beta version because Doubango will not create a resampler based on the "bit per sample" (only rate and channels)
		/*case 8:*/ case 16: /*case 32:*/  m_oInfo->m_nAudioBitsPerSample = nBitsPerSample; return true;
		default: OT_DEBUG_ERROR("%d not valid as audio bits per sample.", nBitsPerSample); return false;
	}
}

bool OTEngine::setAudioSampleRate(int32_t nSampleRate)
{
	if(nSampleRate > 0)
	{
		m_oInfo->m_nAudioSampleRate = nSampleRate; 
		return true;
	}
	OT_DEBUG_ERROR("%d not valid as audio sample rate.", nSampleRate);
	return false;
}

bool OTEngine::setAudioPtime(int32_t nPtime)
{
	if(nPtime > 0 && nPtime < 256)
	{
		if(MediaSessionMgr::defaultsSetAudioPtime(nPtime))
		{
			m_oInfo->m_nAudioPtime = nPtime;
			return true;
		}
	}
	OT_DEBUG_ERROR("%d not valid as audio ptime.", nPtime);
	return false;
}

bool OTEngine::setAudioVolume(float fVolume)
{
	if(fVolume >= 0.f && fVolume <= 1.f)
	{
		m_oInfo->m_fAudioVolume = fVolume; 
		return true;
	}
	OT_DEBUG_ERROR("%f not valid as audio volume.", fVolume);
	return false;
}

bool OTEngine::setAudioDim(std::string strDim)
{
	if(strDim.empty() || (tsk_striequals(strDim.c_str(), "2d")))
	{
		m_oInfo->m_eAudioDim = OTDimension_2D;
		return true;
	}
	else if((tsk_striequals(strDim.c_str(), "3d")))
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameExperimental, "Make sure you know how to use 3D audio. No support will be provided in this beta version.");
		m_oInfo->m_eAudioDim = OTDimension_3D;
		return true;
	}
	OT_DEBUG_ERROR("%s not valid as audio dimension.", strDim.c_str());
	return false;
}

bool OTEngine::setAudioMaxLatency(int32_t nMaxLatency)
{
	if(nMaxLatency >= 0)
	{
		m_oInfo->m_nAudioMaxLatency = nMaxLatency; 
		return true;
	}
	OT_DEBUG_ERROR("%d not valid as audio max latency.", nMaxLatency);
	return false;
}

bool OTEngine::setAudioLoopback(bool bLoopback)
{
	m_oInfo->m_bAudioLoopback = bLoopback;
	return true;
}

bool OTEngine::setRecordEnabled(bool bEnabled)
{
	m_oInfo->m_bRecordEnabled = bEnabled;
	return true;
}

bool OTEngine::setRecordFileExt(std::string strRecordFileExt)
{
	if(!strRecordFileExt.empty())
	{
		m_oInfo->m_strRecordFileExt = strRecordFileExt;
		return true;
	}
	return false;
}

bool OTEngine::setCodecs(const char* pcCodecs)
{
	if(!isValid())
	{
		TSK_DEBUG_ERROR("Engine not valid");
		return false;
	}

	int i;
	struct codec{ const char* name; tmedia_codec_id_t id; };
	static const codec aCodecNames[] = { 
		{"pcma", tmedia_codec_id_pcma}, 
		{"pcmu", tmedia_codec_id_pcmu},
		{"opus", tmedia_codec_id_opus},
		{"amr-nb-be", tmedia_codec_id_amr_nb_be},
		{"amr-nb-oa", tmedia_codec_id_amr_nb_oa},
		{"speex-nb", tmedia_codec_id_speex_nb}, 
		{"speex-wb", tmedia_codec_id_speex_wb}, 
		{"speex-uwb", tmedia_codec_id_speex_uwb}, 
		{"g729", tmedia_codec_id_g729ab}, 
		{"gsm", tmedia_codec_id_gsm}, 
		{"g722", tmedia_codec_id_g722}, 
		{"ilbc", tmedia_codec_id_ilbc},
		{"h264-bp", tmedia_codec_id_h264_bp}, 
		{"h264-mp", tmedia_codec_id_h264_mp}, 
		{"vp8", tmedia_codec_id_vp8}, 
		{"h263", tmedia_codec_id_h263}, 
		{"h263+", tmedia_codec_id_h263p}, 
		{"theora", tmedia_codec_id_theora}, 
		{"mp4v-es", tmedia_codec_id_mp4ves_es} 
	};
	static const int nCodecsCount = sizeof(aCodecNames) / sizeof(aCodecNames[0]);

	int64_t nCodecs = (int64_t)tmedia_codec_id_none;
	tsk_params_L_t* pParams;
	int nPriority = 0;

	if((pParams = tsk_params_fromstring(pcCodecs, ";", tsk_true)))
	{
		const tsk_list_item_t* item;
		tsk_list_foreach(item, pParams)
		{	
			const char* pcCodecName = ((const tsk_param_t*)item->data)->name;
			for(i = 0; i < nCodecsCount; ++i)
			{
				if(tsk_striequals(aCodecNames[i].name, pcCodecName))
				{
					nCodecs |= (int64_t)aCodecNames[i].id;
					if(!tdav_codec_is_supported((tdav_codec_id_t)aCodecNames[i].id)){
						TSK_DEBUG_INFO("'%s' codec enabled but not supported", aCodecNames[i].name);
					}
					else{
						tdav_codec_set_priority((tdav_codec_id_t)aCodecNames[i].id, nPriority++);
					}
					break;
				}
			}
		}
	}

	TSK_OBJECT_SAFE_FREE(pParams);

	const_cast<SipStack*>(m_oSipStack->getWrappedStack())->setCodecs_2(nCodecs);
	return true;
}

bool OTEngine::setCodecOpusMaxRates(int32_t nPlaybackMaxRate, int32_t nCaptureMaxRate)
{
	return MediaSessionMgr::defaultsSetOpusMaxPlaybackRate(nPlaybackMaxRate) && MediaSessionMgr::defaultsSetOpusMaxCaptureRate(nCaptureMaxRate);
}

bool OTEngine::setOverlayFontsFolderPath(std::string strPath)
{
	m_oInfo->m_strOverlayFontsFolderPath = strPath;
	return true;
}

bool OTEngine::setOverlayCopyrightText(std::string strText)
{
	m_oInfo->m_strOverlayCopyrightText = strText;
	return true;
}

bool OTEngine::setOverlayWatermarkImagePath(std::string strPath)
{
	m_oInfo->m_strOverlayWatermarkImagePath = strPath;
	return true;
}

bool OTEngine::setOverlayCopyrightFontSize(size_t nFontSize)
{
	m_oInfo->m_nOverlayCopyrightFontSize = nFontSize;
	return true;
}

bool OTEngine::setOverlayCopyrightFontFileName(std::string strFileName)
{
	m_oInfo->m_strOverlayCopyrightFontFileName = strFileName;
	return true;
}

bool OTEngine::setOverlaySpeakerNameFontSize(size_t nFontSize)
{
	m_oInfo->m_nOverlaySpeakerNameFontSize = nFontSize;
	return true;
}

bool OTEngine::setOverlaySpeakerNameFontFileName(std::string strFileName)
{
	m_oInfo->m_strOverlaySpeakerNameFontFileName = strFileName;
	return true;
}

bool OTEngine::setOverlayDisplaySpeakerName(bool bDisplay)
{
	m_oInfo->m_bOverlayDisplaySpeakerName = bDisplay;
	return true;
}

bool OTEngine::setOverlayDisplaySpeakerJobTitle(bool bDisplay)
{
	m_oInfo->m_bOverlayDisplaySpeakerJobTitle = bDisplay;
	return true;
}

bool OTEngine::setSSLCertificates(std::string strPrivateKey, std::string strPublicKey, std::string strCA, bool bMutualAuth /*= false*/)
{
	m_oInfo->m_strSSLPrivateKey = strPrivateKey;
	m_oInfo->m_strSSLPublicKey = strPublicKey;
	m_oInfo->m_strSSLCA = strCA;
	m_oInfo->m_bSSLMutualAuth = bMutualAuth;

	if(isValid())
	{
		return const_cast<SipStack*>(m_oSipStack->getWrappedStack())->setSSLCertificates(
			strPrivateKey.empty() ? tsk_null : strPrivateKey.c_str(),
			strPublicKey.empty() ? tsk_null : strPublicKey.c_str(),
			strCA.empty() ? tsk_null : strCA.c_str(),
			bMutualAuth);
	}
	return true;
}

bool OTEngine::setSRTPMode(std::string strMode)
{
	if(strMode.empty())
	{
		TSK_DEBUG_ERROR("Invalid parameter");
		return false;
	}

	if(tsk_striequals(strMode.c_str(), "none"))
	{
		return MediaSessionMgr::defaultsSetSRtpMode(tmedia_srtp_mode_none);
	}
	else if(tsk_striequals(strMode.c_str(), "optional"))
	{
		return MediaSessionMgr::defaultsSetSRtpMode(tmedia_srtp_mode_optional);
	}
	else if(tsk_striequals(strMode.c_str(), "mandatory"))
	{
		return MediaSessionMgr::defaultsSetSRtpMode(tmedia_srtp_mode_mandatory);
	}
	else
	{
		OT_DEBUG_ERROR("%s note valid as SRTP mode", strMode.c_str());
		return false;
	}
}

bool OTEngine::setSRTPType(std::string strTypesCommaSep)
{
	if(strTypesCommaSep.empty())
	{
		OT_DEBUG_ERROR("Invalid parameter");
		return false;
	}

	tmedia_srtp_type_t srtp_type = tmedia_srtp_type_none;

	if(tsk_strcontains(strTypesCommaSep.c_str(), tsk_strlen(strTypesCommaSep.c_str()), "sdes"))
	{
		srtp_type = (tmedia_srtp_type_t)(srtp_type | tmedia_srtp_type_sdes);
	}
	if(tsk_strcontains(strTypesCommaSep.c_str(), tsk_strlen(strTypesCommaSep.c_str()), "dtls"))
	{
		srtp_type = (tmedia_srtp_type_t)(srtp_type | tmedia_srtp_type_dtls);
	}

	return MediaSessionMgr::defaultsSetSRtpType(srtp_type);
}

bool OTEngine::setPresentationSharingEnabled(bool bPresentationSharingEnabled)
{
	m_oInfo->m_bPresentationSharingEnabled = bPresentationSharingEnabled;
	return true;
}

bool OTEngine::setPresentationSharingBaseFolder(std::string strPresentationSharingBaseFolder)

{
	m_oInfo->m_strPresentationSharingBaseFolder = strPresentationSharingBaseFolder;
	return true;
}

bool OTEngine::setPresentationSharingLocalPort(unsigned short nPresentationSharingLocalPort)
{
	m_oInfo->m_nPresentationSharingLocalPort = nPresentationSharingLocalPort;
	return true;
}

bool OTEngine::setPresentationSharingAppPath(std::string strPresentationSharingAppPath)
{
	m_oInfo->m_strPresentationSharingAppPath = strPresentationSharingAppPath;
	return true;
}

bool OTEngine::setConfFile(const char* pcConfFileFullPath)
{
	bool bRet = false;
	tsk_params_L_t* pParamValues = tsk_null;
	std::list<OTObjectWrapper<OTCfgSection*> >::iterator iterSection;
	std::list<OTObjectWrapper<OTCfgParam*> >::const_iterator iterParam;
	const char *pcParamName, *pcParamValue;

	if(!pcConfFileFullPath)
	{
		OT_DEBUG_ERROR("Invalid parameter");
		return false;
	}

	m_oMutex->lock();

	// parse configuration file
	bRet = OTCfgParser::parse(pcConfFileFullPath, this, OTEngine::OTCfgParserOnNewCfg);
	if(!bRet)
	{
		goto bail;
	}

	// apply configuration
	for(iterSection = m_oCfgSections.begin(); iterSection != m_oCfgSections.end(); ++iterSection)
	{
		/////////////////
		// GLOBAL
		/////////////////
		if(tsk_striequals("global", (*iterSection)->getName()))
		{
			const std::list<OTObjectWrapper<OTCfgParam *> >*pParams = (*iterSection)->getParams();
			/* BEGIN[GLOBAL] */
			for(iterParam = pParams->begin(); iterParam != pParams->end(); ++iterParam)
			{
				TSK_OBJECT_SAFE_FREE(pParamValues);

				pcParamName = (*iterParam)->getName();
				pcParamValue = (*iterParam)->getValue();
				OT_DEBUG_INFO_EX(kOTMobuleNameCfg, "%s = %s", pcParamName, pcParamValue);
				// BEGIN[PARAM]
				if(tsk_striequals("debug-level", pcParamName))
				{
					if(!(bRet = setDebugLevel(pcParamValue)))
					{
						OT_DEBUG_ERROR_EX(kOTMobuleNameCfg, "Failed to set debug-level = %s", pcParamValue);
						goto bail;
					}
				}
				else if(tsk_striequals("debug-audio-loopback", pcParamName))
				{
					setAudioLoopback(ot_str_is_yes(pcParamValue));
				}
				else if(tsk_striequals("accept-sip-reg", pcParamName))
				{
					setAcceptIncomingSipReg(ot_str_is_yes(pcParamValue));
				}
				else if(tsk_striequals("transport", pcParamName))
				{
					if((pParamValues = tsk_params_fromstring(pcParamValue, ";", tsk_true)) && ot_list_count(pParamValues) == 4)
					{
						const char* pcProto = ((const tsk_param_t*)pParamValues->head->data)->name;
						const char* pcLocalIP = ((const tsk_param_t*)pParamValues->head->next->data)->name;
						const char* pcLocalPort = ((const tsk_param_t*)pParamValues->head->next->next->data)->name;
						const char* pcLocalIPVersion = ((const tsk_param_t*)pParamValues->head->next->next->next->data)->name;
						OT_DEBUG_INFO_EX(kOTMobuleNameCfg, "transport = %s://%s:%s@%s", pcProto, pcLocalIP, pcLocalPort, pcLocalIPVersion);

						if(!addTransport(pcProto, ot_str_is_star(pcLocalPort) ? TNET_SOCKET_PORT_ANY : atoi(pcLocalPort), ot_str_is_star(pcLocalIP) ? TNET_SOCKET_HOST_ANY : pcLocalIP, ot_str_is_star(pcLocalIPVersion) ? tsk_null : pcLocalIPVersion))
						{
							OT_DEBUG_ERROR_EX(kOTMobuleNameCfg, "Failed to add 'transport': %s://%s:%s@%s", pcLocalPort, pcLocalIP, pcLocalPort, pcLocalIPVersion);
						}
					}
				}
				else if(tsk_striequals("rtp-symmetric-enabled", pcParamName))
				{
					setRtpSymmetricEnabled(ot_str_is_yes(pcParamValue));
				}
				else if(tsk_striequals("ice-enabled", pcParamName))
				{
					setIceEnabled(ot_str_is_yes(pcParamValue));
				}
				else if(tsk_striequals("icestun-enabled", pcParamName))
				{
					setIceStunEnabled(ot_str_is_yes(pcParamValue));
				}
				else if(tsk_striequals("stun-server", pcParamName))
				{
					size_t nCount;
					if((pParamValues = tsk_params_fromstring(pcParamValue, ";", tsk_true)) && (nCount = ot_list_count(pParamValues)) >= 2)
					{
						const char* pcServerIP = ((const tsk_param_t*)pParamValues->head->data)->name;
						const char* pcServerPort = ((const tsk_param_t*)pParamValues->head->next->data)->name;
						const char* pcUsrName = (nCount >= 3) ? ((const tsk_param_t*)pParamValues->head->next->next->data)->name : NULL;
						const char* pcUsrPwd = (nCount >= 4) ? ((const tsk_param_t*)pParamValues->head->next->next->next->data)->name : NULL;
						OT_DEBUG_INFO_EX(kOTMobuleNameCfg, "stun-server = %s;%s;-;-", pcServerIP, pcServerPort);
						if(!setStunServer(pcServerIP, atoi(pcServerPort), ot_str_is_star(pcUsrName) ? NULL : pcUsrName, ot_str_is_star(pcUsrPwd) ? NULL : pcUsrPwd))
						{
							OT_DEBUG_ERROR_EX(kOTMobuleNameCfg, "Failed to set 'stun-server': %s;%s;-;-", pcServerIP, pcServerPort);
						}
					}
				}
				else if(tsk_striequals("rtp-buffersize", pcParamName))
				{
					setRtpBufferSize(atoi(pcParamValue));
				}
				else if(tsk_striequals("avpf-tail-length", pcParamName))
				{
					if((pParamValues = tsk_params_fromstring(pcParamValue, ";", tsk_true)) && ot_list_count(pParamValues) == 2)
					{
						const char* pcMin = ((const tsk_param_t*)pParamValues->head->data)->name;
						const char* pcMax = ((const tsk_param_t*)pParamValues->head->next->data)->name;
						if(!setAvpfTailLength(atoi(pcMin), atoi(pcMax)))
						{
							OT_DEBUG_ERROR_EX(kOTMobuleNameCfg, "Failed to set 'avpf-tail-length' = %s", pcParamValue);
						}
					}
				}
				else if(tsk_striequals("codecs", pcParamName))
				{
					if(!setCodecs(pcParamValue))
					{
						OT_DEBUG_ERROR_EX(kOTMobuleNameCfg, "Failed to set 'codecs': %s", pcParamValue);
					}
				}
				else if(tsk_striequals("codec-opus-maxrates", pcParamName))
				{
					if((pParamValues = tsk_params_fromstring(pcParamValue, ";", tsk_true)) && ot_list_count(pParamValues) == 2)
					{
						const char* pcPlaybackMaxRate = ((const tsk_param_t*)pParamValues->head->data)->name;
						const char* pcCaptureMaxRate = ((const tsk_param_t*)pParamValues->head->next->data)->name;
						setCodecOpusMaxRates(atoi(pcPlaybackMaxRate), atoi(pcCaptureMaxRate));
					}
				}
				else if(tsk_striequals("rtcp-mux-enabled", pcParamName))
				{
					setRtcpMuxEnabled(ot_str_is_yes(pcParamValue));
				}
				else if(tsk_striequals("congestion-ctrl-enabled", pcParamName))
				{
					setCongestionCtrlEnabled(ot_str_is_yes(pcParamValue));
				}
				else if(tsk_striequals("video-max-upload-bandwidth", pcParamName))
				{
					if(!setBandwidthVideoUploadMax(atoi(pcParamValue)))
					{
						OT_DEBUG_ERROR_EX(kOTMobuleNameCfg, "Failed to set 'video-max-upload-bandwidth' = %s", pcParamValue);
					}
				}
				else if(tsk_striequals("video-max-download-bandwidth", pcParamName))
				{
					if(!setBandwidthVideoDownloadMax(atoi(pcParamValue)))
					{
						OT_DEBUG_ERROR_EX(kOTMobuleNameCfg, "Failed to set 'video-max-download-bandwidth' = %s", pcParamValue);
					}
				}
				else if(tsk_striequals("video-motion-rank", pcParamName))
				{
					if(!setVideoMotionRank(atoi(pcParamValue)))
					{
						OT_DEBUG_ERROR_EX(kOTMobuleNameCfg, "Failed to set 'video-motion-rank' = %s", pcParamValue);
					}
				}
				else if(tsk_striequals("video-fps", pcParamName))
				{
					setVideoFps(atoi(pcParamValue));
				}
				else if(tsk_striequals("video-jb-enabled", pcParamName))
				{
					if(!setVideoJbEnabled(ot_str_is_yes(pcParamValue)))
					{
						OT_DEBUG_ERROR_EX(kOTMobuleNameCfg, "Failed to set 'video-jb-enabled' = %s", pcParamValue);
					}
				}
				else if(tsk_striequals("video-zeroartifacts-enabled", pcParamName))
				{
					if(!setVideoZeroArtifactsEnabled(ot_str_is_yes(pcParamValue)))
					{
						OT_DEBUG_ERROR_EX(kOTMobuleNameCfg, "Failed to set 'video-zeroartifacts-enabled' = %s", pcParamValue);
					}
				}
				else if(tsk_striequals("video-mixed-size", pcParamName))
				{
					setVideoMixedSize(pcParamValue);
				}
				else if(tsk_striequals("video-listener-par", pcParamName))
				{
					if((pParamValues = tsk_params_fromstring(pcParamValue, ":", tsk_true)) && ot_list_count(pParamValues) == 2)
					{
						const char* pcNum = ((const tsk_param_t*)pParamValues->head->data)->name;
						const char* pcDen = ((const tsk_param_t*)pParamValues->head->next->data)->name;
						setVideoListenerPAR(atoi(pcNum), atoi(pcDen));
					}
				}
				else if(tsk_striequals("video-speaker-par", pcParamName))
				{
					if((pParamValues = tsk_params_fromstring(pcParamValue, ":", tsk_true)) && ot_list_count(pParamValues) == 2)
					{
						const char* pcNum = ((const tsk_param_t*)pParamValues->head->data)->name;
						const char* pcDen = ((const tsk_param_t*)pParamValues->head->next->data)->name;
						setVideoSpeakerPAR(atoi(pcNum), atoi(pcDen));
					}
				}
				else if(tsk_striequals("audio-channels", pcParamName))
				{
					setAudioChannels(atoi(pcParamValue));
				}
				else if(tsk_striequals("audio-bits-per-sample", pcParamName))
				{
					setAudioBitsPerSample(atoi(pcParamValue));
				}
				else if(tsk_striequals("audio-sample-rate", pcParamName))
				{
					setAudioSampleRate(atoi(pcParamValue));
				}
				else if(tsk_striequals("audio-ptime", pcParamName))
				{
					setAudioPtime(atoi(pcParamValue));
				}
				else if(tsk_striequals("audio-volume", pcParamName))
				{
					setAudioVolume((float)atof(pcParamValue));
				}
				else if(tsk_striequals("audio-dim", pcParamName))
				{
					setAudioDim(pcParamValue);
				}
				else if(tsk_striequals("audio-max-latency", pcParamName))
				{
					setAudioMaxLatency(atoi(pcParamValue));
				}
				else if(tsk_striequals("record", pcParamName))
				{
					if(!setRecordEnabled(ot_str_is_yes(pcParamValue)))
					{
						OT_DEBUG_ERROR_EX(kOTMobuleNameCfg, "Failed to set 'record': %s", pcParamValue);
					}
				}
				else if(tsk_striequals("record-file-ext", pcParamName))
				{
					if(!setRecordFileExt(pcParamValue))
					{
						OT_DEBUG_ERROR_EX(kOTMobuleNameCfg, "Failed to set 'record-file-ext': %s", pcParamValue);
					}
				}
				else if(tsk_striequals("overlay-fonts-folder-path", pcParamName))
				{
					setOverlayFontsFolderPath(tsk_strnullORempty(pcParamValue) ? "" : std::string(pcParamValue));
				}
				else if(tsk_striequals("overlay-copyright-text", pcParamName))
				{
					setOverlayCopyrightText(tsk_strnullORempty(pcParamValue) ? "" : std::string(pcParamValue));
				}
				else if(tsk_striequals("overlay-copyright-fontsize", pcParamName))
				{
					setOverlayCopyrightFontSize(atoi(pcParamValue));
				}
				else if(tsk_striequals("overlay-copyright-fontfile", pcParamName))
				{
					setOverlayCopyrightFontFileName(tsk_strnullORempty(pcParamValue) ? "" : std::string(pcParamValue));
				}
				else if(tsk_striequals("overlay-speaker-name-enabled", pcParamName))
				{
					setOverlayDisplaySpeakerName(ot_str_is_yes(pcParamValue));
				}
				else if(tsk_striequals("overlay-speaker-name-fontsize", pcParamName))
				{
					setOverlaySpeakerNameFontSize(atoi(pcParamValue));
				}
				else if(tsk_striequals("overlay-speaker-name-fontfile", pcParamName))
				{
					setOverlaySpeakerNameFontFileName(tsk_strnullORempty(pcParamValue) ? "" : std::string(pcParamValue));
				}
				else if(tsk_striequals("overlay-speaker-jobtitle-enabled", pcParamName))
				{
					setOverlayDisplaySpeakerJobTitle(ot_str_is_yes(pcParamValue));
				}
				else if(tsk_striequals("overlay-watermark-image-path", pcParamName))
				{
					setOverlayWatermarkImagePath(tsk_strnullORempty(pcParamValue) ? "" : std::string(pcParamValue));
				}
				else if(tsk_striequals("ssl-private-key", pcParamName))
				{
					setSSLCertificates(std::string(pcParamValue ? pcParamValue : ""), m_oInfo->m_strSSLPublicKey, m_oInfo->m_strSSLCA, m_oInfo->m_bSSLMutualAuth);
				}
				else if(tsk_striequals("ssl-public-key", pcParamName))
				{
					setSSLCertificates(m_oInfo->m_strSSLPrivateKey, std::string(pcParamValue ? pcParamValue : ""), m_oInfo->m_strSSLCA, m_oInfo->m_bSSLMutualAuth);
				}
				else if(tsk_striequals("ssl-ca", pcParamName))
				{
					setSSLCertificates(m_oInfo->m_strSSLPrivateKey, m_oInfo->m_strSSLPublicKey, std::string(pcParamValue ? pcParamValue : ""), m_oInfo->m_bSSLMutualAuth);
				}
				else if(tsk_striequals("ssl-mutual-auth", pcParamName))
				{
					setSSLCertificates(m_oInfo->m_strSSLPrivateKey, m_oInfo->m_strSSLPublicKey, m_oInfo->m_strSSLCA, ot_str_is_yes(pcParamValue));
				}
				else if(tsk_striequals("srtp-mode", pcParamName))
				{
					setSRTPMode(std::string(pcParamValue ? pcParamValue : ""));
				}
				else if(tsk_striequals("srtp-type", pcParamName))
				{
					setSRTPType(std::string(pcParamValue ? pcParamValue : ""));
				}
				else if(tsk_striequals("presentation-sharing-enabled", pcParamName))
				{
					setPresentationSharingEnabled(ot_str_is_yes(pcParamValue));
				}
				else if(tsk_striequals("presentation-sharing-process-local-port", pcParamName))
				{
					setPresentationSharingLocalPort(atoi(pcParamValue));
				}
				else if(tsk_striequals("presentation-sharing-base-folder", pcParamName))
				{
					setPresentationSharingBaseFolder(tsk_strnullORempty(pcParamValue) ? OPENTELEPRESENCE_PRESENTATION_SHARING_BASE_FOLDER : pcParamValue);
				}
				else if(tsk_striequals("presentation-sharing-app", pcParamName))
				{
					setPresentationSharingAppPath(tsk_strnullORempty(pcParamValue) ? OPENTELEPRESENCE_PRESENTATION_SHARING_APP_PATH : pcParamValue);
				}			
				else
				{
					OT_DEBUG_WARN_EX(kOTMobuleNameCfg, "[%s = %s] not valid at global scoop", pcParamName, pcParamValue);
				}
				// END[PARAM]
			}
			/* END[GLOBAL] */
		}

		/////////////////
		// BRIDGE
		/////////////////
		else if(tsk_striequals("bridge", (*iterSection)->getName()))
		{
			const std::list<OTObjectWrapper<OTCfgParam *> >*pParams = (*iterSection)->getParams();
			std::list<OTObjectWrapper<OTCfgParam *> >::const_iterator iter_bridge_id;

			/* BEGIN[BRIDGE] */
					
			// find the bridge id (which is required)
			if ((iter_bridge_id = std::find_if( pParams->begin(), pParams->end(), std::bind2nd( FindParamByName(), "id" ) )) == pParams->end())
			{
				OT_DEBUG_ERROR_EX(kOTMobuleNameCfg, "No 'id' entry for a '[bridge]' section...ignoring all entries");
				continue;
			}
			if(tsk_strnullORempty((*iter_bridge_id)->getValue()))
			{
				OT_DEBUG_ERROR_EX(kOTMobuleNameCfg, "NULL bridge identifier ...ignoring all entried");
				continue;
			}
			
			// make it kvp for quick lookup
			OTMapOfCfgParams mapOfCfgParams;
			for(iterParam = pParams->begin(); iterParam != pParams->end(); ++iterParam)
			{
				mapOfCfgParams[std::string((*iterParam)->getName())] = *iterParam;
			}
			// store
			m_oBridgeCfgs[std::string((*iter_bridge_id)->getValue())] = mapOfCfgParams;

			OT_DEBUG_INFO_EX(kOTMobuleNameCfg, "Bridge with id ='%s' added", (*iter_bridge_id)->getValue());

			/* END[BRIDGE] */
		}
	}

bail:
	TSK_OBJECT_SAFE_FREE(pParamValues);
	m_oCfgSections.clear();
	OTObjectSafeRelease(m_oCurrCfgSection);

	m_oMutex->unlock();

	return bRet;
}

bool OTEngine::OTCfgParserOnNewCfg(OTObjectWrapper<OTCfg*> oCfg, const void* pcCallbackData)
{
	OTEngine* This = dynamic_cast<OTEngine*>((OTEngine*)pcCallbackData);

	if(!oCfg || !This)
	{
		OT_DEBUG_ERROR("Invalid parameter");
		return false;
	}

	switch(oCfg->getType())
	{
		case OTCfgType_Section:
			{
				if(This->m_oCurrCfgSection)
				{
					This->m_oCfgSections.push_back(This->m_oCurrCfgSection);
				}
				This->m_oCurrCfgSection = dynamic_cast<OTCfgSection*>(*oCfg);
				return !!(This->m_oCurrCfgSection);
			}

		case OTCfgType_Param:
			{
				if(!This->m_oCurrCfgSection)
				{
					OT_DEBUG_ERROR("Cfg with type equal to param not expect because no section");
					return false;
				}
				return This->m_oCurrCfgSection->addParam(dynamic_cast<OTCfgParam*>(*oCfg));
			}

		case OTCfgType_EoF:
			{
				if(This->m_oCurrCfgSection)
				{
					This->m_oCfgSections.push_back(This->m_oCurrCfgSection);
					OTObjectSafeRelease(This->m_oCurrCfgSection);
				}
				return true;
			}
		default:
			{
				OT_DEBUG_ERROR("%d not valid as cfg type", oCfg->getType());
				return false;
			}
	}
}

OTObjectWrapper<OTEngine*> OTEngine::getEngine(uint64_t uId)
{
	static std::map<uint64_t, OTEngine* >::iterator iter;
	if((iter = g_oEngines.find(uId)) != g_oEngines.end())
	{
		return iter->second;
	}
	return NULL;
}

OTObjectWrapper<OTBridge*> OTEngine::getBridge(uint64_t uEngineId, std::string strBridgeId)
{
	OTObjectWrapper<OTEngine*>oEngine = OTEngine::getEngine(uEngineId);
	if(oEngine)
	{
		return oEngine->getBridge(strBridgeId);
	}
	return NULL;
}

OTObjectWrapper<OTEngine*> OTEngine::New()
{
	OTObjectWrapper<OTEngine*> pEngine;

#if OPENTELEPRESENCE_UNDER_WINDOWS
	pEngine = new OTEngineWin32();
#elif OPENTELEPRESENCE_UNDER_OSX
	pEngine = new OTEngineOSX();
#else
	pEngine = new OTEngineGen();
#endif

	OT_ASSERT(pEngine);

	if(!pEngine->isValid())
	{
		OTObjectSafeRelease(pEngine);
	}
	return pEngine;
}

