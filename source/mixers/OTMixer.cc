/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#include "opentelepresence/mixers/OTMixer.h"
#include "opentelepresence/mixers/OTMixerAudio.h"
#include "opentelepresence/mixers/OTMixerVideo.h"

OTMixer::OTMixer(OTMediaType_t eMediaType, OTObjectWrapper<OTBridgeInfo*> oBridgeInfo)
: m_eMediaType(eMediaType)
, m_oBridgeInfo(oBridgeInfo)
{
	
}

OTMixer::~OTMixer()
{
	
}

OTObjectWrapper<OTMixer*> OTMixer::New(OTMediaType_t eMediaType, OTObjectWrapper<OTBridgeInfo*> oBridgeInfo)
{
	OTObjectWrapper<OTMixer*> oMixer;

	switch(eMediaType)
	{
		case OTMediaType_Audio:
		{
			OTObjectWrapper<OTMixerAudio*>oMixerAudio = OTMixerAudio::New(oBridgeInfo);
			if(oMixerAudio)
			{
				oMixer = dynamic_cast<OTMixer*>(*oMixerAudio);
			}
			break;
		}
		case OTMediaType_Video:
		{
			OTObjectWrapper<OTMixerVideo*>oMixerVideo = OTMixerVideo::New(oBridgeInfo);
			if(oMixerVideo)
			{
				oMixer = dynamic_cast<OTMixer*>(*oMixerVideo);
			}
			break;
		}
	}

	if(oMixer && !oMixer->isValid())
	{
		OTObjectSafeRelease(oMixer);
	}
	return oMixer;
}
