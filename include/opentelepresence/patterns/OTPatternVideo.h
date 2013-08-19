/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_PATTERNVIDEO_H
#define OPENTELEPRESENCE_PATTERNVIDEO_H

#include "opentelepresence/patterns/OTPattern.h"
#include "opentelepresence/OTProxyPluginConsumerVideo.h"
#include "opentelepresence/OTFrameVideo.h"

#include <map>

class OTPatternVideo : public OTPattern
{
protected:
	OTPatternVideo(OTPatternType_t eType, OTObjectWrapper<OTBridgeInfo*> oBridgeInfo);
public:
	virtual ~OTPatternVideo();
	virtual OT_INLINE const char* getObjectId() { return "OTPatternVideo"; }

	virtual OTObjectWrapper<OTFrameVideo *> mix(std::map<uint64_t, OTObjectWrapper<OTProxyPluginConsumerVideo*> >*, void** pDstBuffer, uint32_t *pDstBufferSize)=0;

protected:
	OTObjectWrapper<OTFrameVideo *> m_oLastMixedFrameResult;
};

#endif /* OPENTELEPRESENCE_PATTERNVIDEO_H */
