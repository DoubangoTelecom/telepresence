/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_PRESENCEPARSER_H
#define OPENTELEPRESENCE_PRESENCEPARSER_H

#include "OpenTelepresenceConfig.h"
#include "opentelepresence/OTObject.h"

class OTPresenceParser : public OTObject
{
protected:
	OTPresenceParser();
public:
	virtual ~OTPresenceParser();
	virtual OT_INLINE const char* getObjectId() { return "OTPresenceParser"; }
};

#endif /* OPENTELEPRESENCE_PRESENCEPARSER_H */
