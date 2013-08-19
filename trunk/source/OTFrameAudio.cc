/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#include "opentelepresence/OTFrameAudio.h"

OTFrameAudio::OTFrameAudio(bool bOwnBuffer, const void *pBufferPtr, uint32_t nBufferSize)
: OTFrame(OTMediaType_Audio, bOwnBuffer, pBufferPtr, nBufferSize)
, m_fVolume(OPENTELEPRESENCE_AUDIO_MIXER_VOL)
, m_nBitsPerSample(OPENTELEPRESENCE_AUDIO_BITS_PER_SAMPLE_DEFAULT)
, m_nChannels(OPENTELEPRESENCE_AUDIO_CHANNELS_DEFAULT)
, m_nSampleRate(OPENTELEPRESENCE_AUDIO_RATE_DEFAULT)
{
	
}

OTFrameAudio::~OTFrameAudio()
{

}

OTObjectWrapper<OTFrameAudio*> OTFrameAudio::New(bool bOwnBuffer, const void *pBufferPtr, uint32_t nBufferSize)
{
	return new OTFrameAudio(bOwnBuffer, pBufferPtr, nBufferSize);
}
