/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#include "opentelepresence/OTResampler.h"


#include "tsk_memory.h"

#include "tinymedia/tmedia_resampler.h"

#include <assert.h>

extern "C"
{
	#include <libavformat/avformat.h>
	#include <libswresample/swresample.h>
	#include <libavutil/opt.h>
}

static const char* ot_av_err2str(int errnum)
{
	static char __errbuf[AV_ERROR_MAX_STRING_SIZE] = {0};
    return av_make_error_string(__errbuf, AV_ERROR_MAX_STRING_SIZE, errnum);
}

//
//	OTResamplerAudioFFmpeg (Declaration)
//

class OTResamplerAudioFFmpeg : public OTResamplerAudio
{
public:
	OTResamplerAudioFFmpeg(
			size_t nInChannels, size_t nOutChannels,
			size_t nInBitsPerSample, size_t nOutBitsPerSample,
			size_t nInSampleRate, size_t nOutSampleRate,
			size_t nPtime);
	virtual ~OTResamplerAudioFFmpeg();
	virtual OT_INLINE const char* getObjectId() { return "OTResamplerAudioFFmpeg"; }

	// @Override(OTResamplerAudio)
	virtual bool isValid();
	virtual size_t resample(const void* pInBufferPtr, size_t nInBufferSizeInSamples, void* pOutBufferPtr, size_t nOutBufferSizeInSamples);
private:
	void _deInit();
	static enum AVSampleFormat _getSampleFormat(size_t nBytesPerSample, bool bPlanar = false);

private:
	SwrContext *m_pSwr;
};



//
//	OTResamplerAudioFFmpeg (Plugin)
//


typedef struct ot_resampler_audio_ffmpeg_s
{
	TMEDIA_DECLARE_RESAMPLER;

	OTObjectWrapper<OTResamplerAudio*> oResampler;
}
ot_resampler_audio_ffmpeg_t;

/* plugin: open() */
static int ot_resampler_audio_ffmpeg_open(tmedia_resampler_t* self, uint32_t in_freq, uint32_t out_freq, uint32_t frame_duration, uint32_t in_channels, uint32_t out_channels, uint32_t quality)
{
	ot_resampler_audio_ffmpeg_t* _self = (ot_resampler_audio_ffmpeg_t*)self;
	_self->oResampler = OTResamplerAudio::New(in_channels, out_channels,
			OPENTELEPRESENCE_AUDIO_BITS_PER_SAMPLE_DEFAULT, OPENTELEPRESENCE_AUDIO_BITS_PER_SAMPLE_DEFAULT, // FIXME
			in_freq, out_freq,
			frame_duration);
	if(!_self->oResampler)
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameFFmpegResampler, "Failed to create FFmpeg audio resampler");
		return -1;
	}
	return 0;
}

/* plugin: process() */
static tsk_size_t ot_resampler_audio_ffmpeg_process(tmedia_resampler_t* self, const uint16_t* in_data, tsk_size_t in_size16, uint16_t* out_data, tsk_size_t out_size16)
{
	ot_resampler_audio_ffmpeg_t* _self = (ot_resampler_audio_ffmpeg_t*)self;
	if(!_self->oResampler)
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameFFmpegResampler, "Invalid state");
		return -1;
	}
	return _self->oResampler->resample((const void*)in_data, (size_t)in_size16, (void*)out_data, (size_t)out_size16);
}
/* plugin: close() */
static int ot_resampler_audio_ffmpeg_close(tmedia_resampler_t* self)
{
	ot_resampler_audio_ffmpeg_t* _self = (ot_resampler_audio_ffmpeg_t*)self;
	if(_self)
	{
		OTObjectSafeRelease(_self->oResampler);
	}
	return 0;
}

/* constructor */
static tsk_object_t* ot_resampler_audio_ffmpeg_ctor(tsk_object_t * self, va_list * app)
{
	ot_resampler_audio_ffmpeg_t *resampler = (ot_resampler_audio_ffmpeg_t *)self;
	if(resampler)
	{
		/* init base */
		tmedia_resampler_init(TMEDIA_RESAMPLER(resampler));
		/* init self */
	}
	return self;
}
/* destructor */
static tsk_object_t* ot_resampler_audio_ffmpeg_dtor(tsk_object_t * self)
{ 
	ot_resampler_audio_ffmpeg_t *resampler = (ot_resampler_audio_ffmpeg_t *)self;
	if(resampler)
	{
		/* deinit base */
		tmedia_resampler_deinit(TMEDIA_RESAMPLER(resampler));
		/* deinit self */
		OTObjectSafeRelease(resampler->oResampler);
		OT_DEBUG_INFO_EX(kOTMobuleNameFFmpegResampler, "*** FFmpeg resampler (plugin) destroyed ***");
	}

	return self;
}
/* object definition */
static const tsk_object_def_t ot_resampler_audio_ffmpeg_def_s = 
{
	sizeof(ot_resampler_audio_ffmpeg_t),
	ot_resampler_audio_ffmpeg_ctor, 
	ot_resampler_audio_ffmpeg_dtor,
	tsk_null, 
};
/* plugin definition*/
static const tmedia_resampler_plugin_def_t ot_resampler_audio_ffmpeg_plugin_def_s = 
{
	&ot_resampler_audio_ffmpeg_def_s,
	
	"Audio Resampler based on FFmpeg (From telePresense)",
	
	ot_resampler_audio_ffmpeg_open,
	ot_resampler_audio_ffmpeg_process,
	ot_resampler_audio_ffmpeg_close,
};








//
//	OTResamplerAudio (Implementation)
//

OTResamplerAudio::OTResamplerAudio(
			size_t nInChannels, size_t nOutChannels,
			size_t nInBitsPerSample, size_t nOutBitsPerSample,
			size_t nInSampleRate, size_t nOutSampleRate,
			size_t nPtime)
: m_nInChannels(nInChannels), m_nOutChannels(nOutChannels)
, m_nInBitsPerSample(nInBitsPerSample), m_nOutBitsPerSample(nOutBitsPerSample)
, m_nInSampleRate(nInSampleRate), m_nOutSampleRate(nOutSampleRate)
, m_nPtime(nPtime)
{
		OT_ASSERT(nInChannels == 1 || nInChannels == 2);
		OT_ASSERT(nOutChannels == 1 || nOutChannels == 2);
		OT_ASSERT(nInBitsPerSample == 8 || nInBitsPerSample == 16 || nInBitsPerSample == 32);
		OT_ASSERT(nOutBitsPerSample == 8 || nOutBitsPerSample == 16 || nOutBitsPerSample == 32);
		OT_ASSERT(nInSampleRate > 0 && nOutSampleRate > 0);
		OT_ASSERT(nPtime > 0);

		m_nInBytesPerSample = (m_nInBitsPerSample >> 3);
		m_nOutBytesPerSample = (m_nOutBitsPerSample >> 3);

		m_nExpectedInBufferSizeInSamples = ((m_nInSampleRate * m_nPtime * m_nInChannels) / 1000);
		m_nExpectedOutBufferSizeInSamples = ((m_nOutSampleRate * m_nPtime * m_nOutChannels) / 1000);
}

OTResamplerAudio::~OTResamplerAudio()
{
}


bool OTResamplerAudio::registerPlugin()
{
	OT_DEBUG_INFO_EX(kOTMobuleNameFFmpegResampler, "Register FFmpeg audio resampler");
	return (tmedia_resampler_plugin_register(&ot_resampler_audio_ffmpeg_plugin_def_s) == 0);
}

OTObjectWrapper<OTResamplerAudio*> OTResamplerAudio::New(size_t nInChannels, size_t nOutChannels,
			size_t nInBitsPerSample, size_t nOutBitsPerSample,
			size_t nInSampleRate, size_t nOutSampleRate,
			size_t nPtime)
{
	OTObjectWrapper<OTResamplerAudio*> oResamplerAudio = new OTResamplerAudioFFmpeg(
			nInChannels, nOutChannels,
			nInBitsPerSample, nOutBitsPerSample,
			nInSampleRate, nOutSampleRate,
			nPtime);
	if(oResamplerAudio && oResamplerAudio->isValid())
	{
		return oResamplerAudio;
	}
	return NULL;
}






//
//	OTResamplerAudioFFmpeg (Implementation)
//

OTResamplerAudioFFmpeg::OTResamplerAudioFFmpeg(
			size_t nInChannels, size_t nOutChannels,
			size_t nInBitsPerSample, size_t nOutBitsPerSample,
			size_t nInSampleRate, size_t nOutSampleRate,
			size_t nPtime)
: OTResamplerAudio(nInChannels, nOutChannels, nInBitsPerSample, nOutBitsPerSample, nInSampleRate, nOutSampleRate, nPtime)
, m_pSwr(NULL)
{
	OT_DEBUG_INFO_EX(kOTMobuleNameFFmpegResampler, "Create audio resampler Channels:%u->%u, BitsPerSample:%u->%u, SampleRate:%u->%u, Ptime:%u", 
			nInChannels, nOutChannels,
			nInBitsPerSample, nOutBitsPerSample,
			nInSampleRate, nOutSampleRate,
			nPtime
		);

	if((m_pSwr = swr_alloc()))
	{
#if 0 /* Division by zero */
		m_pSwr = swr_alloc_set_opts(
			m_pSwr, 
			av_get_default_channel_layout(m_nOutChannels), OTResamplerAudioFFmpeg::_getSampleFormat(m_nOutBytesPerSample), nOutSampleRate,
			av_get_default_channel_layout(m_nInChannels), OTResamplerAudioFFmpeg::_getSampleFormat(m_nInBytesPerSample), nInSampleRate,
			0, 0);
		if(!m_pSwr)
		{
			OT_DEBUG_ERROR_EX(kOTMobuleNameFFmpegResampler, "swr_alloc_set_opts() failed");
		}

#else
		int ret = av_opt_set_int(m_pSwr, "in_channel_layout",  av_get_default_channel_layout(m_nInChannels), 0);
		ret = av_opt_set_int(m_pSwr, "out_channel_layout", av_get_default_channel_layout(m_nOutChannels),  0);
		ret = av_opt_set_int(m_pSwr, "in_sample_rate",     nInSampleRate, 0);
		ret = av_opt_set_int(m_pSwr, "out_sample_rate",    nOutSampleRate, 0);
		ret = av_opt_set_sample_fmt(m_pSwr, "in_sample_fmt",  OTResamplerAudioFFmpeg::_getSampleFormat(m_nInBytesPerSample), 0);
		ret = av_opt_set_sample_fmt(m_pSwr, "out_sample_fmt", OTResamplerAudioFFmpeg::_getSampleFormat(m_nOutBytesPerSample),  0);
		ret = swr_init(m_pSwr);
		if(ret < 0)
		{
			OT_DEBUG_ERROR_EX(kOTMobuleNameFFmpegResampler, "swr_init() failed: %s", ot_av_err2str(ret));
			_deInit();
		}
#endif
	}
	else
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameFFmpegResampler, "Failed to allocate SwrContext");
	}
}

OTResamplerAudioFFmpeg::~OTResamplerAudioFFmpeg()
{
	_deInit();
	OT_DEBUG_INFO("*** OTResamplerAudioFFmpeg destroyed ***");
}

void OTResamplerAudioFFmpeg::_deInit()
{
	if(m_pSwr)
	{
		swr_free(&m_pSwr);
		m_pSwr = NULL;
	}
}



// @Override(OTResamplerAudio)
bool OTResamplerAudioFFmpeg::isValid()
{ 
	return (m_pSwr != NULL);
}

// @Override(OTResamplerAudio)
size_t OTResamplerAudioFFmpeg::resample(const void* pInBufferPtr, size_t nInBufferSizeInSamples, void* pOutBufferPtr, size_t nOutBufferSizeInSamples)
{
	if(!isValid())
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameFFmpegResampler, "Not valid");
		return 0;
	}

	if(!pInBufferPtr || !pOutBufferPtr)
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameFFmpegResampler, "Invalid parameter");
		return 0;
	}

	if(m_nExpectedInBufferSizeInSamples != nInBufferSizeInSamples)
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameFFmpegResampler, "Invalid input size: %u<>%u", m_nExpectedInBufferSizeInSamples, nInBufferSizeInSamples);
		return 0;
	}

	if(m_nExpectedOutBufferSizeInSamples != nOutBufferSizeInSamples)
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameFFmpegResampler, "Invalid input size: %u<>%u", m_nExpectedOutBufferSizeInSamples, nOutBufferSizeInSamples);
		return 0;
	}

	int ret = swr_convert(m_pSwr, (uint8_t **)&pOutBufferPtr, nOutBufferSizeInSamples, (const uint8_t **)&pInBufferPtr, nInBufferSizeInSamples);
	if(ret < 0)
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameFFmpegResampler, "swr_convert(audio resampler) failed: %s", ot_av_err2str(ret));
		return 0;
	}
	if(m_nExpectedOutBufferSizeInSamples != ret)
	{
		OT_DEBUG_INFO_EX(kOTMobuleNameFFmpegResampler, "Output size not expected %u<>%d...flushing :(", m_nExpectedOutBufferSizeInSamples, ret);
		swr_convert(m_pSwr, (uint8_t **)&pOutBufferPtr, nOutBufferSizeInSamples, NULL, 0); // flush()
		memset(pOutBufferPtr, 0, (m_nExpectedOutBufferSizeInSamples * m_nOutBytesPerSample)); // silence()
		return m_nExpectedOutBufferSizeInSamples;
	}

	return m_nExpectedOutBufferSizeInSamples;
}


enum AVSampleFormat OTResamplerAudioFFmpeg::_getSampleFormat(size_t nBytesPerSample, bool bPlanar /*= false*/)
{
	switch(nBytesPerSample)
	{
		case 1: return bPlanar ? AV_SAMPLE_FMT_U8P : AV_SAMPLE_FMT_U8;
		case 2: return bPlanar ? AV_SAMPLE_FMT_S16P : AV_SAMPLE_FMT_S16;
		case 4: return bPlanar ? AV_SAMPLE_FMT_FLTP : AV_SAMPLE_FMT_FLT;
		default: OT_ASSERT(false); return AV_SAMPLE_FMT_NONE;
	}
}

