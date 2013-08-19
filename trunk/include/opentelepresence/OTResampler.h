/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_RESAMPLER_AUDIO_H
#define OPENTELEPRESENCE_RESAMPLER_AUDIO_H

#include "OpenTelepresenceConfig.h"
#include "opentelepresence/OTObject.h"
#include "opentelepresence/OTCommon.h"

class OTResamplerAudio : public OTObject
{
protected:
	OTResamplerAudio(
			size_t nInChannels, size_t nOutChannels,
			size_t nInBitsPerSample, size_t nOutBitsPerSample,
			size_t nInSampleRate, size_t nOutSampleRate,
			size_t nPtime);
public:
	virtual ~OTResamplerAudio();
	virtual OT_INLINE const char* getObjectId() { return "OTResamplerAudio"; }

	virtual size_t resample(const void* pInBufferPtr, size_t nInBufferSizeInSamples, void* pOutBufferPtr, size_t nOutBufferSizeInSamples) = 0;
	virtual bool isValid() = 0;

	static bool registerPlugin();
	static  OTObjectWrapper<OTResamplerAudio*> New(size_t nInChannels, size_t nOutChannels,
			size_t nInBitsPerSample, size_t nOutBitsPerSample,
			size_t nInSampleRate, size_t nOutSampleRate,
			size_t nPtime);

	virtual OT_INLINE size_t getInChannels(){ return m_nInChannels; }
	virtual OT_INLINE size_t getOutChannels(){ return m_nOutChannels; }
	virtual OT_INLINE size_t getInBitsPerSample(){ return m_nInBitsPerSample; }
	virtual OT_INLINE size_t getOutBitsPerSample(){ return m_nOutBitsPerSample; }
	virtual OT_INLINE size_t getInSampleRate(){ return m_nInSampleRate; }
	virtual OT_INLINE size_t getOutSampleRate(){ return m_nOutSampleRate; }
	virtual OT_INLINE size_t getPtime(){ return m_nPtime; }
	virtual OT_INLINE size_t getInBytesPerSample(){ return m_nInBytesPerSample; }
	virtual OT_INLINE size_t getOutBytesPerSample(){ return m_nOutBytesPerSample; }
	virtual OT_INLINE size_t getExpectedInBufferSizeInSamples(){ return m_nExpectedInBufferSizeInSamples; }
	virtual OT_INLINE size_t getExpectedOutBufferSizeInSamples(){ return m_nExpectedOutBufferSizeInSamples; }

protected:
	size_t m_nInChannels, m_nOutChannels;
	size_t m_nInBitsPerSample, m_nOutBitsPerSample;
	size_t m_nInSampleRate, m_nOutSampleRate;
	size_t m_nPtime;
	size_t m_nInBytesPerSample, m_nOutBytesPerSample;
	size_t m_nExpectedInBufferSizeInSamples, m_nExpectedOutBufferSizeInSamples;
};

#endif /* OPENTELEPRESENCE_RESAMPLER_AUDIO_H */
