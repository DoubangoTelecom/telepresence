/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#include "opentelepresence/OTCodec.h"

#include "SipStack.h"

#include "tsk_string.h"
#include "tsk_debug.h"

#include <assert.h>

//
//	OTCodecEncodingResult
//
OTCodecEncodingResult::OTCodecEncodingResult(const void* pDataPtr, size_t nDataSize, uint32_t nDuration, bool bLastResultInFrame)
: m_pDataPtr(NULL)
, m_nDataSize(0)
, m_nDataEncodedSize(0)
, m_nDuration(0)
{
	OT_ASSERT(update(pDataPtr, nDataSize, nDuration, bLastResultInFrame));
}

OTCodecEncodingResult::~OTCodecEncodingResult()
{
	OT_DEBUG_INFO("*** OTCodecEncodingResult destroyed ***");
	TSK_FREE(m_pDataPtr);
}

bool OTCodecEncodingResult::update(const void* pDataPtr, size_t nDataSize, uint32_t nDuration, bool bLastResultInFrame)
{
	if(m_nDataSize < nDataSize)
	{
		if(!(m_pDataPtr = tsk_realloc(m_pDataPtr, nDataSize)))
		{
			m_nDataSize = m_nDataEncodedSize = 0;
			return false;
		}
		m_nDataSize = nDataSize;
	}
	if((m_nDataEncodedSize = nDataSize) > 0)
	{
		memcpy(m_pDataPtr, pDataPtr, nDataSize);
	}
	m_nDuration = nDuration;
	m_bLastResultInFrame = bLastResultInFrame;
	return true;
}


//
//	Codec base object
//

OTCodec::OTCodec(OTCodec_Type_t eType)
: OTObject()
{
	m_eType = eType;
	m_bOpened = false;

	typedef struct codec_format_s { const char* pcFormat;  OTCodec_Type_t eType; } codec_format_t;
	static const codec_format_t __codec_formats[] = 
	{  
		{ TMEDIA_CODEC_FORMAT_VP8, OTCodecType_VP8 },
#if HAVE_HARTALLO_H
		{ TMEDIA_CODEC_FORMAT_H264SVC, OTCodecType_H264SVC },
#endif
		{ TMEDIA_CODEC_FORMAT_H264_BP, OTCodecType_H264Base },
		{ TMEDIA_CODEC_FORMAT_H264_MP, OTCodecType_H264Main },

		{ TMEDIA_CODEC_FORMAT_H263, OTCodecType_H263 },
		{ TMEDIA_CODEC_FORMAT_H263_1998, OTCodecType_H263P },
		{ TMEDIA_CODEC_FORMAT_H263_2000, OTCodecType_H263PP },
	}; 
	static const size_t __codec_formats_count = (sizeof(__codec_formats)/sizeof(__codec_formats[0]));
	size_t i;

	for(i = 0; i < __codec_formats_count; ++i)
	{
		if(__codec_formats[i].eType == eType)
		{
			m_pWrappedCodec = tmedia_codec_create(__codec_formats[i].pcFormat);
			break;
		}
	}

	OT_ASSERT(m_pWrappedCodec);
	OT_DEBUG_INFO("Create new codec with type = %d", eType);
}

bool OTCodec::open()
{
	if(!isValid())
	{
		OT_DEBUG_ERROR("Codec not valid");
		return false;
	}
	return (tmedia_codec_open(m_pWrappedCodec) == 0);
}

bool OTCodec::close()
{
	if(!isValid())
	{
		OT_DEBUG_ERROR("Codec not valid");
		return false;
	}
	return (tmedia_codec_close(m_pWrappedCodec) == 0);
}

OTCodec::~OTCodec()
{
	OT_DEBUG_INFO("*** OTCodec destroyed ***");
	
	// close() the wrapped codec
	close();
	// destroy the wrapped codec
	TSK_OBJECT_SAFE_FREE(m_pWrappedCodec);
}



//
//	OTCodecVideo
//

OTCodecVideo::OTCodecVideo(OTCodec_Type_t eType, size_t nWidth, size_t nHeight)
: OTCodec(eType)
, m_nResultsCount(0)
{
	TMEDIA_CODEC_VIDEO(m_pWrappedCodec)->in.width = nWidth;
	TMEDIA_CODEC_VIDEO(m_pWrappedCodec)->in.height = nHeight;
	TMEDIA_CODEC_VIDEO(m_pWrappedCodec)->out.width = nWidth;
	TMEDIA_CODEC_VIDEO(m_pWrappedCodec)->out.height = nHeight;

	// set callback for RTP chuncks
	OT_ASSERT((tmedia_codec_video_set_enc_callback(TMEDIA_CODEC_VIDEO(m_pWrappedCodec), OTCodecVideo::encoderResultCb, this) == 0));
}

OTCodecVideo::~OTCodecVideo()
{
	OT_DEBUG_INFO("*** OTCodecVideo destroyed ***");
}

bool OTCodecVideo::encode(const void* pcInData, size_t nInSize, void** ppOutData /*= NULL*/, size_t* pOutMaxSize /*= NULL*/)
{
	return (m_pWrappedCodec->plugin->encode(m_pWrappedCodec, pcInData, nInSize, ppOutData, (tsk_size_t*)pOutMaxSize) == 0);
}

bool OTCodecVideo::executeAction(OTCodecAction_t eAction)
{
	switch(eAction)
	{
		case OTCodecAction_FlushEncodedData:
			{
				m_nResultsCount = 0;
				return true;
			}

		case OTCodecAction_ForceIntra:
			{
				static tmedia_param_t* __paramForceIntra = NULL;
				static const tmedia_codec_action_t __actionForceIntra = tmedia_codec_action_encode_idr;
				if(!__paramForceIntra)
				{
					__paramForceIntra = tmedia_param_create(tmedia_pat_set,
													tmedia_video,
													tmedia_ppt_codec,
													tmedia_pvt_int32,
													"action",
													(void*)&__actionForceIntra);
				}
				return (tmedia_codec_set(m_pWrappedCodec, __paramForceIntra) == 0);
			}
	}
	return false;
}

int OTCodecVideo::encoderResultCb(const tmedia_video_encode_result_xt* pcResult)
{
	OTCodecVideo* pcVideoCodec = (OTCodecVideo*)pcResult->usr_data;
	if(pcVideoCodec->m_nResultsCount >= OT_VIDEO_MAX_CHUNKS_PER_FRAME)
	{
		OT_DEBUG_ERROR("(%u >= %u): ignoring encoded chunk");
		return -1;
	}

	if(!pcVideoCodec->m_aResults[pcVideoCodec->m_nResultsCount])
	{
		if((pcVideoCodec->m_aResults[pcVideoCodec->m_nResultsCount] = new OTCodecEncodingResult(pcResult->buffer.ptr, pcResult->buffer.size, pcResult->duration, (pcResult->last_chunck == tsk_true))))
		{
			pcVideoCodec->m_nResultsCount++;
			return 0;
		}
	}
	else
	{
		if((pcVideoCodec->m_aResults[pcVideoCodec->m_nResultsCount]->update(pcResult->buffer.ptr, pcResult->buffer.size, pcResult->duration, (pcResult->last_chunck == tsk_true))))
		{
			pcVideoCodec->m_nResultsCount++;
			return 0;
		}
	}

	return -1;
}

OTObjectWrapper<OTCodecVideo*> OTCodecVideo::createBestCodec(OTCodec_Type_t eListOfCodecsToSearchInto, size_t nWidth, size_t nHeight)
{
	static const OTCodec_Type_t __bestVideoCodecByPriority[] = 
	{
		OTCodecType_VP8,
		OTCodecType_H264Base,
		OTCodecType_H264Main,
#if HAVE_HARTALLO_H
		OTCodecType_H264SVC,
#endif
		OTCodecType_H263P,
		OTCodecType_H263,
		OTCodecType_H263PP
	};
	static const size_t __bestVideoCodecByPriorityCount = sizeof(__bestVideoCodecByPriority)/sizeof(__bestVideoCodecByPriority[0]);

	size_t i;
	OTObjectWrapper<OTCodecVideo*> oCodecVideo;
	for(i = 0; i <__bestVideoCodecByPriorityCount; ++i)
	{
		if((__bestVideoCodecByPriority[i] & eListOfCodecsToSearchInto) == __bestVideoCodecByPriority[i])
		{
			if(!SipStack::isCodecSupported((tdav_codec_id_t)__bestVideoCodecByPriority[i]))
			{
				OT_DEBUG_INFO("Codec with id = %u has high priority in selection but it's not supported", __bestVideoCodecByPriority[i]);
				continue;
			}
			return new OTCodecVideo(__bestVideoCodecByPriority[i], nWidth, nHeight);
		}
	}
	return NULL;
}

OTObjectWrapper<OTCodec*> OTCodecVideo::OTNew(OTCodec_Type_t eType, size_t nWidth, size_t nHeight)
{
	OTObjectWrapper<OTCodec*> oCodec = new OTCodecVideo(eType, nWidth, nHeight);

	if(oCodec)
	{
		OT_ASSERT(oCodec->getMediaType() == OTMediaType_Video);
		if(!oCodec->isValid())
		{
			oCodec = NULL;
		}
	}
	return oCodec;
}
