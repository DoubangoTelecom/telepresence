/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#include "opentelepresence/platforms/osx/OTEngineOSX.h"

#if OPENTELEPRESENCE_UNDER_OSX

#include "tsk_debug.h"

OTEngineOSX::OTEngineOSX()
:OTEngine()
{
}

OTEngineOSX::~OTEngineOSX()
{
}

bool OTEngineOSX::isValid()
{
	return true;
}

bool OTEngineOSX::start()
{
	return OTEngine::start();
}

bool OTEngineOSX::stop()
{
	return OTEngine::stop();
}


#endif /* OPENTELEPRESENCE_UNDER_OSX */
