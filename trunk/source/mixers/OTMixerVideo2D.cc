/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#include "opentelepresence/mixers/OTMixerVideo2D.h"
#include "opentelepresence/patterns/OTPatternVideoWebEx.h"
#include "opentelepresence/patterns/OTPatternVideoHangout.h"

#include "tsk_debug.h"

#include <assert.h>

OTMixerVideo2D::OTMixerVideo2D(OTObjectWrapper<OTBridgeInfo*> oBridgeInfo)
: OTMixerVideo(oBridgeInfo)
{
	m_pOTPatternVideo = NULL;
	m_nPatterPosition = 0;
	m_bValid = oBridgeInfo 
		&& getMediaType() == OTMediaType_Video 
		&& oBridgeInfo->getVideoDimension() == OTDimension_2D;

	// FIXME: what about other patterns (e.g. WebEx)
	m_pOTPatternVideo = dynamic_cast<OTPatternVideoHangout*>(*OTPatternVideoHangout::New(oBridgeInfo));
}

OTMixerVideo2D::~OTMixerVideo2D()
{
	OT_DEBUG_INFO("*** OTMixerVideo2D destroyed ***");
}

int OTMixerVideo2D::reset()
{
	m_nPatterPosition = 0;
	return 0;
}

bool OTMixerVideo2D::isValid()
{
	return m_bValid;
}

OTObjectWrapper<OTFrameVideo *> OTMixerVideo2D::mix(std::map<uint64_t, OTObjectWrapper<OTProxyPluginConsumerVideo*> >* pOTConsumers, void** ppDstBuffer, uint32_t* pDstBufferSize)
{
	return m_pOTPatternVideo->mix(pOTConsumers, ppDstBuffer, pDstBufferSize);;
}

OTObjectWrapper<OTMixerVideo2D*> OTMixerVideo2D::New(OTObjectWrapper<OTBridgeInfo*> oBridgeInfo)
{
	OTObjectWrapper<OTMixerVideo2D*> oMixerVideo2D = new OTMixerVideo2D(oBridgeInfo);
	if(oMixerVideo2D && !oMixerVideo2D->isValid())
	{
		OTObjectSafeRelease(oMixerVideo2D);
	}
	return oMixerVideo2D;
}
