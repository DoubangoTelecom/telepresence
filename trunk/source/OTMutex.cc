/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#include "opentelepresence/OTMutex.h"

OTMutex::OTMutex(bool bRecursive /*= true*/)
{
	m_phOTMutex = tsk_mutex_create_2(bRecursive ? tsk_true : tsk_false);
}

OTMutex::~OTMutex()
{
	if(m_phOTMutex)
	{
		tsk_mutex_destroy(&m_phOTMutex);
	}
	OT_DEBUG_INFO("*** OTMutex destroyed ***");
}

bool OTMutex::lock()
{
	return (tsk_mutex_lock(m_phOTMutex) == 0);
}

bool OTMutex::unlock()
{
	return (tsk_mutex_unlock(m_phOTMutex) == 0);
}
