/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#include "opentelepresence/OTBridge.h"
#include "opentelepresence/OTSessionInfo.h"
#include "opentelepresence/OTWrap.h"
#include "opentelepresence/OTEngine.h"

#include "SipStack.h"
#include "SipSession.h"
#include "SipEvent.h"
#include "ProxyPluginMgr.h"
#include "MediaSessionMgr.h"

#include "tsk_debug.h"

#include <algorithm>
#include <assert.h>

//
//	OTBridge
//

OTBridge::OTBridge(std::string strId, OTObjectWrapper<OTEngineInfo*> oEngineInfo)
: m_strId(strId)
, m_bValid(false)
, m_bRunning(false)
{
	OT_ASSERT(!m_strId.empty());

	// FIXME: running, valid,... must go into Info
	m_oInfo = new OTBridgeInfo(m_strId, oEngineInfo);
	
	if(m_oInfo)
	{
		// for now we always create a mixer with all supported media
		// => should be dynamic (depends on the current media stream type)
		m_oMixerMgrMgr = OTMixerMgrMgr::New(OTMediaType_All, m_oInfo);
	}

	m_bValid = (m_oMixerMgrMgr && m_oInfo);

	OT_DEBUG_INFO("Create new bridge with id = '%s'", m_strId.c_str());
}

OTBridge::~OTBridge()
{
	stop();

	OT_DEBUG_INFO("*** OTBridge(%s) destroyed ***", m_strId.c_str());
}

int OTBridge::start()
{
	if(isRunning())
	{
		return 0;
	}

	if(!isValid())
	{
		OT_DEBUG_ERROR("Not valid");
		return -1;
	}

	OT_DEBUG_INFO("Bride(%s) start", m_strId.c_str());

	m_bRunning = true;

	// FIXME:
#if 0
	{
		OTObjectWrapper<OTDocStreamer*> oDocStreamer = OTDocStreamer::New(m_oInfo, this);
		m_oDocStreamers[oDocStreamer->getId()] = oDocStreamer;
		oDocStreamer->open("C:/Users/root/Desktop/Doubango Presentation_OO.ppt");
	}
#endif

	return m_oMixerMgrMgr->start(OTMediaType_All);
}

int OTBridge::stop()
{
	if(!isRunning())
	{
		return 0;
	}

	if(!isValid())
	{
		OT_DEBUG_ERROR("Not valid");
		return -1;
	}

	OT_DEBUG_INFO("Bride(%s) stop", m_strId.c_str());

	m_bRunning = false;

	return m_oMixerMgrMgr->stop(OTMediaType_All);
}



int OTBridge::hangUpSession(uint64_t nSessionId)
{
	int ret = -1;
	if(!isValid())
	{
		OT_DEBUG_ERROR("Bridge not valid");
		return -1;
	}

	OTObjectWrapper<OTSipSessionAV*> oAVCall = m_oAVCalls[nSessionId];
	if(oAVCall)
	{
		if(const_cast<CallSession*>(oAVCall->getWrappedCallSession())->hangup())
		{
			ret = 0;
		}
	}

	return ret;
}

int OTBridge::addAVCall(OTObjectWrapper<OTSipSessionAV*> oAVCall)
{
	if(!oAVCall)
	{
		OT_DEBUG_ERROR("Invalid argument");
		return -1;
	}

	// add to the list
	m_oAVCalls[oAVCall->getId()] = oAVCall;
	OT_DEBUG_INFO("Bridge(%s).avcalls.count = %u", m_strId.c_str(), m_oAVCalls.size());
	// attach media plugins
	return m_oMixerMgrMgr->attachMediaPlugins(oAVCall->getInfo());
}

int OTBridge::removeAVCall(uint64_t nSessionId)
{
	// detach media plugins
	std::map<uint64_t, OTObjectWrapper<OTSipSessionAV*> >::iterator iter = m_oAVCalls.find(nSessionId);
	OTObjectWrapper<OTSipSessionAV*> oAVCall;
	if(iter != m_oAVCalls.end())
	{
		if((oAVCall = iter->second))
		{
			m_oMixerMgrMgr->deAttachMediaPlugins(oAVCall->getInfo());
			OT_DEBUG_INFO("Bridge(%s).avcalls.count = %u", m_strId.c_str(), m_oAVCalls.size());
			m_oAVCalls.erase(iter);
		}
	}
	
	
	return 0;
}

OTObjectWrapper<OTSipSessionAV*> OTBridge::findCallByUserId(std::string strUserId)
{
	std::map<uint64_t, OTObjectWrapper<OTSipSessionAV*> > oAVCalls = m_oAVCalls; // copy() for thread safety
	std::map<uint64_t, OTObjectWrapper<OTSipSessionAV*> >::const_iterator iter;
	for(iter = oAVCalls.begin(); iter != oAVCalls.end(); ++iter)
	{
		if(iter->second->getInfo()->getUserId() == strUserId)
		{
			return iter->second;
		}
	}
	return NULL;
}

OTObjectWrapper<OTSipSessionAV*> OTBridge::findCallBySessionId(uint64_t nSessionId)
{
	std::map<uint64_t, OTObjectWrapper<OTSipSessionAV*> > oAVCalls = m_oAVCalls; // copy() for thread safety
	std::map<uint64_t, OTObjectWrapper<OTSipSessionAV*> >::iterator iter;
	if((iter = oAVCalls.find(nSessionId)) != oAVCalls.end())
	{
		return iter->second;
	}
	return NULL;
}

size_t OTBridge::getNumberOfActiveAVCalls()
{
	return m_oAVCalls.size();
}
