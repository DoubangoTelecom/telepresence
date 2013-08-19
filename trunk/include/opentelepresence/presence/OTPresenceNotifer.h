/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_PRESENCENOTIFIER_H
#define OPENTELEPRESENCE_PRESENCENOTIFIER_H

#include "OpenTelepresenceConfig.h"
#include "opentelepresence/OTObject.h"

class OTPresenceNotifer : public OTObject
{
protected:
	OTPresenceNotifer();
public:
	virtual ~OTPresenceNotifer();
	virtual OT_INLINE const char* getObjectId() { return "OTPresenceNotifer"; }
};

#endif /* OPENTELEPRESENCE_PRESENCENOTIFIER_H */
