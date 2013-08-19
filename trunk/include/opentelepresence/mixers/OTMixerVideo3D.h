/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_MIXERVIDEO3D_H
#define OPENTELEPRESENCE_MIXERVIDEO3D_H

#include "opentelepresence/mixers/OTMixerVideo.h"

class OTMixerVideo3D : public OTMixerVideo
{
protected:
	OTMixerVideo3D(OTObjectWrapper<OTBridgeInfo*> oBridgeInfo);
public:
	virtual ~OTMixerVideo3D();
	virtual OT_INLINE const char* getObjectId() { return "OTMixerVideo3D"; }

	virtual int reset();
	virtual bool isValid();
	virtual OTObjectWrapper<OTFrameVideo *> mix(std::map<uint64_t, OTObjectWrapper<OTProxyPluginConsumerVideo*> >*, void** ppDstBuffer, uint32_t* pDstBufferSize);

	static OTObjectWrapper<OTMixerVideo3D*> New(OTObjectWrapper<OTBridgeInfo*> oBridgeInfo);

private:
	bool m_bValid;
};

#endif /* OPENTELEPRESENCE_MIXERVIDEO3D_H */
