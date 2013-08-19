/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#include "opentelepresence/OTProxyPluginProducerAudio.h"

#include "tsk_thread.h"

OTProxyPluginProducerAudio::OTProxyPluginProducerAudio(uint64_t nId, const ProxyAudioProducer* pcProducer)
: OTProxyPluginProducer(OTMediaType_Audio, nId, dynamic_cast<const ProxyPlugin*>(pcProducer))
, m_nPtime(OPENTELEPRESENCE_AUDIO_PTIME_DEFAULT)
, m_nRate(OPENTELEPRESENCE_AUDIO_RATE_DEFAULT)
, m_nChannels(OPENTELEPRESENCE_AUDIO_CHANNELS_DEFAULT)
, m_nBitsPerSample(OPENTELEPRESENCE_AUDIO_BITS_PER_SAMPLE_DEFAULT)
{
	m_phSenderThread[0] = tsk_null;
	m_phSenderSemaphore = tsk_null;
	m_pcWrappedProducer = pcProducer;
	m_pCallback = new OTProxyPluginProducerAudioCallback(this);
	const_cast<ProxyAudioProducer*>(m_pcWrappedProducer)->setCallback(m_pCallback);
}

OTProxyPluginProducerAudio::~OTProxyPluginProducerAudio()
{
	OT_DEBUG_INFO("*** OTProxyPluginProducerAudio destroyed ***");
	m_pcWrappedProducer = NULL;
	if(m_pCallback)
	{
		delete m_pCallback, m_pCallback = NULL;
	}
	if(m_phSenderThread[0])
	{
		tsk_thread_join(&(m_phSenderThread[0]));
	}
	if(m_phSenderSemaphore)
	{
		tsk_semaphore_destroy(&m_phSenderSemaphore);
	}
}

int OTProxyPluginProducerAudio::prepareCallback(int ptime, int rate, int channels)
{
	m_nPtime = ptime;
	m_nRate = rate;
	m_nChannels = channels;

	if(m_oSessionInfo)
	{
		m_nPtime = m_oSessionInfo->getBridgeInfo()->getAudioPtime();
		m_nRate = m_oSessionInfo->getBridgeInfo()->getAudioSampleRate();
		m_nChannels = m_oSessionInfo->getBridgeInfo()->getAudioChannels();
		m_nBitsPerSample = m_oSessionInfo->getBridgeInfo()->getAudioBitsPerSample();

		// use pivot settings to force Doubango to create a resampler if required
		if(!const_cast<ProxyAudioProducer*>(m_pcWrappedProducer)->setActualSndCardRecordParams(m_nPtime, m_nRate, m_nChannels))
		{
			return -1;
		}
	}
	else
	{
		OT_DEBUG_ERROR("Session info is NULL");
	}

	// call parent
	return OTProxyPlugin::prepare();
}

int OTProxyPluginProducerAudio::startCallback()
{
	// create audio buffer
	size_t nExpectFrameSize = (m_nRate * (m_nBitsPerSample >> 3) * m_nChannels * m_nPtime)/1000;
	m_oFrame = OTFrameAudio::New(true, NULL, nExpectFrameSize);
	if(!m_oFrame || (m_oFrame->getBufferSize() != nExpectFrameSize))
	{
		OT_DEBUG_ERROR("Failed to allocate audio frame with size = %u", nExpectFrameSize);
		return -1;
	}

	// create thread if not already done
	if(!m_phSenderSemaphore && !(m_phSenderSemaphore = tsk_semaphore_create()))
	{
		OT_DEBUG_ERROR("Failed to create semaphore");
		return -1;
	}

	// create thread if not already done
	if(!m_phSenderThread[0])
	{
		int ret = tsk_thread_create(&m_phSenderThread[0], OTProxyPluginProducerAudio::senderThread, this);
		if(ret)
		{
			OT_DEBUG_ERROR("tsk_thread_create failed with error code = %d", ret);
			return ret;
		}
	}

	// call parent
	return OTProxyPlugin::start();
}

int OTProxyPluginProducerAudio::push(const void* buffer, unsigned size)
{
	int ret = 0;

	if(!m_oFrame || !m_phSenderSemaphore)
	{
		OT_DEBUG_ERROR("Invalid state");
		return -1;
	}

	if(m_oFrame->getBufferSize() != size)
	{
		OT_DEBUG_ERROR("Unexpected input audio buffer size %u<>%u", m_oFrame->getBufferSize(), size);
		return -1;
	}

	// lock() frame
	m_oFrame->lock();
	// copy() audio samples
	memcpy(m_oFrame->getBufferPtr(), buffer, size);
	// increment semaphore to release the async thread
	if((ret = tsk_semaphore_increment(m_phSenderSemaphore)))
	{
		OT_DEBUG_ERROR("tsk_semaphore_increment() failed with error code=%d", ret);
		goto done;
	}

done:
	// unlock()
	m_oFrame->unlock();

	return ret;
}

int OTProxyPluginProducerAudio::pauseCallback()
{
	// call parent
	return OTProxyPlugin::pause();
}

int OTProxyPluginProducerAudio::stopCallback()
{
	int ret;
	// call parent
	if(ret = OTProxyPlugin::stop())
	{
		OT_DEBUG_ERROR("Failed to stop producer");
		return ret;
	}
	// now isStarted() will return false which means that the thread will exit
	if(m_phSenderSemaphore)
	{
		if((ret = tsk_semaphore_increment(m_phSenderSemaphore)))
		{
			OT_DEBUG_ERROR("tsk_semaphore_increment() failed with error code=%d", ret);
			return ret;
		}
	}
	if(m_phSenderThread[0])
	{
		if((ret = tsk_thread_join(&(m_phSenderThread[0]))))
		{
			OT_DEBUG_ERROR("tsk_thread_join() failed with error code=%d", ret);
			return ret;
		}
	}

	return ret;
}

void* TSK_STDCALL OTProxyPluginProducerAudio::senderThread(void *arg)
{
	OTProxyPluginProducerAudio *This = (OTProxyPluginProducerAudio*)arg;

	while(This->isStarted() && This->m_pcWrappedProducer && This->m_oFrame)
	{
		tsk_semaphore_decrement(This->m_phSenderSemaphore);
		if(!This->isStarted() || !This->m_pcWrappedProducer || !This->m_oFrame)
		{
			break;
		}
		
		This->m_oFrame->lock();
		const_cast<ProxyAudioProducer*>(This->m_pcWrappedProducer)->push(This->m_oFrame->getBufferPtr(), This->m_oFrame->getBufferSize());
		This->m_oFrame->unlock();
	}

	OT_DEBUG_INFO("OTProxyPluginProducerAudio::SenderThread::Exit");
	
	return tsk_null;
}
