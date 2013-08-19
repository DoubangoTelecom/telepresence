/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_ENGINEOSX_H
#define OPENTELEPRESENCE_ENGINEOSX_H

#include "OpenTelepresenceConfig.h"

#if OPENTELEPRESENCE_UNDER_OSX

#include "opentelepresence/OTEngine.h"

class OTEngineOSX : public OTEngine
{
public:
	OTEngineOSX();
	virtual ~OTEngineOSX();
	virtual OT_INLINE const char* getObjectId() { return "OTEngineOSX"; }

public:
	virtual bool start();
	virtual bool stop();

protected:
	virtual bool isValid();
};

#endif /* OPENTELEPRESENCE_UNDER_OSX */


#endif /* OPENTELEPRESENCE_ENGINEOSX_H */
