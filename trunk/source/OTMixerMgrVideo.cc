/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#include "opentelepresence/OTMixerMgrVideo.h"

#include "opentelepresence/OTProxyPluginMgr.h"
#include "opentelepresence/OTFrameVideo.h"
#include "opentelepresence/OTBridge.h"

#include "tsk_time.h"

#include <assert.h>
#include <functional>
#include <algorithm>

// 
#if !defined(OT_MIN_TIME_BEFORE_HONORING_FIR)
// Guard to avoid sending multiple IDR in short time. All FIR between [0-800ms] will be absorbed
#	define OT_MIN_TIME_BEFORE_HONORING_FIR 1000 /* milliseconds FIXME: move to cfg */
#endif

struct PredOTConsumerNotStarted: public std::unary_function< std::pair<uint64_t, OTObjectWrapper<OTProxyPluginConsumerVideo*> >, bool > {
  bool operator () ( const std::pair<uint64_t, OTObjectWrapper<OTProxyPluginConsumerVideo*> > &pcOTConsumer ) const {
	  return !pcOTConsumer.second->isStarted();
    }
};

struct PredOTProducerNotStarted: public std::unary_function< std::pair<uint64_t, OTObjectWrapper<OTProxyPluginProducerVideo*> >, bool > {
  bool operator () ( const std::pair<uint64_t, OTObjectWrapper<OTProxyPluginProducerVideo*> > &pcOTProducer ) const {
	  return !pcOTProducer.second->isStarted();
    }
};

OTMixerMgrVideo::OTMixerMgrVideo(OTObjectWrapper<OTBridgeInfo*> oBridgeInfo)
:OTMixerMgr(OTMediaType_Video, oBridgeInfo)
{
	m_phPullThread[0] = NULL;
	m_phPullCond = NULL;

	m_bStarted = false;
	m_bPaused = false;
	m_bRequestedIntraViaRtcp = false;

	m_phProducersMutex = tsk_mutex_create_2(tsk_false);
	m_phConsumersMutex = tsk_mutex_create_2(tsk_false);
	
	m_nLastTimeRTPTimeoutChecked = 0;

	m_pMixedBufferPtr = NULL;
	m_nMixedBufferSize = 0;

	m_nLastTimeIdrSent = 0;
	
	m_oMixerVideo = OTMixerVideo::New(oBridgeInfo);

	m_bValid = (m_phConsumersMutex && m_oMixerVideo);
}

OTMixerMgrVideo::~OTMixerMgrVideo()
{
	stop();

	tsk_mutex_destroy(&m_phProducersMutex);
	tsk_mutex_destroy(&m_phConsumersMutex);

	TSK_FREE(m_pMixedBufferPtr);

	OTObjectSafeRelease(m_oMixerVideo);

	OT_DEBUG_INFO("*** OTMixerMgrVideo destroyed ***");
}

int OTMixerMgrVideo::start()
{
	OT_DEBUG_INFO("Video Mixer Start - Consumers.Count=%u, Producers.Count=%u", m_oConsumers.size(), m_oProducers.size());

	if(isStarted() && !isPaused())
	{
		return 0;
	}

	if(!isStarted())
	{
		if(isPaused()) /* //was paused? */
		{
			resetConsumersJitter();
		}
	}

	m_bPaused = false;

	if(!m_phPullCond && !(m_phPullCond = tsk_condwait_create()))
	{
		OT_DEBUG_ERROR("Failed to create condwait");
		return -1;
	}

	if(!m_phPullThread[0])
	{
		// set started value to avoid early async thread exit
		m_bStarted = true;
		int ret = tsk_thread_create(&m_phPullThread[0], OTMixerMgrVideo::pullThreadFunc, this);
		if(ret != 0 || !m_phPullThread[0])
		{
			OT_DEBUG_ERROR("Failed to create with error code = %d", ret);
			m_bStarted = false;
			return ret;
		}
		if((ret = tsk_thread_set_priority(m_phPullThread[0], TSK_THREAD_PRIORITY_TIME_CRITICAL)))
		{
			/*return ret;*/
		}
	}

	return 0;
}

bool OTMixerMgrVideo::isStarted()
{
	return m_bStarted;
}

int OTMixerMgrVideo::pause()
{
	if(!isStarted() || isPaused()){
		return 0;
	}

	m_bPaused = true;
	return 0;
}

bool OTMixerMgrVideo::isPaused()
{
	return m_bPaused;
}

int OTMixerMgrVideo::flush()
{
	if(isStarted() && !isPaused()){
		OT_DEBUG_ERROR("You must Stop() or Pause() the mixer before flushing");
		return -1;
	}

	std::map<uint64_t, OTObjectWrapper<OTProxyPluginConsumerVideo*> >::iterator iter_cons;
	std::map<uint64_t, OTObjectWrapper<OTProxyPluginProducerVideo*> >::iterator iter_prod;
	
	// consumers
	tsk_mutex_lock(m_phConsumersMutex);
	iter_cons = m_oConsumers.begin();
	while((iter_cons = find_if(m_oConsumers.begin(), m_oConsumers.end(), PredOTConsumerNotStarted())) != m_oConsumers.end())
	{
		m_oConsumers.erase(iter_cons);
	}
	tsk_mutex_unlock(m_phConsumersMutex);

	// producers
	tsk_mutex_lock(m_phProducersMutex);
	iter_prod = m_oProducers.begin();
	while((iter_prod = find_if(m_oProducers.begin(), m_oProducers.end(), PredOTProducerNotStarted())) != m_oProducers.end())
	{
		m_oProducers.erase(iter_prod);
	}
	tsk_mutex_unlock(m_phProducersMutex);

	return 0;
}

int OTMixerMgrVideo::stop()
{
	OT_DEBUG_INFO("MixerMgr video stop");

	if(!isStarted())
	{
		return 0;
	}

	m_bStarted = false;
	m_bPaused = false;

	if(m_phPullCond)
	{
		tsk_condwait_broadcast(m_phPullCond);
	}

	if(m_phPullThread[0])
	{
		tsk_thread_join(&m_phPullThread[0]);
	}
	if(m_phPullCond)
	{
		tsk_condwait_destroy(&m_phPullCond);
	}
	return 0;
}

int OTMixerMgrVideo::attachMediaPlugins(OTObjectWrapper<OTSessionInfo*> pOTSessionInfo)
{
	int ret = 0;
	OTObjectWrapper<OTProxyPluginProducerVideo*> pOTPluginProducerVideo;
	OTObjectWrapper<OTProxyPluginConsumerVideo*> pOTPluginConsumerVideo;
	OTObjectWrapper<OTProxyPlugin*> pOTPlugin;
	const tmedia_session_mgr_t* pcWrappedMgr;

	OTObjectWrapper<OTSessionInfoAV*>pOTSessionInfoAV = dynamic_cast<OTSessionInfoAV*>(*pOTSessionInfo);
	OT_ASSERT(pOTSessionInfoAV);

	//
	//	Consumer
	//
	pOTPlugin = OTProxyPluginMgr::findPlugin(pOTSessionInfoAV->getVideoConsumerId());
	if(!pOTPlugin)
	{
		OT_DEBUG_ERROR("Cannot find audio consumer with id=%llu", pOTSessionInfoAV->getVideoConsumerId());
		return -2;
	}

	// lock() consumers
	tsk_mutex_lock(m_phConsumersMutex);
	pOTPluginConsumerVideo = dynamic_cast<OTProxyPluginConsumerVideo*>(*pOTPlugin);
	OT_ASSERT(pOTPluginConsumerVideo);
	
	pOTPluginConsumerVideo->setSessionInfo(pOTSessionInfo);
	m_oConsumers[pOTSessionInfoAV->getVideoConsumerId()] = pOTPluginConsumerVideo;
	// unlock() consumers
	tsk_mutex_unlock(m_phConsumersMutex);

	if(ret != 0)
	{
		goto bail;
	}

	//
	//	Producer
	//
	pOTPlugin = OTProxyPluginMgr::findPlugin(pOTSessionInfoAV->getVideoProducerId());
	if(!pOTPlugin)
	{
		OT_DEBUG_ERROR("Cannot find audio producer with id=%llu", pOTSessionInfoAV->getVideoProducerId());
		return -3;
	}

	// lock() producers
	tsk_mutex_lock(m_phProducersMutex);
	pOTPluginProducerVideo = dynamic_cast<OTProxyPluginProducerVideo*>(*pOTPlugin);
	OT_ASSERT(pOTPluginProducerVideo);

	pOTPluginProducerVideo->setSessionInfo(pOTSessionInfo);
	m_oProducers[pOTSessionInfoAV->getVideoProducerId()] = (dynamic_cast<OTProxyPluginProducerVideo*>(*pOTPlugin));

	// reset() codecs before unlocking producers
	// FIXME: only if this producer is an active contributor
	executeActionOnCodecs(OTCodecAction_ForceIntra);

	// unlock() producers
	tsk_mutex_unlock(m_phProducersMutex);

	// listen for RTCP events
	pcWrappedMgr = pOTSessionInfoAV->getWrappedMediaSessionMgr();
	if(pcWrappedMgr)
	{
		ret = tmedia_session_mgr_set_onrtcp_cbfn(const_cast<tmedia_session_mgr_t*>(pcWrappedMgr), tmedia_video, this, OTMixerMgrVideo::rtcpOnEventCb);
	}
	else
	{
		OT_DEBUG_WARN("No wrapped media session managed: Video quality will be bad :)");
	}
	
bail:
	
	return ret;
}

int OTMixerMgrVideo::deAttachMediaPlugins(OTObjectWrapper<OTSessionInfo*> pOTSessionInfo)
{
	OTObjectWrapper<OTSessionInfoAV*>pOTSessionInfoAV = dynamic_cast<OTSessionInfoAV*>(*pOTSessionInfo);
	OT_ASSERT(pOTSessionInfoAV);

	//
	//	Consumer
	//
	tsk_mutex_lock(m_phConsumersMutex);
	m_oConsumers.erase(pOTSessionInfoAV->getVideoConsumerId());
	tsk_mutex_unlock(m_phConsumersMutex);

	//
	//	Producer
	//

	tsk_mutex_lock(m_phProducersMutex);
	m_oProducers.erase(pOTSessionInfoAV->getVideoProducerId());
	tsk_mutex_unlock(m_phProducersMutex);

	return 0;
}

bool OTMixerMgrVideo::isValid()
{
	return m_bValid;
}

void* TSK_STDCALL OTMixerMgrVideo::pullThreadFunc(void *arg)
{
	OTMixerMgrVideo *This = (OTMixerMgrVideo *)arg;
	int64_t nDelay = 0;

	int64_t nPtime = (1000 /  This->m_oBridgeInfo->getVideoFps());
	uint64_t nExpectedNextTime = tsk_time_now();

	OT_DEBUG_INFO(" video pullThreadFunc ENTER (ptime = %lld)", nPtime);

	while(This->isStarted())
	{
		tsk_condwait_timedwait(This->m_phPullCond, (nPtime - nDelay));

		if(!This->isStarted())
		{
			break;
		}
		
		if(!This->isPaused())
		{
			This->mixAndSend();
		}

		nExpectedNextTime += nPtime;
		nDelay = (tsk_time_now() - nExpectedNextTime);
		nDelay = TSK_CLAMP(0, nDelay, nPtime);
	}
	
	OT_DEBUG_INFO(" video pullThreadFunc EXIT ");

	return NULL;
}

int OTMixerMgrVideo::rtcpOnEventCb(const void* pcUsrData, enum tmedia_rtcp_event_type_e eEventType, uint32_t nSsrcMedia)
{
	OTMixerMgrVideo *This = (OTMixerMgrVideo *)pcUsrData;

	switch(eEventType)
	{
		case tmedia_rtcp_event_type_fir:
			{
				OT_DEBUG_INFO("Remote party requested IDR via RTCP");
				This->m_bRequestedIntraViaRtcp = true;
				break;
			}
	default: break;
	}

	return 0;
}

int OTMixerMgrVideo::mixAndSend()
{
	std::map<uint64_t, OTObjectWrapper<OTProxyPluginConsumerVideo*> >::const_iterator iter_cons;
	std::map<uint64_t, OTObjectWrapper<OTProxyPluginProducerVideo*> >::const_iterator iter_prod;
	
	bool bHasAtLeastOneValidSession = false;

	std::map<uint64_t, OTObjectWrapper<OTProxyPluginConsumerVideo*> > _oConsumers = m_oConsumers; // ref() to avoid locking
	std::map<uint64_t, OTObjectWrapper<OTProxyPluginProducerVideo*> > _oProducers = m_oProducers; // ref() to avoid locking

	//int nMaxWidth = 0;
	//int nMaxHeight = 0;
	uint32_t nConsumersCount = _oConsumers.size();

	// FIXME: loop useless -> use bHasAtLeastOneValidSession = predicate(...)
	for(iter_cons = _oConsumers.begin(); iter_cons != _oConsumers.end(); )
	{
		if(!(*iter_cons).second->isValid())
		{
			goto next_iter_cons_1;
		}

		bHasAtLeastOneValidSession = true;

		if(!(*iter_cons).second->isStarted())
		{
			goto next_iter_cons_1;
		}

next_iter_cons_1:
			++iter_cons;
	}

	OTObjectWrapper<OTFrameVideo *> oFrameVideo;
	OTObjectWrapper<OTSessionInfoAV *>oSessionInfoAudioVideo;
	OTObjectWrapper<OTCodecVideo *>oCodec;

	// do nothing if there is a active consumer
	if(!bHasAtLeastOneValidSession)
	{
		goto done;
	}

	//
	//	Producers
	//	

	
	// Reset() the buffer and Mix()
	oFrameVideo = m_oMixerVideo->mix(&_oConsumers, &m_pMixedBufferPtr, &m_nMixedBufferSize);

	// check whether remote one of remote peers request IntraFrame via RTCP callback
	// FIXME: this is not very optimal as the reset is done on all codecs
	if(m_bRequestedIntraViaRtcp)
	{
		if(m_nLastTimeIdrSent && (m_nLastTimeIdrSent + OT_MIN_TIME_BEFORE_HONORING_FIR) > tsk_time_now())
		{
			OT_DEBUG_INFO("Do not honor this FIR because previous one was too close.");
		}
		else
		{
			if(executeActionOnCodecs(OTCodecAction_ForceIntra))
			{
				m_bRequestedIntraViaRtcp = false;
				m_nLastTimeIdrSent = tsk_time_now();
			}
		}
	}

	if(oFrameVideo && oFrameVideo->getValidDataSize())
	{
		// Record video frame
		if(m_oRecorder)
		{
			m_oRecorder->writeRawVideoPayload(oFrameVideo->getBufferPtr(), oFrameVideo->getBufferSize());
		}

		// Send video frame
		for(iter_prod = _oProducers.begin(); iter_prod != _oProducers.end(); )
		{
			if(!(*iter_prod).second->isValid() || !(*iter_prod).second->isStarted())
			{
				goto next_iter_prod;
			}
			
			// get session info associated to this producer
			oSessionInfoAudioVideo = dynamic_cast<OTSessionInfoAV*>(*(*iter_prod).second->getSessionInfo());
			// find best codec to use for encoding
			if(!(oCodec = dynamic_cast<OTCodecVideo*>(*findBestCodec(oSessionInfoAudioVideo->getNegotiatedCodecs()))))
			{
				OT_DEBUG_INFO("No codec associated to video producer with id = %llu yet", (*iter_prod).second->getId());
				oCodec = OTCodecVideo::createBestCodec(oSessionInfoAudioVideo->getNegotiatedCodecs(), oFrameVideo->getWidth(), oFrameVideo->getHeight());
				if(oCodec)
				{
					if(!oCodec->open())
					{
						OT_DEBUG_ERROR("Failed to open video code");
						OTObjectSafeRelease(oCodec);
					}
					else
					{
						// add the codec to the list of supported codecs
						OT_ASSERT(addCodec(*oCodec));
					}
				}
				else
				{
					OT_DEBUG_ERROR("Failed to find best codec with negCodecs = %d", oSessionInfoAudioVideo->getNegotiatedCodecs());
				}
			}

			if(oCodec)
			{
				// if hasResult() means mixed data encoded for another producer
				if(!oCodec->hasResults())
				{
					// encode mixed data if not already done
					if(!oCodec->encode(m_pMixedBufferPtr, m_nMixedBufferSize))
					{
						OT_DEBUG_ERROR("VideoEncode(size = %u) failed", m_nMixedBufferSize);
					}
				}
				size_t nChunkIndex = 0;
				OTObjectWrapper<OTCodecEncodingResult*> oEncodingResult;
				// for each video chunk, send()
				while((oEncodingResult = oCodec->getEncodingResultAt(nChunkIndex++)))
				{
					(*iter_prod).second->sendRaw(oEncodingResult->getDataPtr(), oEncodingResult->getDataEncodedSize(), oEncodingResult->getDuration(), oEncodingResult->isLastResultInFrame());
				}
			}
			
			bHasAtLeastOneValidSession = true;

	next_iter_prod:
			++iter_prod;
		}//end-of-producers (for)
	} // if(videoFrame)

	// flush() latest encoded data
	executeActionOnCodecs(OTCodecAction_FlushEncodedData);

done:

	// pause the timer if there is no consumer/producer
	// will be reStarted when consumers are added
	if(!bHasAtLeastOneValidSession)
	{
		OT_DEBUG_INFO("Pausing the 'video' mixer because no active session left...");
		pause();
	}

	return 0;
}

void OTMixerMgrVideo::resetConsumersJitter()
{
	std::map<uint64_t, OTObjectWrapper<OTProxyPluginConsumerVideo*> >::const_iterator iter_cons;
	tsk_mutex_lock(m_phConsumersMutex);

	iter_cons = m_oConsumers.begin();
	while(iter_cons != m_oConsumers.end())
	{
		if(!(*iter_cons).second->isValid() || !(*iter_cons).second->isStarted())
		{
			goto next_iter_cons;
		}
		(*iter_cons).second->reset();
next_iter_cons:
		++iter_cons;
	}

	tsk_mutex_unlock(m_phConsumersMutex);
}