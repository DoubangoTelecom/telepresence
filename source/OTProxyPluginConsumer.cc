/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#include "opentelepresence/OTProxyPluginConsumer.h"

OTProxyPluginConsumer::OTProxyPluginConsumer(OTMediaType_t eMediaType, uint64_t nId, const ProxyPlugin* pcConsumer)
: OTProxyPlugin(eMediaType, nId, dynamic_cast<const ProxyPlugin*>(pcConsumer))
{

}

OTProxyPluginConsumer::~OTProxyPluginConsumer()
{
	OT_DEBUG_INFO("*** OTProxyPluginConsumer destroyed ***");
}