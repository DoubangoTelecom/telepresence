/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_MIXERAUDIO3D_H
#define OPENTELEPRESENCE_MIXERAUDIO3D_H

#include "opentelepresence/mixers/OTMixerAudio.h"

class OTMixerAudio3D : public OTMixerAudio
{
protected:
	OTMixerAudio3D(OTObjectWrapper<OTBridgeInfo*> oBridgeInfo);
public:
	virtual ~OTMixerAudio3D();
	virtual OT_INLINE const char* getObjectId() { return "OTMixerAudio3D"; }

	virtual int reset();
	virtual bool isValid();
	virtual OTObjectWrapper<OTFrameAudio *> mix(std::map<uint64_t, OTObjectWrapper<OTProxyPluginConsumerAudio*> >*, uint64_t nConsumerToIgnore = 0) = 0;

	static OTObjectWrapper<OTMixerAudio3D*> New(OTObjectWrapper<OTBridgeInfo*> oBridgeInfo);

private:
	bool m_bValid;
};

#endif /* OPENTELEPRESENCE_MIXERAUDIO3D_H */
