/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_PATTERN_H
#define OPENTELEPRESENCE_PATTERN_H

#include "OpenTelepresenceConfig.h"
#include "opentelepresence/OTObject.h"
#include "opentelepresence/OTCommon.h"
#include "opentelepresence/OTBridgeInfo.h"

class OTPattern : public OTObject
{
protected:
	OTPattern(OTPatternType_t eType, OTObjectWrapper<OTBridgeInfo*> oBridgeInfo);
public:
	virtual ~OTPattern();
	virtual OT_INLINE const char* getObjectId() { return "OTPattern"; }

	virtual OT_INLINE OTPatternType_t getType() { return m_eType; }

protected:
	OTPatternType_t m_eType;
	OTObjectWrapper<OTBridgeInfo*> m_oBridgeInfo;
};

#endif /* OPENTELEPRESENCE_PATTERN_H */
