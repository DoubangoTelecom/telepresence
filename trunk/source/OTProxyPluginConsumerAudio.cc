/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#include "opentelepresence/OTProxyPluginConsumerAudio.h"

#include <math.h>
#include <assert.h>

#include "tsk_time.h"

//
//	OTProxyPluginConsumerAudio
//
OTProxyPluginConsumerAudio::OTProxyPluginConsumerAudio(uint64_t nId, const ProxyAudioConsumer* pcConsumer)
: OTProxyPluginConsumer(OTMediaType_Audio, nId, dynamic_cast<const ProxyPlugin*>(pcConsumer)), m_pcWrappedConsumer(pcConsumer)
, m_nPtime(OPENTELEPRESENCE_AUDIO_PTIME_DEFAULT)
, m_nRate(OPENTELEPRESENCE_AUDIO_RATE_DEFAULT)
, m_nChannels(OPENTELEPRESENCE_AUDIO_CHANNELS_DEFAULT)
, m_nBitsPerSample(OPENTELEPRESENCE_AUDIO_BITS_PER_SAMPLE_DEFAULT)
, m_nMaxLatency(OPENTELEPRESENCE_AUDIO_MAX_LATENCY)
, m_bSettingsChanged(false)
{
	m_nVolumeComputeCount = 0;
	m_nVolumeNow = m_nVolumeAvg = 0.0;
	m_pHeldBufferPtr = NULL;
	m_nHeldBufferSizeInBytes = m_nHeldBufferSizeInSamples = 0;
	m_nHeldBufferPos = 0;
	m_nRTPPktPullCount = 0;
	m_nRTPPktPullTime = 0;

	if((m_pCallback = new OTProxyPluginConsumerAudioCallback(this)))
	{
		const_cast<ProxyAudioConsumer*>(m_pcWrappedConsumer)->setCallback(m_pCallback);
	}
}

OTProxyPluginConsumerAudio::~OTProxyPluginConsumerAudio()
{
	OT_DEBUG_INFO("*** OTProxyPluginConsumerAudio destroyed ***");
	m_pcWrappedConsumer = NULL;
	if(m_pCallback)
	{
		delete m_pCallback, m_pCallback = NULL;
	}
	TSK_FREE(m_pHeldBufferPtr);
}

int OTProxyPluginConsumerAudio::prepareCallback(int nPtime, int nRate, int nChannels)
{
	m_nPtime = nPtime;
	m_nRate = nRate;
	m_nChannels = nChannels;

	if(m_oSessionInfo)
	{
		m_nPtime = m_oSessionInfo->getBridgeInfo()->getAudioPtime();
		m_nRate = m_oSessionInfo->getBridgeInfo()->getAudioSampleRate();
		m_nChannels = m_oSessionInfo->getBridgeInfo()->getAudioChannels();
		m_nBitsPerSample = m_oSessionInfo->getBridgeInfo()->getAudioBitsPerSample();
		m_nMaxLatency = m_oSessionInfo->getBridgeInfo()->getAudioMaxLatency();

		// use pivot settings to force Doubango to create a resampler if required
		if(!const_cast<ProxyAudioConsumer*>(m_pcWrappedConsumer)->setActualSndCardPlaybackParams(m_nPtime, m_nRate, m_nChannels))
		{
			return -1;
		}
	}
	else
	{
		OT_DEBUG_ERROR("Session info is NULL");
	}
	
	m_bSettingsChanged = true;

	return OTProxyPlugin::prepare();
}

int OTProxyPluginConsumerAudio::startCallback()
{
	// call parent
	return OTProxyPlugin::start();
}

int OTProxyPluginConsumerAudio::pauseCallback()
{
	// call parent
	return OTProxyPlugin::pause();
}

int OTProxyPluginConsumerAudio::stopCallback()
{
	// call parent
	return OTProxyPlugin::stop();
}

unsigned OTProxyPluginConsumerAudio::pullAndHold()
{
	if(!isPrepared())
	{
		return 0;
	}

	if(!m_pHeldBufferPtr || m_bSettingsChanged)
	{
		m_nHeldBufferSizeInBytes = (m_nRate * (m_nBitsPerSample >> 3) * m_nChannels * m_nPtime)/1000;
		m_nHeldBufferSizeInSamples = m_nHeldBufferSizeInBytes / (m_nBitsPerSample >> 3);
		OT_ASSERT((m_pHeldBufferPtr = tsk_realloc(m_pHeldBufferPtr, m_nHeldBufferSizeInBytes)));
		m_bSettingsChanged = false;
	}
	
	bool bReset = false;
	uint64_t nNow = tsk_time_now();

	if(m_nRTPPktPullCount > 0)
	{
		uint64_t nExpectedTime = m_nRTPPktPullTime + (m_nRTPPktPullCount * m_nPtime);
		if((nNow - nExpectedTime) > m_nMaxLatency)
		{
			OT_DEBUG_INFO("Too much latency: nExpectedTime=%llu, Now=%llu, MaxAcceptedLatency=%u", nExpectedTime, nNow, m_nMaxLatency);
			bReset = true;
		}
	}
	else
	{
		m_nRTPPktPullTime = nNow;
	}

	m_nHeldBufferPos = pull(m_pHeldBufferPtr, m_nHeldBufferSizeInBytes);
	if(m_nHeldBufferPos)
	{
		m_nVolumeComputeCount += m_nPtime;
		if(m_nVolumeComputeCount >= OPENTELEPRESENCE_AUDIO_TIME_BEFORE_COMPUTING_AVG_VOL)
		{
			m_nVolumeAvg = m_nVolumeNow;
			m_nVolumeComputeCount = 0;
			m_nVolumeNow = 0;
		}
		int16_t* pHeldBufferPtr = (int16_t*)m_pHeldBufferPtr;
		double nValue = 0;
		for(uint32_t i = 0; i< m_nHeldBufferSizeInSamples; ++i)
		{
			nValue += (pHeldBufferPtr[i] > 0) ? 20. * log10((double)TSK_ABS(pHeldBufferPtr[i]) / 32768.) : 72.;
		}
		nValue /= m_nHeldBufferSizeInSamples;
		m_nVolumeNow = TSK_ABS(m_nVolumeNow + nValue);
		if(m_nVolumeComputeCount != 0)
		{
			m_nVolumeNow /= 2.;
		}
		// OT_DEBUG_INFO("m_nVolumeNow=%f", m_nVolumeNow);
	}
	else
	{
		//m_nVolumeNow /= 2;
	}
	
	if(bReset)
	{
		OT_DEBUG_INFO("/!\\ Too much delay in audio channel. Sorry but I have to reset internal jb buffers :(");
		reset();

		m_nRTPPktPullCount = 0;
		m_nRTPPktPullTime = nNow;
	}

	++m_nRTPPktPullCount;

	return m_nHeldBufferPos;
}

unsigned OTProxyPluginConsumerAudio::copyFromHeldBuffer(void* output, unsigned size)
{
	if(!output || !size){
		OT_DEBUG_ERROR("Invalid parameter");
		return 0;
	}
	
	unsigned nRetSize = 0;

	if(m_pHeldBufferPtr)
	{
		nRetSize = TSK_MIN(m_nHeldBufferPos, size);
		if(nRetSize)
		{
			memcpy(output, m_pHeldBufferPtr, nRetSize);
			if(nRetSize < size)
			{
				// complete with silence
				memset(((uint8_t*)output) + nRetSize, 0, (size - nRetSize));
			}
		}
	}

	return nRetSize;
}
