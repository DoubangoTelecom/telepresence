/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#include "opentelepresence/mixers/OTMixerVideo3D.h"

#include "tsk_debug.h"

OTMixerVideo3D::OTMixerVideo3D(OTObjectWrapper<OTBridgeInfo*> oBridgeInfo)
: OTMixerVideo(oBridgeInfo)
{
	m_bValid = oBridgeInfo 
		&& getMediaType() == OTMediaType_Video 
		&& oBridgeInfo->getVideoDimension() == OTDimension_3D;
}

OTMixerVideo3D::~OTMixerVideo3D()
{
}

int OTMixerVideo3D::reset()
{
	return 0;
}

bool OTMixerVideo3D::isValid()
{
	return m_bValid;
}

OTObjectWrapper<OTFrameVideo *> OTMixerVideo3D::mix(std::map<uint64_t, OTObjectWrapper<OTProxyPluginConsumerVideo*> >* pOTConsumers, void** ppDstBuffer, uint32_t* pDstBufferSize)
{
	OT_DEBUG_ERROR("Not supported");
	return NULL;
}


OTObjectWrapper<OTMixerVideo3D*> OTMixerVideo3D::New(OTObjectWrapper<OTBridgeInfo*> oBridgeInfo)
{
	OTObjectWrapper<OTMixerVideo3D*> oMixerVideo3D = new OTMixerVideo3D(oBridgeInfo);
	if(oMixerVideo3D && !oMixerVideo3D->isValid())
	{
		OTObjectSafeRelease(oMixerVideo3D);
	}
	return oMixerVideo3D;
}
