/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#include "opentelepresence/OTMixerMgrAudio.h"

#include "opentelepresence/OTProxyPluginMgr.h"
#include "opentelepresence/OTFrameAudio.h"
#include "opentelepresence/OTBridge.h"

#include <assert.h>
#include <functional>
#include <algorithm>

struct PredOTConsumerNotStarted: public std::unary_function< std::pair<uint64_t, OTObjectWrapper<OTProxyPluginConsumerAudio*> >, bool > {
  bool operator () ( const std::pair<uint64_t, OTObjectWrapper<OTProxyPluginConsumerAudio*> > &pcOTConsumer ) const {
	  return !pcOTConsumer.second->isStarted();
    }
};


struct PredOTProducerNotStarted: public std::unary_function< std::pair<uint64_t, OTObjectWrapper<OTProxyPluginProducerAudio*> >, bool > {
  bool operator () ( const std::pair<uint64_t, OTObjectWrapper<OTProxyPluginProducerAudio*> > &pcOTProducer ) const {
    return !pcOTProducer.second->isStarted();
    }
};


OTMixerMgrAudio::OTMixerMgrAudio(OTObjectWrapper<OTBridgeInfo*> oBridgeInfo)
:OTMixerMgr(OTMediaType_Audio, oBridgeInfo)
{
	m_phPullThread[0] = NULL;
	m_phPullCond = NULL;
	m_nLastTimerPull = 0;

	m_bStarted = false;
	m_bPaused = false;

	OT_ASSERT(m_phProducersMutex = tsk_mutex_create());
	OT_ASSERT(m_phConsumersMutex = tsk_mutex_create());
	
	m_oMixerAudio = OTMixerAudio::New(oBridgeInfo);

	m_bValid = (m_phConsumersMutex && m_oMixerAudio);
}

OTMixerMgrAudio::~OTMixerMgrAudio()
{
	stop();

	tsk_mutex_destroy(&m_phProducersMutex);
	tsk_mutex_destroy(&m_phConsumersMutex);

	OTObjectSafeRelease(m_oMixerAudio);

	OT_DEBUG_INFO("*** OTMixerMgrAudio destroyed ***");
}

int OTMixerMgrAudio::start()
{
	OT_DEBUG_INFO("Audio Mixer Start - Consumers.Count=%u, Producers.Count=%u", m_oConsumers.size(), m_oProducers.size());

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
		m_bStarted = true;
		int ret = tsk_thread_create(&m_phPullThread[0], OTMixerMgrAudio::pullThreadFunc, this);
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

bool OTMixerMgrAudio::isStarted()
{
	return m_bStarted;
}

int OTMixerMgrAudio::pause()
{
	OT_DEBUG_INFO("Audio Mixer Pause");

	if(!isStarted() || isPaused()){
		return 0;
	}

	m_bPaused = true;
	return 0;
}

bool OTMixerMgrAudio::isPaused()
{
	return m_bPaused;
}

int OTMixerMgrAudio::flush()
{
	if(isStarted() && !isPaused())
	{
		OT_DEBUG_ERROR("You must Stop() or Pause() the mixer before flushing");
		return -1;
	}

	std::map<uint64_t, OTObjectWrapper<OTProxyPluginConsumerAudio*> >::iterator iter_cons;
	std::map<uint64_t, OTObjectWrapper<OTProxyPluginProducerAudio*> >::iterator iter_prod;
	
	// consumers
	tsk_mutex_lock(m_phConsumersMutex);
	iter_cons = m_oConsumers.begin();
	while((iter_cons = find_if(m_oConsumers.begin(), m_oConsumers.end(), PredOTConsumerNotStarted())) != m_oConsumers.end())
	{
		m_oConsumers.erase(iter_cons);
		//OTObjectSafeRelease(*iter_cons);
	}
	tsk_mutex_unlock(m_phConsumersMutex);

	// producers
	tsk_mutex_lock(m_phProducersMutex);
	iter_prod = m_oProducers.begin();
	while((iter_prod = find_if(m_oProducers.begin(), m_oProducers.end(), PredOTProducerNotStarted())) != m_oProducers.end())
	{
		m_oProducers.erase(iter_prod);
		//OTObjectSafeRelease(*iter_prod);
	}
	tsk_mutex_unlock(m_phProducersMutex);

	return 0;
}

int OTMixerMgrAudio::stop()
{
	OT_DEBUG_INFO("MixerMgr audio stop");

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

int OTMixerMgrAudio::attachMediaPlugins(OTObjectWrapper<OTSessionInfo*> pOTSessionInfo)
{
	int ret = 0;
	OTObjectWrapper<OTProxyPluginProducerAudio*> pOTPluginProducerAudio;
	OTObjectWrapper<OTProxyPluginConsumerAudio*> pOTPluginConsumerAudio;
	OTObjectWrapper<OTProxyPlugin*> pOTPlugin;

	OTObjectWrapper<OTSessionInfoAV*>pOTSessionInfoAV = dynamic_cast<OTSessionInfoAV*>(*pOTSessionInfo);
	OT_ASSERT(pOTSessionInfoAV);

	//
	//	Consumer
	//
	pOTPlugin = OTProxyPluginMgr::findPlugin(pOTSessionInfoAV->getAudioConsumerId());
	if(!pOTPlugin)
	{
		OT_DEBUG_ERROR("Cannot find audio consumer with id=%llu", pOTSessionInfoAV->getAudioConsumerId());
		return -2;
	}

	tsk_mutex_lock(m_phConsumersMutex); // lock()
	pOTPluginConsumerAudio = dynamic_cast<OTProxyPluginConsumerAudio*>(*pOTPlugin);
	OT_ASSERT(pOTPluginConsumerAudio);
	
	pOTPluginConsumerAudio->setSessionInfo(pOTSessionInfo);
	m_oConsumers[pOTSessionInfoAV->getAudioConsumerId()] = pOTPluginConsumerAudio;

	tsk_mutex_unlock(m_phConsumersMutex); // unlock()

	//
	//	Producer
	//
	pOTPlugin = OTProxyPluginMgr::findPlugin(pOTSessionInfoAV->getAudioProducerId());
	if(!pOTPlugin)
	{
		OT_DEBUG_ERROR("Cannot find audio producer with id=%llu", pOTSessionInfoAV->getAudioProducerId());
		return -3;
	}
	tsk_mutex_lock(m_phProducersMutex); // lock()
	pOTPluginProducerAudio = dynamic_cast<OTProxyPluginProducerAudio*>(*pOTPlugin);
	OT_ASSERT(pOTPluginProducerAudio);

	pOTPluginProducerAudio->setSessionInfo(pOTSessionInfo);
	m_oProducers[pOTSessionInfoAV->getAudioProducerId()] = dynamic_cast<OTProxyPluginProducerAudio*>(*pOTPlugin);
	tsk_mutex_unlock(m_phProducersMutex); // unlock()

	
	return ret;
}

int OTMixerMgrAudio::deAttachMediaPlugins(OTObjectWrapper<OTSessionInfo*> pOTSessionInfo)
{
	OTObjectWrapper<OTSessionInfoAV*>pOTSessionInfoAV = dynamic_cast<OTSessionInfoAV*>(*pOTSessionInfo);
	OT_ASSERT(pOTSessionInfoAV);

	//
	//	Consumer
	//
	tsk_mutex_lock(m_phConsumersMutex);
	m_oConsumers.erase(pOTSessionInfoAV->getAudioConsumerId());
	tsk_mutex_unlock(m_phConsumersMutex);

	if(m_oMixerAudio)
	{
		m_oMixerAudio->releaseConsumerInternals(pOTSessionInfoAV->getAudioConsumerId());
	}

	//
	//	Producer
	//

	tsk_mutex_lock(m_phProducersMutex);
	m_oProducers.erase(pOTSessionInfoAV->getAudioProducerId());
	tsk_mutex_unlock(m_phProducersMutex);

	return 0;
}

bool OTMixerMgrAudio::isValid()
{
	return m_bValid;
}

void* TSK_STDCALL OTMixerMgrAudio::pullThreadFunc(void *arg)
{
	OTMixerMgrAudio *This = (OTMixerMgrAudio *)arg;
	int64_t nDelay = 0;
	int64_t nPtime = This->m_oBridgeInfo->getAudioPtime();

	OT_DEBUG_INFO(" audio pullThreadFunc ENTER ");

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

		nDelay = 0;
		uint64_t nNow = tsk_time_now();
		if(This->m_nLastTimerPull)
		{
			nDelay = (nNow - This->m_nLastTimerPull) - nPtime;
			nDelay = TSK_CLAMP(0, nDelay, nPtime);
		}
		
		This->m_nLastTimerPull = nNow;
	}
	
	OT_DEBUG_INFO(" audio pullThreadFunc EXIT ");

	return NULL;
}

void OTMixerMgrAudio::resetConsumersJitter()
{
	std::map<uint64_t, OTObjectWrapper<OTProxyPluginConsumerAudio*> >::const_iterator iter_cons;
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

	if(m_oMixerAudio)
	{
		m_oMixerAudio->reset();
	}
}

int OTMixerMgrAudio::mixAndSend()
{
	std::map<uint64_t, OTObjectWrapper<OTProxyPluginConsumerAudio*> >::const_iterator iter_cons;
	std::map<uint64_t, OTObjectWrapper<OTProxyPluginProducerAudio*> >::const_iterator iter_prod;
	
	bool bAudioLoopback = m_oBridgeInfo->isAudioLoopback(); // used for debuggin only
	bool bRecorderOrSelfDone = false;
	bool bHasAtLeastOneValidSession = false;
	double dMaxVolume, dMaxVolumeAvg = 0.0, dMaxVolumeNow = 0.0;
	OTObjectWrapper<OTSessionInfo*> oSpeakerSessionInfo, oSpeakingSessionInfo;

	std::map<uint64_t, OTObjectWrapper<OTProxyPluginConsumerAudio*> > _oConsumers = m_oConsumers; // ref() to avoid locking
	std::map<uint64_t, OTObjectWrapper<OTProxyPluginProducerAudio*> > _oProducers = m_oProducers; // ref() to avoid locking
	OTObjectWrapper<OTFrameAudio *> oMixedFrame;

	//
	//	Consumers
	//
	
	for(iter_cons = _oConsumers.begin(); iter_cons != _oConsumers.end(); )
	{
		if(!(*iter_cons).second->isValid() || !(*iter_cons).second->isStarted())
		{
			goto next_iter_cons_1;
		}

		// Pull data and hold in internal buffer
		(*iter_cons).second->pullAndHold();

		// Check volume and set speaker
		(*iter_cons).second->setSessionInfoSpeaker(false); // reset()
		(*iter_cons).second->setSessionInfoSpeaking(false); // reset()
		if(!dynamic_cast<OTSessionInfoAV*>(*(*iter_cons).second->getSessionInfo())->isMuteRemote())
		{
			if((dMaxVolume = (*iter_cons).second->getVolumeNow()) >= dMaxVolumeNow)
			{
				oSpeakingSessionInfo = (*iter_cons).second->getSessionInfo();
				dMaxVolumeNow = dMaxVolume;
			}
			if((dMaxVolume = (*iter_cons).second->getVolumeAvg()) >= dMaxVolumeAvg)
			{
				oSpeakerSessionInfo = (*iter_cons).second->getSessionInfo();
				dMaxVolumeAvg = dMaxVolume;
			}
		}

next_iter_cons_1:
			++iter_cons;
	}

	if(oSpeakerSessionInfo)
	{
		oSpeakerSessionInfo->setSpeaker(true);
	}
	if(oSpeakingSessionInfo)
	{
		oSpeakingSessionInfo->setSpeaking(true);
	}

	//
	//	Producers
	//

	for(iter_prod = _oProducers.begin(); iter_prod != _oProducers.end(); )
	{
		if(!(*iter_prod).second->isValid())
		{
			goto next_iter_prod;
		}
		
		bHasAtLeastOneValidSession = true;

		if(!(*iter_prod).second->isStarted() || dynamic_cast<OTSessionInfoAV*>(*(*iter_prod).second->getSessionInfo())->isMuteRemote())
		{
			goto next_iter_prod;
		}

		// echo or record
		if(!bRecorderOrSelfDone && (m_oRecorder || bAudioLoopback))
		{
			bRecorderOrSelfDone = true;
			// mix all
			if((oMixedFrame = m_oMixerAudio->mix(&_oConsumers)) && oMixedFrame->getValidDataSize() > 0)
			{
				if(m_oRecorder)
				{
					m_oRecorder->writeRawAudioPayload(oMixedFrame->getBufferPtr(), oMixedFrame->getValidDataSize());
				}
				if(bAudioLoopback)
				{
					// send echo
					(*iter_prod).second->push(oMixedFrame->getBufferPtr(), oMixedFrame->getValidDataSize());
				}
			}
		}
		
		// mix and send
		if((oMixedFrame = m_oMixerAudio->mix(&_oConsumers, (*iter_prod).second->getSessionInfoConsumerId(OTMediaType_Audio))) && oMixedFrame->getValidDataSize() > 0)
		{
			(*iter_prod).second->push(oMixedFrame->getBufferPtr(), oMixedFrame->getValidDataSize());
		}

next_iter_prod:
		++iter_prod;
	}//end-of-producers

	// pause the timer if there is no consumer/producer
	// will be reStarted when consumers are added
	if(!bHasAtLeastOneValidSession)
	{
		OT_DEBUG_INFO("Pausing the 'audio' mixer because no active session left...");
		pause();
	}
		
	return 0;
}