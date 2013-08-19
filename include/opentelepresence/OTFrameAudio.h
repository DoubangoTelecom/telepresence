/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_FRAMEAUDIO_H
#define OPENTELEPRESENCE_FRAMEAUDIO_H

#include "OpenTelepresenceConfig.h"
#include "opentelepresence/OTFrame.h"

class OTFrameAudio : public OTFrame
{
protected:
	OTFrameAudio(bool bOwnBuffer, const void *pBufferPtr, uint32_t nBufferSize);
public:
	virtual ~OTFrameAudio();
	virtual OT_INLINE const char* getObjectId() { return "OTFrameAudio"; }

	virtual OT_INLINE void setVolume(float fVolume) { m_fVolume = fVolume; }
	virtual OT_INLINE float getVolume() { return m_fVolume; }
	virtual OT_INLINE void setBitsPerSample(uint16_t nBitsPerSample) { m_nBitsPerSample = nBitsPerSample; }
	virtual OT_INLINE uint16_t getBitsPerSample() { return m_nBitsPerSample; }
	virtual OT_INLINE void setChannels(uint16_t nChannels) { m_nChannels = nChannels; }
	virtual OT_INLINE uint16_t getChannels(){ return m_nChannels; }
	virtual OT_INLINE void setSampleRate(uint16_t nSampleRate) { m_nSampleRate = nSampleRate; }
	virtual OT_INLINE uint16_t getSampleRate(){ return m_nSampleRate; }

	static OTObjectWrapper<OTFrameAudio*> New(bool bOwnBuffer, const void *pBufferPtr, uint32_t nBufferSize);

private:
	float m_fVolume;
	uint16_t m_nBitsPerSample;
	uint16_t m_nChannels;
	uint16_t m_nSampleRate;
};

#endif /* OPENTELEPRESENCE_FRAMEAUDIO_H */
