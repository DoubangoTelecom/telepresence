/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#include "opentelepresence/OTProxyPlugin.h"

OTProxyPlugin::OTProxyPlugin(OTMediaType_t eMediaType, uint64_t nId, const ProxyPlugin* pcProxyPlugin)
: OTObject()
{
	m_eMediaType = eMediaType;
	m_nId = nId;
	m_pcProxyPlugin = pcProxyPlugin;
	m_bValid = true;
	m_bStarted = false;
	m_bPaused = false;
	m_phMutex = tsk_mutex_create_2(tsk_false);
}


OTProxyPlugin::~OTProxyPlugin()
{
	m_pcProxyPlugin = NULL;
	tsk_mutex_destroy(&m_phMutex);

	OT_DEBUG_INFO("*** OTProxyPlugin destroyed ***");
}
