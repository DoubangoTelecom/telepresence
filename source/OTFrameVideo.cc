/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#include "opentelepresence/OTFrameVideo.h"

#include <assert.h>

OTFrameVideo::OTFrameVideo(bool bOwnBuffer, const void *pBufferPtr, uint32_t nBufferSize)
:OTFrame(OTMediaType_Video, bOwnBuffer, pBufferPtr, nBufferSize)
{
	m_nWidth = 0;
	m_nHeight = 0;
}

OTFrameVideo::~OTFrameVideo()
{
	
}


OTObjectWrapper<OTFrameVideo*> OTFrameVideo::New(bool bOwnBuffer, const void *pBufferPtr, uint32_t nBufferSize)
{
	return new OTFrameVideo(bOwnBuffer, pBufferPtr, nBufferSize);
}
