/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#include "opentelepresence/docstreamer/OTDocStreamer.h"
#include "opentelepresence/docstreamer/OTDocStreamerOpenOffice.h"

#include "tsk_debug.h"

uint64_t OTDocStreamer::g_uId = 0;

//
//	OTDocStreamer
//

OTDocStreamer::OTDocStreamer(OTDocStreamerType_t eType, OTObjectWrapper<OTBridgeInfo*> oBridgeInfo, const OTDocStreamerCallback* pcCallback /*= NULL*/)
: OTObject()
, m_eType(eType)
, m_uId(++g_uId)
, m_oBridgeInfo(oBridgeInfo)
, m_eState(OTDocStreamerState_None)
, m_pcCallback(pcCallback)
{
}

OTDocStreamer::~OTDocStreamer()
{
	OT_DEBUG_INFO("*** OTDocStreamer(%s,%llu) destroyed ***", getObjectId(), m_uId);
}

bool OTDocStreamer::isSupported()
{
#if HAVE_OPENOFFICE /* || HAVE_HARTALLO */
	return true;
#else
	return false;
#endif
}

std::string OTDocStreamer::buildCommandArgs(unsigned short nLocalPort)
{
#if HAVE_OPENOFFICE
	return OTDocStreamerOpenOffice::buildCommandArgs(nLocalPort);
#else
	return "";
#endif
}

std::string OTDocStreamer::buildConnectionString(unsigned short nLocalPort)
{
#if HAVE_OPENOFFICE
	return OTDocStreamerOpenOffice::buildConnectionString(nLocalPort);
#else
	return "";
#endif
}

OTObjectWrapper<OTDocStreamer*> OTDocStreamer::New(OTObjectWrapper<OTBridgeInfo*> oBridgeInfo, const OTDocStreamerCallback* pcCallback /*= NULL*/)
{
	OTObjectWrapper<OTDocStreamer*> oStreamer;

#if HAVE_OPENOFFICE
	oStreamer = new OTDocStreamerOpenOffice(oBridgeInfo, pcCallback);
#endif /* HAVE_OPENOFFICE */

#if HAVE_HARTALLO /* direct conversion from document to H.264 SVC */
	oStreamer = new OTDocStreamerHartallo(oBridgeInfo, pcCallback);
#endif

	if(oStreamer && !oStreamer->isValid())
	{
		OTObjectSafeRelease(oStreamer);
	}

	return oStreamer;
}

