/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#include "opentelepresence/platforms/generic/OTEngineGen.h"

OTEngineGen::OTEngineGen()
:OTEngine()
{
}

OTEngineGen::~OTEngineGen()
{
}

bool OTEngineGen::isValid()
{
	return true;
}

bool OTEngineGen::start()
{
	return OTEngine::start();
}

bool OTEngineGen::stop()
{
	return OTEngine::stop();
}

