/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#include "opentelepresence/OTProxyPluginProducer.h"

#include "MediaSessionMgr.h" /* Codec */

OTProxyPluginProducer::OTProxyPluginProducer(OTMediaType_t eMediaType, uint64_t nId, const ProxyPlugin* pcProducer)
: OTProxyPlugin(eMediaType, nId, dynamic_cast<const ProxyPlugin*>(pcProducer))
, m_pCodec(NULL)
{
	
}

OTProxyPluginProducer::~OTProxyPluginProducer()
{
	OT_DEBUG_INFO("*** OTProxyPluginProducer destroyed ***");
	setCodec(NULL);
}

void OTProxyPluginProducer::setCodec(Codec* pcCodec)
{
	if(m_pCodec)
	{
		delete m_pCodec;
		m_pCodec = NULL;
	}

	if(pcCodec)
	{
		m_pCodec = new Codec(pcCodec->getWrappedCodec());
	}
}
