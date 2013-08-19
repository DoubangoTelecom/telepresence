/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_MIXERVIDEO2D_H
#define OPENTELEPRESENCE_MIXERVIDEO2D_H

#include "opentelepresence/mixers/OTMixerVideo.h"
#include "opentelepresence/patterns/OTPatternVideo.h"

class OTMixerVideo2D : public OTMixerVideo
{
protected:
	OTMixerVideo2D(OTObjectWrapper<OTBridgeInfo*> oBridgeInfo);
public:
	virtual ~OTMixerVideo2D();
	virtual OT_INLINE const char* getObjectId() { return "OTMixerVideo2D"; }

	virtual int reset();
	virtual bool isValid();
	virtual OTObjectWrapper<OTFrameVideo *> mix(std::map<uint64_t, OTObjectWrapper<OTProxyPluginConsumerVideo*> >*, void** ppDstBuffer, uint32_t* pDstBufferSize);

	static OTObjectWrapper<OTMixerVideo2D*> New(OTObjectWrapper<OTBridgeInfo*> oBridgeInfo);

private:
	bool m_bValid;
	int32_t m_nPatterPosition;
	OTObjectWrapper<OTPatternVideo*>m_pOTPatternVideo;
};

#endif /* OPENTELEPRESENCE_MIXERVIDEO2D_H */
