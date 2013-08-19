/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#include "opentelepresence/mixers/OTMixerAudio.h"
#include "opentelepresence/mixers/OTMixerAudio2D.h"
#include "opentelepresence/mixers/OTMixerAudio3D.h"

OTMixerAudio::OTMixerAudio(OTObjectWrapper<OTBridgeInfo*> oBridgeInfo)
:OTMixer(OTMediaType_Audio, oBridgeInfo)
{
}

OTMixerAudio::~OTMixerAudio()
{
}

OTObjectWrapper<OTMixerAudio*> OTMixerAudio::New(OTObjectWrapper<OTBridgeInfo*> oBridgeInfo)
{
	switch(oBridgeInfo->getAudioDimension())
	{
		case OTDimension_2D:
		{
			OTObjectWrapper<OTMixerAudio2D*> oMixerAudio2D = OTMixerAudio2D::New(oBridgeInfo);
			if(oMixerAudio2D)
			{
				return dynamic_cast<OTMixerAudio*>(*oMixerAudio2D);
			}
			break;
		}
		case OTDimension_3D:
		{
			OTObjectWrapper<OTMixerAudio3D*> oMixerAudio3D = OTMixerAudio3D::New(oBridgeInfo);
			if(oMixerAudio3D)
			{
				return dynamic_cast<OTMixerAudio*>(*oMixerAudio3D);
			}
			break;
		}
	}
	return NULL;
}
