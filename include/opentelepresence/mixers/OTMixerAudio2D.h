/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_MIXERAUDIO2D_H
#define OPENTELEPRESENCE_MIXERAUDIO2D_H

#include "opentelepresence/mixers/OTMixerAudio.h"

class OTMixerAudio2D : public OTMixerAudio
{
protected:
	OTMixerAudio2D(OTObjectWrapper<OTBridgeInfo*> oBridgeInfo);
public:
	virtual ~OTMixerAudio2D();
	virtual OT_INLINE const char* getObjectId() { return "OTMixerAudio2D"; }

	virtual int reset();
	virtual bool isValid();
	virtual OTObjectWrapper<OTFrameAudio *> mix(std::map<uint64_t, OTObjectWrapper<OTProxyPluginConsumerAudio*> >*, uint64_t nConsumerToIgnore = 0);

	static OTObjectWrapper<OTMixerAudio2D*> New(OTObjectWrapper<OTBridgeInfo*> oBridgeInfo);

private:
	void* m_pTempBuffer;
	size_t m_nTempBufferSize;
	bool m_bValid;
	OTObjectWrapper<OTFrameAudio *> m_oFrame;
};

#endif /* OPENTELEPRESENCE_MIXERAUDIO2D_H */
