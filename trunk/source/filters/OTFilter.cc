/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#include "opentelepresence/filters/OTFilter.h"

extern "C"
{
#include <libswscale/swscale.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/avfiltergraph.h>
#include <libavfilter/avcodec.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
}

#include <assert.h>
#include <stdio.h> /* snprintf */

#if defined(_MSC_VER)
#	define snprintf		_snprintf
#endif

static const char* ot_av_err2str(int errnum)
{
	static char __errbuf[AV_ERROR_MAX_STRING_SIZE] = {0};
    return av_make_error_string(__errbuf, AV_ERROR_MAX_STRING_SIZE, errnum);
}

class OTFilterVideoFFmeg : public OTFilterVideo
{
public:
	OTFilterVideoFFmeg(std::string strDescription, size_t nInWidth, size_t nInHeight, size_t nOutWidth, size_t nOutHeight, size_t nFps);
	virtual ~OTFilterVideoFFmeg();
	virtual OT_INLINE const char* getObjectId() { return "OTFilterVideoFFmeg"; }
	
	// @Override (OTFilter)
	bool isValid() { return m_bInitialized; }

	// @Override (OTFilterVideo)
	bool filterFrame(
			const void* pcInBufferPtr, size_t nInBufferSize, size_t nInWidth, size_t nInHeight, size_t nOutWidth, size_t nOutHeight,
			void* pOutFrame
		);

	// private functions
	bool _init(std::string strDescription, size_t nInWidth, size_t nInHeight, size_t nOutWidth, size_t nOutHeight, size_t nFps);
	bool _deInit();

private:
	bool m_bInitialized;
	size_t m_nInWidth, m_nInHeight, m_nOutWidth, m_nOutHeight, m_nFps;
	AVFilterInOut *m_pOutputs, *m_pInputs;
	AVFilterContext *m_pBufferSinkCtx, *m_pBufferSrcCtx;
	AVFilterGraph *m_pFilterGraph;
	AVFrame *m_pFrameIn;
};

//
//	OTFilter
//

OTFilter::OTFilter(OTMediaType_t eMediaType, std::string strDescription)
: m_eMediaType(eMediaType)
, m_strDescription(strDescription)
{

}

OTFilter::~OTFilter()
{

}


//
//	OTFilterVideo
//


OTFilterVideo::OTFilterVideo(std::string strDescription, size_t nInWidth, size_t nInHeight, size_t nOutWidth, size_t nOutHeight, size_t nFps)
: OTFilter(OTMediaType_Video, strDescription)
{
}

OTFilterVideo::~OTFilterVideo()
{
}

OTObjectWrapper<OTFilterVideo*> OTFilterVideo::New(
		std::string strDescription, 
		size_t nInWidth, size_t nInHeight, 
		size_t nOutWidth, size_t nOutHeight, 
		size_t nFps)
{
	OTObjectWrapper<OTFilterVideo*> oFilter = new OTFilterVideoFFmeg(strDescription, nInWidth, nInHeight, nOutWidth, nOutHeight, nFps);
	if(oFilter && oFilter->isValid())
	{
		return oFilter;
	}
	return NULL;
}


//
//	OTFilterVideoFFmeg
//

OTFilterVideoFFmeg::OTFilterVideoFFmeg(std::string strDescription, size_t nInWidth, size_t nInHeight, size_t nOutWidth, size_t nOutHeight, size_t nFps)
: OTFilterVideo(strDescription, nInWidth, nInHeight, nOutWidth, nOutHeight, nFps)
, m_bInitialized(false)
, m_pOutputs(NULL), m_pInputs(NULL)
, m_pBufferSinkCtx(NULL), m_pBufferSrcCtx(NULL)
, m_pFilterGraph(NULL)
, m_pFrameIn(NULL)
, m_nInWidth(0), m_nInHeight(0), m_nOutWidth(0), m_nOutHeight(0), m_nFps(nFps)
{
	_init(strDescription, nInWidth, nInHeight, nOutWidth, nOutHeight, nFps);
}

OTFilterVideoFFmeg::~OTFilterVideoFFmeg()
{
	_deInit();

	OT_DEBUG_INFO("*** OTFilterVideoFFmeg destroyed ***");
}
	
// @Override (OTFilterVideo)
bool OTFilterVideoFFmeg::filterFrame(
			const void* pcInBufferPtr, size_t nInBufferSize, size_t nInWidth, size_t nInHeight, size_t nOutWidth, size_t nOutHeight,
			void* pOutFrame
		)
{
	AVFilterBufferRef *pPicref = NULL;
	int ret;

	// check that all parameters are valid (buffer must contain YUV420 image)
	if(!pcInBufferPtr || !pOutFrame || ((nInWidth * nInHeight * 3) >>1) > nInBufferSize)
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameFFmpegFilter, "Invalid parameter");
		return false;
	}
	// check whether the size changed
	if(m_nInWidth != nInWidth || m_nInHeight != nInHeight || m_nOutWidth != nOutWidth || m_nOutHeight != nOutHeight)
	{
		OT_DEBUG_INFO_EX(kOTMobuleNameFFmpegFilter, "Filter image size changed (%u->%u and %u->%u) or (%u->%u and %u->%u)", m_nInWidth, nInWidth, m_nInHeight, nInHeight, m_nOutWidth, nOutWidth, m_nOutHeight, nOutHeight);
		if(!(_init(m_strDescription, nInWidth, nInHeight, nOutWidth, nOutHeight, m_nFps)))
		{
			return false;
		}
	}

	if(!m_bInitialized)
	{
		OT_DEBUG_INFO_EX(kOTMobuleNameFFmpegFilter, "Filter not initialized");
		if(!(_init(m_strDescription, nInWidth, nInHeight, nOutWidth, nOutHeight, m_nFps)))
		{
			return false;
		}
	}

	/* wrap buffers */
	avpicture_fill((AVPicture *)m_pFrameIn, (const uint8_t*)pcInBufferPtr, PIX_FMT_YUV420P, nInWidth, nInHeight);
	// because of scaling filter(optional), we've to restore the size
	m_pFrameIn->width = nInWidth;
	m_pFrameIn->height = nInHeight;

	/* push the decoded frame into the filtergraph */
    if((ret = av_buffersrc_add_frame(m_pBufferSrcCtx, m_pFrameIn, 0)) < 0) 
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameFFmpegFilter, "Error while feeding the filtergraph: %s", ot_av_err2str(ret));
        goto end;
    }

    /* pull filtered pictures from the filtergraph */
    while(1)
	{
        ret = av_buffersink_get_buffer_ref(m_pBufferSinkCtx, &pPicref, 0);
        if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
		{
			ret = 0;
            break;
		}
        if(ret < 0)
		{
			OT_DEBUG_ERROR_EX(kOTMobuleNameFFmpegFilter, "Error while getting the picref: %s", ot_av_err2str(ret));
            goto end;
		}

        if(pPicref)
		{
			/* apply filter */
			ret = avfilter_copy_buf_props(m_pFrameIn, pPicref); // FIXME: use out here? instead of layouting?
			avfilter_unref_bufferp(&pPicref);	
			av_picture_copy((AVPicture*)pOutFrame, (const AVPicture*)m_pFrameIn, (enum AVPixelFormat)m_pFrameIn->format, m_pFrameIn->width, m_pFrameIn->height);
        }
    }

end:
	avfilter_unref_bufferp(&pPicref);
	return (ret == 0);
}

bool OTFilterVideoFFmeg::_init(std::string strDescription, size_t nInWidth, size_t nInHeight, size_t nOutWidth, size_t nOutHeight, size_t nFps)
{
	static enum AVPixelFormat __pix_fmts[] = { PIX_FMT_YUV420P, AV_PIX_FMT_NONE };
	AVBufferSinkParams *pBufferSinkParams;

	_deInit();

	// at least one field must not be empty
	OT_ASSERT(!strDescription.empty() && nInWidth > 0 && nInHeight > 0 && nOutWidth > 0 && nOutHeight > 0);
	
	char args[512];
	int ret;
	const AVFilter *pcBufferSrc  = avfilter_get_by_name("buffer");
	const AVFilter *pcBufferSink = avfilter_get_by_name("ffbuffersink");
	m_pOutputs = avfilter_inout_alloc();
	m_pInputs  = avfilter_inout_alloc();
	if(!m_pOutputs || !m_pInputs)
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameFFmpegFilter, "Allocation failed");
		goto end;
	}

	m_pFilterGraph = avfilter_graph_alloc();
	if(!m_pFilterGraph)
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameFFmpegFilter, "Allocation failed");
		goto end;
	}

	/* buffer video source: the decoded frames from the decoder will be inserted here. */
	snprintf(args, sizeof(args),
			"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
			nInWidth, nInHeight, AV_PIX_FMT_YUV420P,
			1, nFps,
			nInWidth, nInHeight);

	ret = avfilter_graph_create_filter(&m_pBufferSrcCtx, (AVFilter *)pcBufferSrc, "in", args, NULL, m_pFilterGraph);
	if(ret < 0 || !m_pBufferSrcCtx)
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameFFmpegFilter, "Cannot create buffer source: %s", ot_av_err2str(ret));
		goto end;
	}

	/* buffer video sink: to terminate the filter chain. */
	pBufferSinkParams = av_buffersink_params_alloc();
	if(!pBufferSinkParams)
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameFFmpegFilter, "Allocation failed");
		goto end;
	}
	pBufferSinkParams->pixel_fmts = __pix_fmts;
	ret = avfilter_graph_create_filter(&m_pBufferSinkCtx, (AVFilter *)pcBufferSink, "out", NULL, pBufferSinkParams, m_pFilterGraph);
	av_free(pBufferSinkParams);
	if(ret < 0 || !m_pBufferSinkCtx)
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameFFmpegFilter, "Cannot create buffer sink: %s", ot_av_err2str(ret));
		goto end;
	}

	/* Endpoints for the filter graph. */
	m_pOutputs->name       = av_strdup("in");
	m_pOutputs->filter_ctx = m_pBufferSrcCtx;
	m_pOutputs->pad_idx    = 0;
	m_pOutputs->next       = NULL;

	m_pInputs->name       = av_strdup("out");
	m_pInputs->filter_ctx = m_pBufferSinkCtx;
	m_pInputs->pad_idx    = 0;
	m_pInputs->next       = NULL;

	if((ret = avfilter_graph_parse(m_pFilterGraph, strDescription.c_str(), &m_pInputs, &m_pOutputs, NULL)) < 0)
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameFFmpegFilter, "Failed to parse graph[%s]: %s", strDescription.c_str(), ot_av_err2str(ret));
		goto end;
	}

	if((ret = avfilter_graph_config(m_pFilterGraph, NULL)) < 0)
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameFFmpegFilter, "Failed to config graph: %s", ot_av_err2str(ret));
		goto end;
	}

	m_pFrameIn = avcodec_alloc_frame();
	if(!m_pFrameIn)
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameFFmpegFilter, "Failed to allocate video frame");
		goto end;
	}	
	
	m_pFrameIn->pts = 0;
	m_pFrameIn->width = nInWidth;
	m_pFrameIn->height = nInHeight;
	m_pFrameIn->format = PIX_FMT_YUV420P;

	m_nInWidth = nInWidth;
	m_nInHeight = nInHeight;
	m_nOutWidth = nOutWidth;
	m_nOutHeight = nOutHeight;
	m_nFps = nFps;

	m_bInitialized = true;
	
end:
	if(!m_bInitialized)
	{
		_deInit();
	}
	
	return m_bInitialized;
}

bool OTFilterVideoFFmeg::_deInit()
{
	avfilter_inout_free(&m_pOutputs);
	avfilter_inout_free(&m_pInputs);
	
	if(m_pFilterGraph)
	{
		avfilter_graph_free(&m_pFilterGraph); // will delete contexts
		m_pBufferSinkCtx = NULL;
		m_pBufferSrcCtx = NULL;
	}

	if(m_pBufferSinkCtx)
	{
		avfilter_free(m_pBufferSinkCtx);
		m_pBufferSinkCtx = NULL;
	}
	if(m_pBufferSrcCtx)
	{
		avfilter_free(m_pBufferSrcCtx);
		m_pBufferSrcCtx = NULL;
	}

	if(m_pFrameIn)
	{
		av_free(m_pFrameIn);
		m_pFrameIn = NULL;
	}

	m_bInitialized = false;

	return true;
}