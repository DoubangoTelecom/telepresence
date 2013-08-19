/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#include "opentelepresence/OTProxyPluginMgr.h"
#include "opentelepresence/OTProxyPluginConsumerAudio.h"
#include "opentelepresence/OTProxyPluginConsumerVideo.h"
#include "opentelepresence/OTProxyPluginProducerAudio.h"
#include "opentelepresence/OTProxyPluginProducerVideo.h"
#include "opentelepresence/OTProxyPluginProducer.h"
#include "opentelepresence/OTResampler.h"

#include "MediaSessionMgr.h"

#include <assert.h>
#include <limits.h> /* INT_MAX */

using namespace std;

ProxyPluginMgr* OTProxyPluginMgr::g_pPluginMgr = NULL;
OTProxyPluginMgrCallback* OTProxyPluginMgr::g_pPluginMgrCallback = NULL;
OTMapOfPlugins* OTProxyPluginMgr::g_pPlugins = NULL;
tsk_mutex_handle_t* OTProxyPluginMgr::g_phMutex = NULL;
bool OTProxyPluginMgr::g_bInitialized = false;

//
//	OTProxyPluginMgr
//
void OTProxyPluginMgr::initialize()
{
	if(!OTProxyPluginMgr::g_bInitialized)
	{
		// register audio/video conumers and producers
		ProxyAudioConsumer::registerPlugin();
		ProxyAudioProducer::registerPlugin();
		ProxyVideoProducer::registerPlugin();
		ProxyVideoConsumer::registerPlugin();

		// register FFmpeg audio resampler (will replace SpeexDSP from Doubango)
		OTResamplerAudio::registerPlugin();

		// defaul chroma for consumers and producers
		ProxyVideoConsumer::setDefaultChroma(tmedia_chroma_yuv420p);
		ProxyVideoProducer::setDefaultChroma(tmedia_chroma_yuv420p);
		// do not try to resize incoming video packets (will be done at mixing stage)
		ProxyVideoConsumer::setDefaultAutoResizeDisplay(true);

		// up to the remote peers
		MediaSessionMgr::defaultsSetEchoSuppEnabled(false);
		MediaSessionMgr::defaultsSetAgcEnabled(false);
		MediaSessionMgr::defaultsSetNoiseSuppEnabled(false);
		MediaSessionMgr::defaultsSetAudioPtime(OPENTELEPRESENCE_AUDIO_PTIME_DEFAULT);
		MediaSessionMgr::defaultsSetAudioChannels(OPENTELEPRESENCE_AUDIO_CHANNELS_DEFAULT, OPENTELEPRESENCE_AUDIO_CHANNELS_DEFAULT);

		// set audio jb margin and max late percent
		MediaSessionMgr::defaultsSetJbMaxLateRate(1);
		MediaSessionMgr::defaultsSetJbMargin(100);

		// disable 100rel (PRACK)
		MediaSessionMgr::defaultsSet100relEnabled(false);

		// SRTP options
		MediaSessionMgr::defaultsSetSRtpMode(tmedia_srtp_mode_optional);
		MediaSessionMgr::defaultsSetSRtpType(tmedia_srtp_type_sdes_dtls);

		// set preferred video size
		MediaSessionMgr::defaultsSetPrefVideoSize(tmedia_pref_video_size_vga);
		
		// Video bandwidth and congestion control
		MediaSessionMgr::defaultsSetBandwidthVideoUploadMax(INT_MAX);
		MediaSessionMgr::defaultsSetBandwidthVideoDownloadMax(INT_MAX);
		MediaSessionMgr::defaultsSetVideoMotionRank(OPENTELEPRESENCE_VIDEO_MOTION_RANK_DEFAULT/*1(low),2(medium) or 4(high)*/);
		MediaSessionMgr::defaultsSetCongestionCtrlEnabled(true);
		MediaSessionMgr::defaultsSetVideoFps(OPENTELEPRESENCE_VIDEO_FPS_DEFAULT);

		// do not use internal encoders
		MediaSessionMgr::defaultsSetByPassEncoding(true);
		MediaSessionMgr::defaultsSetByPassDecoding(false);

		// disable/enable video jitter buffer
		MediaSessionMgr::defaultsSetVideoJbEnabled(true);

		// Do not allow video artifacts (requires remote peers to support AVPF (RTCP-PLI/NACK/FIR))
		MediaSessionMgr::defaultsSetVideoZeroArtifactsEnabled(true);

		// enlarge RTP network buffer size
		MediaSessionMgr::defaultsSetRtpBuffSize(0xFFFF);

		// NATT traversal options
		MediaSessionMgr::defaultsSetRtpSymetricEnabled(true);
		MediaSessionMgr::defaultsSetIceEnabled(true);
		MediaSessionMgr::defaultsSetIceStunEnabled(true);

		// enlarge AVPF tail to honor more RTCP-NACK requests
		MediaSessionMgr::defaultsSetAvpfTail(200, 500);

		OTProxyPluginMgr::g_pPluginMgrCallback = new OTProxyPluginMgrCallback();
		OTProxyPluginMgr::g_pPluginMgr = ProxyPluginMgr::createInstance(OTProxyPluginMgr::g_pPluginMgrCallback);
		OT_ASSERT(OTProxyPluginMgr::g_pPluginMgr);
		OTProxyPluginMgr::g_pPlugins = new OTMapOfPlugins();
		OTProxyPluginMgr::g_phMutex = tsk_mutex_create();
		OTProxyPluginMgr::g_bInitialized = true;
	}
}

void OTProxyPluginMgr::deInitialize()
{
	if(OTProxyPluginMgr::g_bInitialized)
	{
		if(OTProxyPluginMgr::g_phMutex){
			tsk_mutex_destroy(&OTProxyPluginMgr::g_phMutex);
		}
		if(OTProxyPluginMgr::g_pPlugins){
			delete OTProxyPluginMgr::g_pPlugins, OTProxyPluginMgr::g_pPlugins = NULL;
		}
		if(OTProxyPluginMgr::g_pPluginMgrCallback){
			delete OTProxyPluginMgr::g_pPluginMgrCallback, OTProxyPluginMgr::g_pPluginMgrCallback = NULL;
		}
		ProxyPluginMgr::destroyInstance(&OTProxyPluginMgr::g_pPluginMgr);

		OTProxyPluginMgr::g_bInitialized = false;
	}
}

//
//	OTProxyPluginMgrCallback
//
OTProxyPluginMgrCallback::OTProxyPluginMgrCallback()
:ProxyPluginMgrCallback()
{
}

OTProxyPluginMgrCallback::~OTProxyPluginMgrCallback()
{
	OT_DEBUG_INFO("*** OTProxyPluginMgrCallback destroyed ***");
}

// @Override
int OTProxyPluginMgrCallback::OnPluginCreated(uint64_t id, enum twrap_proxy_plugin_type_e type)
{
	tsk_mutex_lock(OTProxyPluginMgr::getMutex());

	switch(type){
		case twrap_proxy_plugin_audio_producer:
			{
				const ProxyAudioProducer* pcProducer = OTProxyPluginMgr::getPluginMgr()->findAudioProducer(id);
				if(pcProducer)
				{
					OTObjectWrapper<OTProxyPlugin*> pOTProducer = new OTProxyPluginProducerAudio(id, pcProducer);
					OTProxyPluginMgr::getPlugins()->insert( pair<uint64_t, OTObjectWrapper<OTProxyPlugin*> >(id, pOTProducer) );
				}
				break;
			}
		case twrap_proxy_plugin_video_producer:
			{
				const ProxyVideoProducer* pcProducer = OTProxyPluginMgr::getPluginMgr()->findVideoProducer(id);
				if(pcProducer)
				{
					OTObjectWrapper<OTProxyPlugin*> pOTProducer = new OTProxyPluginProducerVideo(id, pcProducer);
					OTProxyPluginMgr::getPlugins()->insert( pair<uint64_t, OTObjectWrapper<OTProxyPlugin*> >(id, pOTProducer) );
				}
				break;
			}
		case twrap_proxy_plugin_audio_consumer:
			{
				const ProxyAudioConsumer* pcConsumer = OTProxyPluginMgr::getPluginMgr()->findAudioConsumer(id);
				if(pcConsumer)
				{
					OTObjectWrapper<OTProxyPlugin*> pOTConsumer = new OTProxyPluginConsumerAudio(id, pcConsumer);
					OTProxyPluginMgr::getPlugins()->insert( pair<uint64_t, OTObjectWrapper<OTProxyPlugin*> >(id, pOTConsumer) );
				}
				break;
			}
		case twrap_proxy_plugin_video_consumer:
			{
				const ProxyVideoConsumer* pcConsumer = OTProxyPluginMgr::getPluginMgr()->findVideoConsumer(id);
				if(pcConsumer)
				{
					OTObjectWrapper<OTProxyPlugin*> pOTConsumer = new OTProxyPluginConsumerVideo(id, pcConsumer);
					OTProxyPluginMgr::getPlugins()->insert( pair<uint64_t, OTObjectWrapper<OTProxyPlugin*> >(id, pOTConsumer) );
				}
				break;
			}
		default:
			{
				OT_ASSERT(false);
				break;
			}
	}

	tsk_mutex_unlock(OTProxyPluginMgr::getMutex());

	return 0;
}

// @Override
int OTProxyPluginMgrCallback::OnPluginDestroyed(uint64_t id, enum twrap_proxy_plugin_type_e type)
{
	tsk_mutex_lock(OTProxyPluginMgr::getMutex());

	switch(type){
		case twrap_proxy_plugin_audio_producer:
		case twrap_proxy_plugin_video_producer:
		case twrap_proxy_plugin_audio_consumer:
		case twrap_proxy_plugin_video_consumer:
		{
			OTProxyPluginMgr::erasePlugin(id);
			break;
		}
		default:
		{
			OT_ASSERT(false);
			break;
		}
	}

	tsk_mutex_unlock(OTProxyPluginMgr::getMutex());

	return 0;
}
