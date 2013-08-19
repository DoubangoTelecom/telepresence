/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_PLUGINCONSUMER_H
#define OPENTELEPRESENCE_PLUGINCONSUMER_H

#include "OpenTelepresenceConfig.h"

#include "OTProxyPlugin.h"
#include "ProxyConsumer.h"


class OTProxyPluginConsumer : public OTProxyPlugin
{
public:
	OTProxyPluginConsumer(OTMediaType_t eMediaType, uint64_t nId, const ProxyPlugin* pcConsumer);
	virtual ~OTProxyPluginConsumer();
};

#endif /* OPENTELEPRESENCE_PLUGINCONSUMER_H */
