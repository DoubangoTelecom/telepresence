/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#include "opentelepresence/OTProxyPluginConsumerVideo.h"

#include <assert.h>

static const std::string __strEmpty("");

OTProxyPluginConsumerVideo::OTProxyPluginConsumerVideo(uint64_t nId, const ProxyVideoConsumer* pcConsumer)
: OTProxyPluginConsumer(OTMediaType_Video, nId, dynamic_cast<const ProxyPlugin*>(pcConsumer)), m_pcWrappedConsumer(pcConsumer),
m_nWidth(OPENTELEPRESENCE_VIDEO_WIDTH_DEFAULT), m_nHeight(OPENTELEPRESENCE_VIDEO_HEIGHT_DEFAULT), m_nFPS(OPENTELEPRESENCE_VIDEO_FPS_DEFAULT)
{
	m_OTHeldFrameVideo = NULL;
	m_nPtime = (1000 / m_nFPS);

	m_pCallback = new OTProxyPluginConsumerVideoVideoCallback(this);
	const_cast<ProxyVideoConsumer*>(m_pcWrappedConsumer)->setCallback(m_pCallback);
}

OTProxyPluginConsumerVideo::~OTProxyPluginConsumerVideo()
{
	OT_DEBUG_INFO("*** OTProxyPluginConsumerVideo destroyed ***");
	m_pcWrappedConsumer = NULL;
	if(m_pCallback)
	{
		delete m_pCallback, m_pCallback = NULL;
	}
	OTObjectSafeRelease(m_OTHeldFrameVideo);
}

bool OTProxyPluginConsumerVideo::drawOverlay(bool bListener, 
		const void* pcInBufferPtr, size_t nInBufferSize, size_t nInWidth, size_t nInHeight, 
		size_t nOutWidth, size_t nOutHeight, void* pOutFrame
		)
{
	OTObjectWrapper<OTOverlayVideoText *> oOverlayText;
	if(bListener)
	{
		if(!m_oOverlayListenerText)
		{
			m_oOverlayListenerText = OTOverlayVideoText::New(nInWidth, nInHeight, nOutWidth, nOutHeight,
					__strEmpty, // No displayName info for listeners
					__strEmpty, // No jobTitle info for listeners
					__strEmpty // No copyright info for listeners
				);
		}
		oOverlayText = m_oOverlayListenerText;
	}
	else
	{
		if(!m_oOverlaySpeakerText)
		{
			OTObjectWrapper<OTBridgeInfo*> oBridgeInfo = m_oSessionInfo->getBridgeInfo();
			m_oOverlaySpeakerText = OTOverlayVideoText::New(nInWidth, nInHeight, nOutWidth, nOutHeight,
				oBridgeInfo->isOverlayDisplaySpeakerName() ? m_oSessionInfo->getDisplayName() : __strEmpty,
				oBridgeInfo->isOverlayDisplaySpeakerJobTitle() ?	m_oSessionInfo->getJobTitle() : __strEmpty,
				oBridgeInfo->getOverlayCopyrightText(),
				oBridgeInfo->getOverlaySpeakerNameFontSize(),
				oBridgeInfo->getOverlayCopyrightFontSize()
				);
#if 0
			if(!oBridgeInfo->getOverlayWatermarkImagePath().empty())
			{
				m_oOverlaySpeakerWatermark = OTOverlayVideoWatermark::New(nInWidth, nInHeight, nOutWidth, nOutHeight, oBridgeInfo->getOverlayWatermarkImagePath());
			}
#endif
			if(m_oOverlaySpeakerText)
			{
				m_oOverlaySpeakerText->setFontsFolderPath(oBridgeInfo->getOverlayFontsFolderPath());
				m_oOverlaySpeakerText->setCopyrightFontFileName(oBridgeInfo->getOverlayCopyrightFontFileName());
				m_oOverlaySpeakerText->setSpeakerNameFontFileName(oBridgeInfo->getOverlaySpeakerNameFontFileName());
			}
		}
		oOverlayText = m_oOverlaySpeakerText;
	}

	if(!oOverlayText)
	{
		OT_DEBUG_ERROR("Failed to create new overlay");
		return false;
	}

	if(oOverlayText->draw(pcInBufferPtr, nInBufferSize, nInWidth, nInHeight, nOutWidth, nOutHeight, pOutFrame))
	{
#if 0
		if(!bListener && m_oOverlaySpeakerWatermark)
		{
			return m_oOverlaySpeakerWatermark->draw(pcInBufferPtr, nInBufferSize, nInWidth, nInHeight, nOutWidth, nOutHeight, pOutFrame);
		}
#endif
		return true;
	}
	return false;
}

int OTProxyPluginConsumerVideo::prepareCallback(int nWidth, int nHeight, int nFps)
{
	m_nWidth = nWidth;
	m_nHeight = nHeight;
	m_nFPS = nFps;

	if(!m_OTHeldFrameVideo)
	{
		if(!(m_OTHeldFrameVideo = OTFrameVideo::New(true, NULL, (m_nWidth * m_nHeight * 3) >> 1)))
		{
			return -1;
		}
	}
	m_OTHeldFrameVideo->lock();
	m_OTHeldFrameVideo->setWidth(m_nWidth);
	m_OTHeldFrameVideo->setHeight(m_nHeight);
	m_OTHeldFrameVideo->setValidDataSize(0);
	m_OTHeldFrameVideo->unlock();

	// call parent
	return OTProxyPlugin::prepare();
}

int OTProxyPluginConsumerVideo::startCallback()
{
	// call parent
	return OTProxyPlugin::start();
}

int OTProxyPluginConsumerVideo::consumeCallback(const ProxyVideoFrame* frame)
{
	uint32_t nFrameWidth = frame->getFrameWidth();
	uint32_t nFrameHeight = frame->getFrameHeight();
	uint32_t nFrameSize = ((nFrameWidth * nFrameHeight * 3) >> 1);

	OT_ASSERT(frame->getBufferSize() == nFrameSize);
	
	m_OTHeldFrameVideo->lock();

	if(m_OTHeldFrameVideo->getBufferSize() < nFrameSize)
	{
		if(!m_OTHeldFrameVideo->resizeBuffer(nFrameSize))
		{
			goto bail;
		}
	}

	m_OTHeldFrameVideo->setWidth(nFrameWidth);
	m_OTHeldFrameVideo->setHeight(nFrameHeight);
	m_OTHeldFrameVideo->setValidDataSize(nFrameSize);

	memcpy(m_OTHeldFrameVideo->getBufferPtr(), frame->getBufferPtr(), nFrameSize);

bail:
	m_OTHeldFrameVideo->unlock();
	
	return 0;
}

int OTProxyPluginConsumerVideo::pauseCallback()
{
	// call parent
	return OTProxyPlugin::pause();
}

int OTProxyPluginConsumerVideo::stopCallback()
{
	// call parent
	return OTProxyPlugin::stop();
}

