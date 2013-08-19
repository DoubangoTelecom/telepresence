/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#include "opentelepresence/patterns/OTPatternVideoWebEx.h"

#include <assert.h>

OTPatternVideoWebEx::OTPatternVideoWebEx(OTObjectWrapper<OTBridgeInfo*> oBridgeInfo)
: OTPatternVideo(OTPatternType_WebEx, oBridgeInfo)
{
}

OTPatternVideoWebEx::~OTPatternVideoWebEx()
{

}

OTObjectWrapper<OTFrameVideo *>  OTPatternVideoWebEx::mix(std::map<uint64_t, OTObjectWrapper<OTProxyPluginConsumerVideo*> >*pOTConsumers, void** pDstBuffer, uint32_t *pDstBufferSize)
{
	OT_ASSERT(false);
	return NULL;
}