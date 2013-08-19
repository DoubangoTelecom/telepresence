/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_ENGINEWIN32_H
#define OPENTELEPRESENCE_ENGINEWIN32_H

#include "OpenTelepresenceConfig.h"

#if OPENTELEPRESENCE_UNDER_WINDOWS

#include "opentelepresence/OTEngine.h"

class OTEngineWin32 : public OTEngine
{
public:
	OTEngineWin32();
	virtual ~OTEngineWin32();
	virtual OT_INLINE const char* getObjectId() { return "OTEngineWin32"; }

public:
	virtual bool start();
	virtual bool stop();

protected:
	virtual bool isValid();

private:	
	static int initialize();
	static int deInitialize();

	static ULONG_PTR g_pGdiplusToken;
	static bool g_bInitialized;

	bool m_bValid;
};

#endif /* OPENTELEPRESENCE_UNDER_WINDOWS */


#endif /* OPENTELEPRESENCE_ENGINEWIN32_H */
