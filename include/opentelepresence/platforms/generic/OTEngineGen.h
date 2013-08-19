/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_ENGINE_GEN_H
#define OPENTELEPRESENCE_ENGINE_GEN_H

#include "OpenTelepresenceConfig.h"

#include "opentelepresence/OTEngine.h"

class OTEngineGen : public OTEngine
{
public:
	OTEngineGen();
	virtual ~OTEngineGen();
	virtual OT_INLINE const char* GetObjectId() { return "OTEngineGen"; }

public:
	virtual bool start();
	virtual bool stop();

protected:
	virtual bool isValid();
};


#endif /* OPENTELEPRESENCE_ENGINE_GEN_H */
