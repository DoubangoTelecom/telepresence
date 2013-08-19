/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_MIXERVIDEO_H
#define OPENTELEPRESENCE_MIXERVIDEO_H

#include "opentelepresence/mixers/OTMixer.h"
#include "opentelepresence/OTFrameVideo.h"
#include "opentelepresence/patterns/OTPatternVideo.h"
#include "opentelepresence/OTProxyPluginConsumerVideo.h"

#include <map>

class OTMixerVideo : public OTMixer
{
protected:
	OTMixerVideo(OTObjectWrapper<OTBridgeInfo*> oBridgeInfo);
public:
	virtual ~OTMixerVideo();
	virtual OT_INLINE const char* getObjectId() { return "OTMixerVideo"; }
	virtual OTObjectWrapper<OTFrameVideo *> mix(std::map<uint64_t, OTObjectWrapper<OTProxyPluginConsumerVideo*> >*, void** ppDstBuffer, uint32_t* pDstBufferSize) = 0;

	static OTObjectWrapper<OTMixerVideo*> New(OTObjectWrapper<OTBridgeInfo*> oBridgeInfo);

protected:
	OTObjectWrapper<OTPatternVideo*> m_pOTPatternVideo;
};

#endif	/* OPENTELEPRESENCE_MIXERVIDEO_H */
