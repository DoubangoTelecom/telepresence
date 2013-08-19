/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_MIXER_H
#define OPENTELEPRESENCE_MIXER_H

#include "OpenTelepresenceConfig.h"

#include "opentelepresence/OTObject.h"
#include "opentelepresence/OTBridgeInfo.h"

class OTMixer : public OTObject
{
protected:
	OTMixer(OTMediaType_t eMediaType, OTObjectWrapper<OTBridgeInfo*> oBridgeInfo);
public:
	virtual ~OTMixer();
	virtual OT_INLINE const char* getObjectId() { return "OTMixer"; }

	virtual int reset() = 0;
	virtual bool isValid() = 0;
	virtual bool releaseConsumerInternals(uint64_t nConsumerId) { return true; };

	virtual OT_INLINE OTMediaType_t getMediaType() { return m_eMediaType; }
	virtual OT_INLINE OTDimension_t getDimension() 
	{ 
		switch(getMediaType())
		{
			case OTMediaType_Audio: return  m_oBridgeInfo->getAudioDimension();
			case OTMediaType_Video: return  m_oBridgeInfo->getVideoDimension();
			default: return OTDimension_2D;
		}
		
	}

	static OTObjectWrapper<OTMixer*> New(OTMediaType_t eMediaType, OTObjectWrapper<OTBridgeInfo*> oBridgeInfo);

protected:
	OTObjectWrapper<OTBridgeInfo*> m_oBridgeInfo;
	OTMediaType_t m_eMediaType;
};

#endif /* OPENTELEPRESENCE_MIXER_H */

