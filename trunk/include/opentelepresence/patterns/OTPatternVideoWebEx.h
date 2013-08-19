/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_PATTERNVIDEO_WEBEX_H
#define OPENTELEPRESENCE_PATTERNVIDEO_WEBEX_H

#include "opentelepresence/patterns/OTPatternVideo.h"

class OTPatternVideoWebEx : public OTPatternVideo
{
protected:
	OTPatternVideoWebEx(OTObjectWrapper<OTBridgeInfo*> oBridgeInfo);
public:
	virtual ~OTPatternVideoWebEx();
	virtual OT_INLINE const char* getObjectId() { return "OTPatternVideoWebEx"; }

	// Override from OTPatternVideo
	virtual OTObjectWrapper<OTFrameVideo *> mix(std::map<uint64_t, OTObjectWrapper<OTProxyPluginConsumerVideo*> >* pOTConsumers, void** pDstBuffer, uint32_t *pDstBufferSize);

};

#endif /* OPENTELEPRESENCE_PATTERNVIDEO_WEBEX_H */
