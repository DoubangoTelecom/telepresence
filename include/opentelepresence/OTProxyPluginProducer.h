/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_PLUGINPRODUCER_H
#define OPENTELEPRESENCE_PLUGINPRODUCER_H

#include "OpenTelepresenceConfig.h"

#include "OTProxyPlugin.h"
#include "ProxyProducer.h"

class Codec;

class OTProxyPluginProducer : public OTProxyPlugin
{
public:
	OTProxyPluginProducer(OTMediaType_t eMediaType, uint64_t nId, const ProxyPlugin* pcProducer);
	virtual ~OTProxyPluginProducer();

	virtual OT_INLINE const Codec* getCodec(){ return m_pCodec; }
	virtual void setCodec(Codec* pcCodec);

private:
	Codec* m_pCodec;
};


#endif /* OPENTELEPRESENCE_PLUGINPRODUCER_H */
