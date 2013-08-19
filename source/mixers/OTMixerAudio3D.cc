/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#include "opentelepresence/mixers/OTMixerAudio3D.h"

#include "tsk_debug.h"

#include <assert.h>
#include <map>

OTMixerAudio3D::OTMixerAudio3D(OTObjectWrapper<OTBridgeInfo*> oBridgeInfo)
:OTMixerAudio(oBridgeInfo)
{
	m_bValid = oBridgeInfo 
		&& getMediaType() == OTMediaType_Audio 
		&& oBridgeInfo->getAudioDimension() == OTDimension_3D;
}

OTMixerAudio3D::~OTMixerAudio3D()
{
}

int OTMixerAudio3D::reset()
{
	return 0;
}

bool OTMixerAudio3D::isValid()
{
	return m_bValid;
}

////////////////
#if HAVE_OPENAL

#if !defined(AL_ALEXT_PROTOTYPES)
#	define AL_ALEXT_PROTOTYPES
#endif

#if !defined(OT_AL_MAX_SOURCES)
#	define OT_AL_MAX_SOURCES 256 /* Default value from OpenAL when 'sources' not defined in 'alsoft.ini' */
#endif

#include <AL/alc.h>
#include <AL/al.h>
#include <AL/alext.h>

//
//	OTMixerAudioAL(Declaration)
//

class OTMixerAudioAL : public OTMixerAudio3D
{
	class OTMixerAudioSourceAL : public OTObject
	{
		friend class OTMixerAudioAL;
	public:
		OTMixerAudioSourceAL(ALCcontext *pALCtx, OTObjectWrapper<OTProxyPluginConsumerAudio*>oConsumer)
			: m_pALCtx(pALCtx)
			, m_nConsumerId(oConsumer->getId())
			, m_uSrc(0)
			, m_uBuffer(0)
		{
			alcMakeContextCurrent(m_pALCtx);
			alGenBuffers(1, &m_uBuffer);
			alGenSources(1, &m_uSrc);

			const OT3f_t*pcPosition = oConsumer->getSessionInfo()->getAudioPosition();
			const OT3f_t*pcVelocity = oConsumer->getSessionInfo()->getAudioVelocity();

			alSourcef(m_uSrc, AL_PITCH, 1.0f);
			alSourcef(m_uSrc, AL_GAIN, 1.0f);
			if(pcPosition)
			{
				alSource3f(m_uSrc, AL_POSITION, pcPosition->x, pcPosition->y, pcPosition->z);
			}
			if(pcVelocity)
			{
				alSource3f(m_uSrc, AL_VELOCITY, pcVelocity->x, pcVelocity->y, pcVelocity->y);
			}
			alSource3f(m_uSrc, AL_DIRECTION, 0.0f, 0.0f, 0.0f);
			//alSourcef (m_uSrc, AL_ROLLOFF_FACTOR, 0.0);
			//alSourcei (m_uSrc, AL_SOURCE_RELATIVE, AL_TRUE);
			alSourcei(m_uSrc, AL_LOOPING, AL_FALSE);
			//alSourcei(m_uSrc, AL_BUFFER, m_uBuffer);
		}
		virtual ~OTMixerAudioSourceAL()
		{
			alcMakeContextCurrent(m_pALCtx);
			alDeleteSources(1, &m_uSrc);
			alDeleteBuffers(1, &m_uBuffer);
			OT_DEBUG_INFO("*** OTMixerAudioSourceAL destroyed ***");
		}
		virtual OT_INLINE const char* getObjectId() { return "OTMixerAudioSourceAL"; }

	private:
		uint64_t m_nConsumerId;
		ALCcontext *m_pALCtx;
		ALuint m_uSrc, m_uBuffer;
	};

public:
	OTMixerAudioAL(OTObjectWrapper<OTBridgeInfo*> oBridgeInfo);
	virtual ~OTMixerAudioAL();
	virtual OT_INLINE const char* getObjectId() { return "OTMixerAudioAL"; }

	// @Override(OTMixerAudio)
	virtual bool releaseConsumerInternals(uint64_t nConsumerId);

	// @Overrid(OTMixerAudio3D)
	virtual bool isValid();
	virtual OTObjectWrapper<OTFrameAudio *> mix(std::map<uint64_t, OTObjectWrapper<OTProxyPluginConsumerAudio*> >*, uint64_t nConsumerToIgnore = 0);

private:
	void _deInit();

private:
	ALCdevice *m_pALDevice;
	ALCcontext *m_pALCtx;
	std::map<uint64_t, OTObjectWrapper<OTMixerAudioSourceAL*> > m_oALSources;
	void* m_pTempBuffer;
	size_t m_nTempBufferSize;
	OTObjectWrapper<OTFrameAudio *> m_oFrame;
	ALuint m_alTmpSources[OT_AL_MAX_SOURCES];
};

//
//	OTMixerAudioAL(Implementation)
//

OTMixerAudioAL::OTMixerAudioAL(OTObjectWrapper<OTBridgeInfo*> oBridgeInfo)
: OTMixerAudio3D(oBridgeInfo)
, m_pALDevice(NULL)
, m_pALCtx(NULL)
, m_pTempBuffer(NULL)
, m_nTempBufferSize(0)
{
	m_pALDevice = alcLoopbackOpenDeviceSOFT(NULL);
	if(!m_pALDevice)
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameOpenAL, "Failed to create new device");
		return;
	}

	ALCenum alChannels, alSampleType;
	switch(oBridgeInfo->getAudioChannels())
	{
		case 1: alChannels = ALC_MONO_SOFT; break;
		case 2: alChannels = ALC_STEREO_SOFT; break;
		default: OT_DEBUG_ERROR_EX(kOTMobuleNameOpenAL, "%d not valid as audio channels count", oBridgeInfo->getAudioChannels()); _deInit(); return;
	}
	switch(oBridgeInfo->getAudioBitsPerSample())
	{
		case 8: alSampleType = ALC_BYTE_SOFT; break;
		case 16: alSampleType = ALC_SHORT_SOFT; break;
		case 32: alSampleType = ALC_FLOAT_SOFT; break;
		default: OT_DEBUG_ERROR_EX(kOTMobuleNameOpenAL, "%d not valid as audio bits per sample", oBridgeInfo->getAudioBitsPerSample()); _deInit(); return;
	}

	if(alcIsRenderFormatSupportedSOFT(m_pALDevice, oBridgeInfo->getAudioSampleRate(), alChannels, alSampleType) == ALC_FALSE)
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameOpenAL, "alcIsRenderFormatSupportedSOFT(%d, %d, %d) returned false", oBridgeInfo->getAudioSampleRate(), alChannels, alSampleType);
		_deInit();
		return;
	}

	ALCint attr[7];
	attr[0] = ALC_FORMAT_CHANNELS_SOFT;
	attr[1] = alChannels;
	attr[2] = ALC_FORMAT_TYPE_SOFT;
	attr[3] = alSampleType;
	attr[4] = ALC_FREQUENCY;
	attr[5] = oBridgeInfo->getAudioSampleRate();
	attr[6] = 0;
	m_pALCtx = alcCreateContext(m_pALDevice, attr);
	if(!m_pALCtx)
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameOpenAL, "alcCreateContext(%d, %d, %d) returned false", oBridgeInfo->getAudioSampleRate(), alChannels, alSampleType);
		_deInit();
		return;
	}

	if(alcMakeContextCurrent(m_pALCtx) == ALC_FALSE)
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameOpenAL, "alcMakeContextCurrent() failed");
		_deInit();
		return;
	}
}

OTMixerAudioAL::~OTMixerAudioAL()
{
	_deInit();
	OT_DEBUG_INFO("*** OTMixerAudioAL destroyed ***");
}

// @Override(OTMixerAudio)
bool OTMixerAudioAL::releaseConsumerInternals(uint64_t nConsumerId)
{
	m_oALSources.erase(nConsumerId);
	return true;
}

// @Overrid(OTMixerAudio3D)
bool OTMixerAudioAL::isValid()
{
	if(OTMixerAudio3D::isValid()) // base.isValid()
	{
		return (m_pALCtx && m_pALDevice);
	}
	return false;
}

// @Overrid(OTMixerAudio3D)
OTObjectWrapper<OTFrameAudio *> OTMixerAudioAL::mix(std::map<uint64_t, OTObjectWrapper<OTProxyPluginConsumerAudio*> >*pConsumers, uint64_t nConsumerToIgnore /*= 0*/)
{
	if(!isValid())
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameOpenAL, "Invalid");
		return false;
	}

	if(!m_nTempBufferSize || !m_oFrame)
	{
		OT_ASSERT(m_oBridgeInfo->getAudioBitsPerSample() == 16);
		size_t nTempBufferSize = (m_oBridgeInfo->getAudioSampleRate() * (m_oBridgeInfo->getAudioBitsPerSample() >> 3) * m_oBridgeInfo->getAudioChannels() * m_oBridgeInfo->getAudioPtime())/1000;
		if(!(m_pTempBuffer = tsk_realloc(m_pTempBuffer, nTempBufferSize)))
		{
			OT_DEBUG_ERROR_EX(kOTMobuleNameOpenAL, "Failed to alloc buffer with size = %u", nTempBufferSize);
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

	std::map<uint64_t, OTObjectWrapper<OTProxyPluginConsumerAudio*> >::iterator iter;
	
	ALsizei nALSourcesCount = 0;
	OTObjectWrapper<OTMixerAudioSourceAL*> oSource;
	size_t nCopiedSizeInBytes, nCopiedSizeInBytesMax = 0;

	ALenum err;

	alcMakeContextCurrent(m_pALCtx);

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
			oSource = m_oALSources[iter->second->getId()];
			if(!oSource)
			{
				oSource = new OTMixerAudioSourceAL(m_pALCtx, iter->second);
				m_oALSources[oSource->m_nConsumerId] = oSource;
			}

			m_alTmpSources[nALSourcesCount++] = oSource->m_uSrc;
			alSourceUnqueueBuffers(oSource->m_uSrc, 1, &oSource->m_uBuffer);
			err = alGetError();
			alBufferData(
				oSource->m_uBuffer, 
				m_oFrame->getChannels() == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16, 
				m_pTempBuffer, 
				m_nTempBufferSize, 
				m_oFrame->getSampleRate()
				);
			err = alGetError();
			alSourceQueueBuffers(oSource->m_uSrc, 1, &oSource->m_uBuffer);
			err = alGetError();

			if(nALSourcesCount >= OT_AL_MAX_SOURCES)
			{
				OT_DEBUG_ERROR_EX(kOTMobuleNameOpenAL, "UnExpected code called: More than %u AL active sources(%u)", OT_AL_MAX_SOURCES, nALSourcesCount);
				break;
			}
		}
	}
	
	if(nALSourcesCount && nCopiedSizeInBytesMax)
	{	
		alSourcePlayv(nALSourcesCount, m_alTmpSources);
		err = alGetError();
		alcRenderSamplesSOFT(
				m_pALDevice, 
				m_oFrame->getBufferPtr(), 
				(nCopiedSizeInBytesMax >> 1) /* Assume BitsPerSample = 2*/
			);
		err = alGetError();
		m_oFrame->setValidDataSize(nCopiedSizeInBytesMax);
		return m_oFrame;
	}
	
	return NULL;
}

void OTMixerAudioAL::_deInit()
{
	m_oALSources.clear(); // must be done before destroying the context

	if(m_pALCtx)
	{
		alcDestroyContext(m_pALCtx);
		m_pALCtx = NULL;
	}

	if(m_pALDevice)
	{
		alcCloseDevice(m_pALDevice);
		m_pALDevice = NULL;
	}
	TSK_FREE(m_pTempBuffer);
}


#endif /* HAVE_OPENAL */
////////


OTObjectWrapper<OTMixerAudio3D*> OTMixerAudio3D::New(OTObjectWrapper<OTBridgeInfo*> oBridgeInfo)
{
#if HAVE_OPENAL
	OTObjectWrapper<OTMixerAudio3D*> oMixer = new OTMixerAudioAL(oBridgeInfo);
	if(oMixer && oMixer->isValid())
	{
		return oMixer;
	}
	return NULL;
#endif
}
