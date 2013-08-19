/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#include "opentelepresence/patterns/OTPatternVideoHangout.h"
#include "opentelepresence/docstreamer/OTDocStreamer.h"
#include "opentelepresence/OTWrap.h"
#include "opentelepresence/OTEngine.h"

#include "tsk_memory.h"
#include "tsk_debug.h"

#include <assert.h>

extern "C"
{
	#include <libavcodec/avcodec.h>
	#include <libswscale/swscale.h>
}

static const size_t kLitenerBorder = 0;
static const size_t kMinActiveParticipants4SizeCompute = 4;
static const size_t kListenerHeightLog2 = 2;

/*
int Y = (int)(R *  .299000 + G *  .587000 + B *  0.114000);
int U = (int)(R * -.168736 + G * -.331264 + B *  0.500000 + 128);
int V = (int)(R *  .500000 + G * -.418688 + B * -0.081312 + 128);
*/
#define Y_BLUE	29
#define U_BLUE	255
#define V_BLUE	107
#define Y_WHITE	255
#define U_WHITE	128
#define V_WHITE	128
#define Y_BLACK	0
#define U_BLACK	128
#define V_BLACK	128
#define Y_RED	76
#define U_RED	84
#define V_RED	255
#define Y_GREEN	149
#define U_GREEN	43
#define V_GREEN	21



OTPatternVideoHangout::OTPatternVideoHangout(OTObjectWrapper<OTBridgeInfo*> oBridgeInfo)
: OTPatternVideo(OTPatternType_Hangout, oBridgeInfo)
, m_pFrameMix(NULL)
{
	OT_ASSERT(oBridgeInfo);
	
	m_oBridgeInfo->getVideoSpeakerPAR(m_parSpeaker.nNumerator, m_parSpeaker.nDenominator);
	m_oBridgeInfo->getVideoListenerPAR(m_parListener.nNumerator, m_parListener.nDenominator);
}

OTPatternVideoHangout::~OTPatternVideoHangout()
{
	if(m_pFrameMix)
	{
		av_free(m_pFrameMix);
	}

	OT_DEBUG_INFO("*** OTPatternVideoHangout destroyed ***");
}

#if OPENTELEPRESENCE_UNDER_WINDOWS
#	define OTMulDiv MulDiv
#else
static inline int OTMulDiv(int nNumber, int nNumerator, int nDenominator)
{
    long long llRet = (nNumber);
	llRet *= (nNumerator);
	llRet /= (nDenominator);
	return (int)llRet;
}
#endif

static inline void _correctAspectRatio(
	const OTRect_t& rcSrc, 
	const OTRatio_t& parSrc, 
	OTRect_t& rcDst
)
{
    if ((parSrc.nNumerator != 1) || (parSrc.nDenominator != 1))
    {
        if (parSrc.nNumerator > parSrc.nDenominator)
        {
            rcDst.nRight = OTMulDiv(rcDst.nRight, parSrc.nNumerator, parSrc.nDenominator);
        }
        else if (parSrc.nNumerator < parSrc.nDenominator)
        {
            rcDst.nBottom = OTMulDiv(rcDst.nBottom, parSrc.nDenominator, parSrc.nNumerator);
        }
    }
}

static inline void _letterBoxRect(
	const OTRect_t& rcSrc, 
	const OTRect_t& rcDst, 
	OTRect_t &rcResult
)
{
    // figure out src/dest scale ratios
    int iSrcWidth  = OTRectWidth(rcSrc);
    int iSrcHeight = OTRectHeight(rcSrc);

    int iDstWidth  = OTRectWidth(rcDst);
    int iDstHeight = OTRectHeight(rcDst);

    int iDstLBWidth;
    int iDstLBHeight;

    if(OTMulDiv(iSrcWidth, iDstHeight, iSrcHeight) <= iDstWidth)
	{
        iDstLBWidth  = OTMulDiv(iDstHeight, iSrcWidth, iSrcHeight);
        iDstLBHeight = iDstHeight;
    }
    else
	{
        iDstLBWidth  = iDstWidth;
        iDstLBHeight = OTMulDiv(iDstWidth, iSrcHeight, iSrcWidth);
    }    

    rcResult.nLeft = rcDst.nLeft + ((iDstWidth - iDstLBWidth) >> 1);
    rcResult.nTop = rcDst.nTop + ((iDstHeight - iDstLBHeight) >> 1);
	rcResult.nRight = rcResult.nLeft + iDstLBWidth;
	rcResult.nBottom = rcResult.nTop + iDstLBHeight;
}

// we assume "rectDst" is already filled with zeros
static inline void _findListenerDest(
	size_t nConsumers, size_t nListenerIndex, 
	size_t nMixFrameWidth, size_t nMixFrameHeight, 
	size_t nListenerFrameWidth, size_t nListenerFrameHeight, 
	OTRect_t &rectDst,
	const OTRatio_t &PAR
	)
{
	const size_t nActiveParticipants = TSK_MAX(nConsumers, kMinActiveParticipants4SizeCompute);
	const size_t nListenerWidth = (nMixFrameWidth / nActiveParticipants);
	const size_t nListenersWidth = (nConsumers * nListenerWidth);

	// init()
	rectDst.nLeft = rectDst.nTop = kLitenerBorder;
	rectDst.nBottom = (nMixFrameHeight >> kListenerHeightLog2) - kLitenerBorder;
	// center()
	rectDst.nLeft += (((nMixFrameWidth >> 1) - (nListenersWidth >> 1)) + (nListenerIndex * nListenerWidth));
	// setRight()
	rectDst.nRight = rectDst.nLeft + nListenerWidth;

	// ratio()
	OTRect_t rcSrc = {0, 0, nListenerFrameWidth, nListenerFrameHeight};
	if(PAR.nDenominator != 0 && PAR.nDenominator != 0)
	{
		_correctAspectRatio(rcSrc, PAR, rcSrc);
		_letterBoxRect(rcSrc, rectDst, rectDst);
	}
}

static void _mixSpeaker(
	OTObjectWrapper<OTProxyPluginConsumerVideo*>oConsumer, 
	struct AVFrame *pMixFrame, 
	size_t nConsumers,
	const OTRatio_t &PAR,
	OTDocStreamer* pDocStreamer = NULL
	)
{
	OTObjectWrapper<OTFrameVideo*> oSpeakerFrame = oConsumer->getHeldFrameVideo();
	if(!oSpeakerFrame && !pDocStreamer)
	{
		// must never be called as test already done in Mix()
		OT_DEBUG_ERROR("No video frame held");
		return;
	}
	
	size_t nFrameWidth = 0;
	size_t nFrameHeight = 0;
	const void* pFramePtr = NULL;
	if(pDocStreamer)
	{
		nFrameWidth = pDocStreamer->getWidth();
		nFrameHeight = pDocStreamer->getHeight();
		pDocStreamer->getCurrentFrame(NULL, &pFramePtr, &nFrameWidth, &nFrameHeight);
	}
	if(!pFramePtr)
	{
		nFrameWidth = oSpeakerFrame->getWidth();
		nFrameHeight = oSpeakerFrame->getHeight();
		pFramePtr = oSpeakerFrame->getBufferPtr();
	}
	const size_t nFrameSize = (nFrameWidth * nFrameHeight * 3) >> 1;

#if 1 /* resize */
	const size_t nListenerHeight = (pMixFrame->height >> kListenerHeightLog2);
	const size_t nSpeakerWidth = pMixFrame->width;
	const size_t nSpeakerHeight = (pMixFrame->height - nListenerHeight);

	OTRect_t rcDst = {0, 0, nSpeakerWidth, nSpeakerHeight};
	OTRect_t rcSrc = {0, 0, nFrameWidth, nFrameHeight};
	if(PAR.nDenominator != 0 && PAR.nDenominator != 0)
	{
		_correctAspectRatio(rcSrc, PAR, rcSrc);
		_letterBoxRect(rcSrc, rcDst, rcDst);
	}

	const size_t nRectWidth = OTRectWidth(rcDst); // save()
	const size_t nRectHeight = OTRectHeight(rcDst); // save()

	// turn even
	rcDst.nTop &= ~1;
	rcDst.nBottom &= ~1;
	rcDst.nLeft &= ~1;
	rcDst.nRight &= ~1;

	// Convert to samples
	rcDst.nTop *= pMixFrame->width;
	rcDst.nBottom *= pMixFrame->width;

	// save addresses
	uint8_t *ppFrameMixData[3] =
	{
		pMixFrame->data[0],
		pMixFrame->data[1],
		pMixFrame->data[2]
	};

	pMixFrame->data[0] = ppFrameMixData[0] + rcDst.nTop + rcDst.nLeft;
	pMixFrame->data[1] = ppFrameMixData[1] + (rcDst.nTop >> 2) + (rcDst.nLeft >> 1);
	pMixFrame->data[2] = ppFrameMixData[2] + (rcDst.nTop >> 2) + (rcDst.nLeft >> 1);

	oConsumer->drawOverlay(false, 
		pFramePtr, nFrameSize, nFrameWidth, nFrameHeight,
		nRectWidth, nRectHeight, (void*)pMixFrame
		);

	// restore addresses
	pMixFrame->data[0] = ppFrameMixData[0];
	pMixFrame->data[1] = ppFrameMixData[1];
	pMixFrame->data[2] = ppFrameMixData[2];
#else
	const size_t nListenerHeight = (pMixFrame->height >> kListenerHeightLog2);
	const size_t nSpeakerWidth = pMixFrame->width;
	const size_t nSpeakerHeight = (pMixFrame->height - nListenerHeight);

	// draw() overlay
	oConsumer->drawOverlay(false, 
		pFramePtr, nFrameSize, nFrameWidth, nFrameHeight,
		nSpeakerWidth, nSpeakerHeight, (void*)pMixFrame
		);
#endif
}

static void _mixListener(
	OTObjectWrapper<OTProxyPluginConsumerVideo*>oConsumer, 
	struct AVFrame *pMixFrame, 
	size_t nConsumers, size_t nListenerIndex, 
	bool bSpeaker, bool bSpeaking,
	const OTRatio_t &PAR,
	OTDocStreamer* pDocStreamer = NULL
	)
{
	OTObjectWrapper<OTFrameVideo*> oListenerFrame = oConsumer->getHeldFrameVideo();
	if(!oListenerFrame)
	{
		// must never be called as test already done in Mix()
		OT_DEBUG_ERROR("No video frame held");
		return;
	}

	OTRect_t rectDst = {0};

	_findListenerDest(
		nConsumers, nListenerIndex, 
		pMixFrame->width, pMixFrame->height, 
		oListenerFrame->getWidth(), oListenerFrame->getHeight(), 
		rectDst,
		PAR
	);

	const size_t nListenerHeight = (pMixFrame->height >> kListenerHeightLog2);
	const size_t nSpeakerHeight = (pMixFrame->height - nListenerHeight);
	const size_t nRectWidth = OTRectWidth(rectDst); // save()
	const size_t nRectHeight = OTRectHeight(rectDst); // save()
	

	// save addresses
	uint8_t *ppFrameMixData[3] =
	{
		pMixFrame->data[0],
		pMixFrame->data[1],
		pMixFrame->data[2]
	};
	
	// turn even
	rectDst.nTop &= ~1;
	rectDst.nBottom &= ~1;
	rectDst.nLeft &= ~1;
	rectDst.nRight &= ~1;

	// Convert to samples
	rectDst.nTop += nSpeakerHeight;
	rectDst.nBottom += nSpeakerHeight;
	rectDst.nTop *= pMixFrame->width;
	rectDst.nBottom *= pMixFrame->width;

	pMixFrame->data[0] = ppFrameMixData[0] + rectDst.nTop + rectDst.nLeft;
	pMixFrame->data[1] = ppFrameMixData[1] + (rectDst.nTop >> 2) + (rectDst.nLeft >> 1);
	pMixFrame->data[2] = ppFrameMixData[2] + (rectDst.nTop >> 2) + (rectDst.nLeft >> 1);

	oConsumer->drawOverlay(true, 
		oListenerFrame->getBufferPtr(), oListenerFrame->getBufferSize(), oListenerFrame->getWidth(), oListenerFrame->getHeight(),
		nRectWidth, nRectHeight, (void*)pMixFrame
		);

	// draw border
	if(bSpeaker || bSpeaking)
	{
		const uint8_t Y = bSpeaker ? Y_BLUE : Y_GREEN;
		const uint8_t U = bSpeaker ? U_BLUE : U_GREEN;
		const uint8_t V = bSpeaker ? V_BLUE : V_GREEN;

		const int pnFrameMixLines[3] = { nRectWidth, nRectWidth >> 1, nRectWidth >> 1 };

		// top border
		memset(pMixFrame->data[0], Y, pnFrameMixLines[0]);
		memset(pMixFrame->data[1], U, pnFrameMixLines[1]);
		memset(pMixFrame->data[2], V, pnFrameMixLines[2]);
#if 0
		// bottom border
		memset(pMixFrame->data[0] + ((nRectWidth - 1) * pMixFrame->linesize[0]), Y, pnFrameMixLines[0]);
		memset(pMixFrame->data[1] + (((nRectWidth - 1) >> 1) * pMixFrame->linesize[1]), U, pnFrameMixLines[1]);
		memset(pMixFrame->data[2] + (((nRectWidth - 1) >> 1) * pMixFrame->linesize[2]), V, pnFrameMixLines[2]);
#endif
	}

	// restore addresses
	pMixFrame->data[0] = ppFrameMixData[0];
	pMixFrame->data[1] = ppFrameMixData[1];
	pMixFrame->data[2] = ppFrameMixData[2];
}

OTObjectWrapper<OTFrameVideo *> OTPatternVideoHangout::mix(std::map<uint64_t, OTObjectWrapper<OTProxyPluginConsumerVideo*> >*pConsumers, void** ppDstBuffer, uint32_t *pDstBufferSize)
{
	bool bMixed = false, bSpeakerFound = false, bIsSpeaker = false;
	uint32_t nWidth = m_oBridgeInfo->getVideoWidth(), nHeight = m_oBridgeInfo->getVideoHeight();
	uint32_t nMaxBufferSize = (nWidth * nHeight * 3) >> 1;
	uint32_t nListenerIndex = 0;
	OTObjectWrapper<OTDocStreamer*> oStreamer;
	OTObjectWrapper<OTFrameVideo *> oFrameVideo;
	OTObjectWrapper<OTSipSessionAV*> oAVCall;
	OTObjectWrapper<OTSessionInfoAV*> oSessionInfo;

#if 0
	std::map<uint64_t, OTObjectWrapper<OTProxyPluginConsumerVideo*> >oConsumers = *pConsumers;
	if(pConsumers->size() > 0)
	{
		for(int i = 0; i < 4; ++i)
		{
			oConsumers[10000 + i] = pConsumers->begin()->second;
		}
		pConsumers = &oConsumers;
	}
#endif

	// adjust the buffer
	if(*pDstBufferSize < nMaxBufferSize)
	{
		if(!(*ppDstBuffer = tsk_realloc(*ppDstBuffer, (nMaxBufferSize + FF_INPUT_BUFFER_PADDING_SIZE))))
		{
			OT_DEBUG_ERROR("Failed to alloc buffer with size = %u", nMaxBufferSize);
			*pDstBufferSize = 0;
			return 0;
		}
		*pDstBufferSize = nMaxBufferSize;
	}

	std::map<uint64_t, OTObjectWrapper<OTProxyPluginConsumerVideo*> >::iterator iter;

	const size_t nConsumers = pConsumers->size();
	size_t i;

	for(iter = pConsumers->begin(), i = 0; iter != pConsumers->end(); ++iter, ++i, bIsSpeaker = false)
	{
		oFrameVideo = (*iter).second->getHeldFrameVideo();
		oSessionInfo = dynamic_cast<OTSessionInfoAV*>(*(*iter).second->getSessionInfo());
		oAVCall = NULL;
		oStreamer = NULL;
		// we must not hold a reference to OTBrige to avoid circular ref
		// comparing ids is the fastest way to check that we have an active doc streamer
		if(oSessionInfo->getDocStreamerId() != OPENTELEPRESENCE_INVALID_ID)
		{
			OTObjectWrapper<OTBridge*> oBridge = oSessionInfo->getBridgeInfo()->getBridge();
			if(oBridge)
			{
				if((oAVCall = oBridge->findCallBySessionId(oSessionInfo->getSipSessionId())))
				{
					oStreamer = oAVCall->getDocStreamer();
				}
			}
		}
		
		if(!oFrameVideo)
		{
			continue;
		}
		
		if(!bSpeakerFound && ((bIsSpeaker = (*iter).second->getSessionInfo()->isSpeaker()) || ((i + 1) == nConsumers)))
		{
			bIsSpeaker = true;
			bSpeakerFound = true;
		}
		
		if(!bMixed) // First time to mix a buffer
		{
			if(!m_pFrameMix)
			{
				OT_ASSERT(m_pFrameMix = avcodec_alloc_frame());
			}
			
			avpicture_fill((AVPicture*)m_pFrameMix, (uint8_t*)*ppDstBuffer, PIX_FMT_YUV420P, nWidth, nHeight);
			m_pFrameMix->width = nWidth;
			m_pFrameMix->height = nHeight;
			m_pFrameMix->format = PIX_FMT_YUV420P;

			// fill with white
			memset(m_pFrameMix->data[0], Y_WHITE, (m_pFrameMix->linesize[0] * nHeight));
			memset(m_pFrameMix->data[1], U_WHITE, (m_pFrameMix->linesize[1] * (nHeight >> 1)));
			memset(m_pFrameMix->data[2], V_WHITE, (m_pFrameMix->linesize[2] * (nHeight >> 1)));

			m_oLastMixedFrameResult = OTFrameVideo::New(false, *ppDstBuffer, *pDstBufferSize);
			m_oLastMixedFrameResult->setWidth(nWidth);
			m_oLastMixedFrameResult->setHeight(nHeight);

			bMixed = true;
		}

		// lock() frame
		oFrameVideo->lock();
		// mix() listener (speaker is also mixed as listener)
		_mixListener(
			(*iter).second, 
			m_pFrameMix, 
			nConsumers, nListenerIndex, 
			bIsSpeaker, (*iter).second->getSessionInfo()->isSpeaking(),
			m_parListener
			);
		// mix() Speaker
		if(bIsSpeaker)
		{
			_mixSpeaker(
					(*iter).second, 
					m_pFrameMix, 
					nConsumers,
					m_parSpeaker,
					(oStreamer && oStreamer->isOpened()) ? *oStreamer : NULL
					);
		}
		// unlock() frame
		oFrameVideo->unlock();

		++nListenerIndex;		
	}//for

	if(bMixed)
	{
		return m_oLastMixedFrameResult;
	}
	return NULL;
}

OTObjectWrapper<OTPatternVideoHangout*> OTPatternVideoHangout::New(OTObjectWrapper<OTBridgeInfo*> oBridgeInfo)
{
	return new OTPatternVideoHangout(oBridgeInfo);
}