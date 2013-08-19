/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#include "opentelepresence/platforms/win32/OTEngineWin32.h"

#if OPENTELEPRESENCE_UNDER_WINDOWS

#include "tsk_debug.h"

#include <Gdiplus.h>

ULONG_PTR OTEngineWin32::g_pGdiplusToken = NULL;
bool OTEngineWin32::g_bInitialized = FALSE;

OTEngineWin32::OTEngineWin32()
: OTEngine()
{
	int ret;

	m_bValid = FALSE;

	if(!OTEngineWin32::g_bInitialized && (ret = OTEngineWin32::initialize())){
		OT_DEBUG_ERROR("OTEngineWin32::Initialize failed with error code=%d", ret);
		return;
	}

	if((m_bValid = g_bInitialized))
	{

	}
}

OTEngineWin32::~OTEngineWin32()
{
}

bool OTEngineWin32::isValid()
{
	return m_bValid;
}

bool OTEngineWin32::start()
{
	if(!OTEngineWin32::g_bInitialized)
	{
		OT_DEBUG_ERROR("Engine not initialized. Did you forget to call OTEngine::Initialize()");
		return false;
	}

	// call parent
	return OTEngine::start();
}

bool OTEngineWin32::stop()
{
	if(!OTEngineWin32::g_bInitialized)
	{
		OT_DEBUG_ERROR("Engine not initialized. Did you forget to call OTEngine::Initialize()");
		return false;
	}

	// call parent
	return OTEngine::stop();
}

int OTEngineWin32::initialize()
{
	if(OTEngineWin32::g_bInitialized)
	{
		OT_DEBUG_WARN("Engine already initialized");
		return 0;
	}


	using namespace Gdiplus;
	// Initialize GDI+
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&OTEngineWin32::g_pGdiplusToken, &gdiplusStartupInput, NULL);
	// Process priorty
	// SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
	
	OTEngineWin32::g_bInitialized = TRUE;
	return 0;
}

int OTEngineWin32::deInitialize()
{
	if(!OTEngineWin32::g_bInitialized)
	{
		OT_DEBUG_WARN("not initialized");
		return 0;
	}

	using namespace Gdiplus;
	// Uninitialize GDI+
	GdiplusShutdown(OTEngineWin32::g_pGdiplusToken);

	OTEngineWin32::g_bInitialized = FALSE;
	return 0;
}

#endif /* OPENTELEPRESENCE_UNDER_WINDOWS */
