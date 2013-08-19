/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#include "opentelepresence/patterns/OTPattern.h"

OTPattern::OTPattern(OTPatternType_t eType, OTObjectWrapper<OTBridgeInfo*> oBridgeInfo)
: OTObject()
, m_eType(eType)
, m_oBridgeInfo(oBridgeInfo)
{
	
}

OTPattern::~OTPattern()
{
	
}
