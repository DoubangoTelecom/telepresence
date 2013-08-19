/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#include "opentelepresence/mixers/OTMixerVideo.h"
#include "opentelepresence/mixers/OTMixerVideo2D.h"
#include "opentelepresence/mixers/OTMixerVideo3D.h"

OTMixerVideo::OTMixerVideo(OTObjectWrapper<OTBridgeInfo*> oBridgeInfo)
: OTMixer(OTMediaType_Video, oBridgeInfo)
{
}

OTMixerVideo::~OTMixerVideo()
{
	OTObjectSafeRelease(m_pOTPatternVideo);
}

OTObjectWrapper<OTMixerVideo*> OTMixerVideo::New(OTObjectWrapper<OTBridgeInfo*> oBridgeInfo)
{
	switch(oBridgeInfo->getVideoDimension())
	{
		case OTDimension_2D:
		{
			OTObjectWrapper<OTMixerVideo2D*> oMixerVideo2D = OTMixerVideo2D::New(oBridgeInfo);
			if(oMixerVideo2D && oMixerVideo2D->isValid())
			{
				return dynamic_cast<OTMixerVideo*>(*oMixerVideo2D);
			}
			break;
		}
		case OTDimension_3D:
		{
			OTObjectWrapper<OTMixerVideo3D*> oMixerVideo3D = OTMixerVideo3D::New(oBridgeInfo);
			if(oMixerVideo3D && oMixerVideo3D->isValid())
			{
				return dynamic_cast<OTMixerVideo*>(*oMixerVideo3D);
			}
			break;
		}
	}
	return NULL;
}
