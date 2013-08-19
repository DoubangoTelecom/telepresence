/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_MIXERAUDIO_H
#define OPENTELEPRESENCE_MIXERAUDIO_H

#include "opentelepresence/mixers/OTMixer.h"
#include "opentelepresence/OTFrameAudio.h"
#include "opentelepresence/OTProxyPluginConsumerAudio.h"

#include <map>

class OTMixerAudio : public OTMixer
{
protected:
	OTMixerAudio(OTObjectWrapper<OTBridgeInfo*> oBridgeInfo);
public:
	virtual ~OTMixerAudio();
	virtual OT_INLINE const char* getObjectId() { return "OTMixerAudio"; }
	virtual OTObjectWrapper<OTFrameAudio *> mix(std::map<uint64_t, OTObjectWrapper<OTProxyPluginConsumerAudio*> >*, uint64_t nConsumerToIgnore = 0) = 0;

	static OTObjectWrapper<OTMixerAudio*> New(OTObjectWrapper<OTBridgeInfo*> oBridgeInfo);
};

#endif	/* OPENTELEPRESENCE_MIXERAUDIO_H */
