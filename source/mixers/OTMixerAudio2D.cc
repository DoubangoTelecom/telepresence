/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#include "opentelepresence/mixers/OTMixerAudio2D.h"

#include "tsk_memory.h"
#include "tsk_debug.h"

#include <assert.h>

OTMixerAudio2D::OTMixerAudio2D(OTObjectWrapper<OTBridgeInfo*> oBridgeInfo)
: OTMixerAudio(oBridgeInfo)
, m_pTempBuffer(NULL)
, m_nTempBufferSize(0)
{
	m_bValid = oBridgeInfo 
		&& getMediaType() == OTMediaType_Audio 
		&& oBridgeInfo->getAudioDimension() == OTDimension_2D;
}

OTMixerAudio2D::~OTMixerAudio2D()
{
	TSK_FREE(m_pTempBuffer);

	OT_DEBUG_INFO("*** OTMixerAudio2D destroyed ***");
}

int OTMixerAudio2D::reset()
{
	return 0;
}

bool OTMixerAudio2D::isValid()
{
	return m_bValid;
}

//
// http://www.vttoth.com/digimix.htm
//

static void _mix16(const int16_t* pInBufferPtr, float dInBufferVolume, int16_t* pOutBufferPtr, size_t nSizeToMixInBytes)
{
	register uint32_t i;
	float mixedSample;
	size_t nSizeToMixInSamples = (nSizeToMixInBytes >>1);
	for(i = 0; i<nSizeToMixInSamples; ++i)
	{
		mixedSample = ((float)pOutBufferPtr[i]/32768.0f) + (((float)pInBufferPtr[i]/32768.0f) * dInBufferVolume);
		//FIXME
		//mixedSample = TSK_CLAMP(-1.0f, mixedSample, +1.0f);
		if(mixedSample<-1.0f)mixedSample=-1.0f;
		if(mixedSample>+1.0f)mixedSample=+1.0f;
		pOutBufferPtr[i] = (int16_t)(mixedSample  * 32768.0f);
	}
}

OTObjectWrapper<OTFrameAudio *> OTMixerAudio2D::mix(std::map<uint64_t, OTObjectWrapper<OTProxyPluginConsumerAudio*> >*pConsumers, uint64_t nConsumerToIgnore /*= 0*/)
{
	OT_ASSERT(pConsumers);

	std::map<uint64_t, OTObjectWrapper<OTProxyPluginConsumerAudio*> >::iterator iter;

	if(!m_nTempBufferSize || !m_oFrame)
	{
		OT_ASSERT(m_oBridgeInfo->getAudioBitsPerSample() == 16);
		size_t nTempBufferSize = (m_oBridgeInfo->getAudioSampleRate() * (m_oBridgeInfo->getAudioBitsPerSample() >> 3) * m_oBridgeInfo->getAudioChannels() * m_oBridgeInfo->getAudioPtime())/1000;
		if(!(m_pTempBuffer = tsk_realloc(m_pTempBuffer, nTempBufferSize)))
		{
			OT_DEBUG_ERROR("Failed to alloc buffer with size = %u", nTempBufferSize);
			m_nTempBufferSize = 0;
			return false;
		}
		m_nTempBufferSize = nTempBufferSize;
		if(!(m_oFrame = OTFrameAudio::New(true, NULL, m_nTempBufferSize)))
		{
			return NULL;
		}
		m_oFrame->setBitsPerSample(m_oBridgeInfo->getAudioBitsPerSample());
		m_oFrame->setChannels(m_oBridgeInfo->getAudioChannels());
		m_oFrame->setSampleRate(m_oBridgeInfo->getAudioSampleRate());
		m_oFrame->setVolume(m_oBridgeInfo->getAudioVolume());
	}
	
	bool bMixed = false;
	size_t nCopiedSizeInBytes, nCopiedSizeInBytesMax = 0;

	for(iter = pConsumers->begin(); iter != pConsumers->end(); ++iter)
	{
		if(iter->first == nConsumerToIgnore)
		{
			continue;
		}
		
		nCopiedSizeInBytes = iter->second->copyFromHeldBuffer(m_pTempBuffer, m_nTempBufferSize);
		if(nCopiedSizeInBytes)
		{
			nCopiedSizeInBytesMax = TSK_MAX(nCopiedSizeInBytesMax, nCopiedSizeInBytes);
			if(!bMixed) // First time ?
			{
				memcpy(m_oFrame->getBufferPtr(), m_pTempBuffer, nCopiedSizeInBytes);
			}
			else
			{
				_mix16((const int16_t *)m_pTempBuffer, m_oFrame->getVolume(), (int16_t*)m_oFrame->getBufferPtr(), nCopiedSizeInBytes);
			}
			bMixed = true;
		}
	}

	if(bMixed)
	{
		m_oFrame->setValidDataSize(nCopiedSizeInBytesMax);
		return m_oFrame;
	}

	return NULL;
}

OTObjectWrapper<OTMixerAudio2D*> OTMixerAudio2D::New(OTObjectWrapper<OTBridgeInfo*> oBridgeInfo)
{
	OTObjectWrapper<OTMixerAudio2D*> oMixerAudio2D = new OTMixerAudio2D(oBridgeInfo);
	if(oMixerAudio2D && !oMixerAudio2D->isValid())
	{
		OTObjectSafeRelease(oMixerAudio2D);
	}
	return oMixerAudio2D;
}
