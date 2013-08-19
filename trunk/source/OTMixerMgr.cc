/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#include "opentelepresence/OTMixerMgr.h"
#include "opentelepresence/OTMixerMgrAudio.h"
#include "opentelepresence/OTMixerMgrVideo.h"

#include "tsk_debug.h"

#include <assert.h>

OTMixerMgr::OTMixerMgr(OTMediaType_t eMediaType, OTObjectWrapper<OTBridgeInfo*> oBridgeInfo)
:OTObject()
{
	m_eMediaType = eMediaType;
	m_oBridgeInfo = oBridgeInfo;
}

OTMixerMgr::~OTMixerMgr()
{
	// FIXME: loop and release()?
	m_OTCodecs.clear();
}

/**
Finds the best codec to use.
/!\Function not thread safe
@param listOfCodecsToSearchInto List of codecs combined using binary OR (|).
@return The best codec if exist, otherwise NULL
*/
OTObjectWrapper<OTCodec*> OTMixerMgr::findBestCodec(OTCodec_Type_t eListOfCodecsToSearchInto)
{
	std::map<OTCodec_Type_t, OTObjectWrapper<OTCodec*> >::iterator iter = m_OTCodecs.begin();
	while(iter != m_OTCodecs.end())
	{
		if(((*iter).first & eListOfCodecsToSearchInto) == (*iter).first)
		{
			return (*iter).second;
		}
		++iter;
	}
	return NULL;
}

/**
Removes a list of codecs
/!\Function not thread safe
@param listOfCodecsToSearchInto List of codecs combined using binary OR (|).
@return True if removed, False otherwise
*/
bool OTMixerMgr::removeCodecs(OTCodec_Type_t eListOfCodecsToSearchInto)
{
	bool bFound = false;
	std::map<OTCodec_Type_t, OTObjectWrapper<OTCodec*> >::iterator iter;
	
again:
	iter = m_OTCodecs.begin();
	while(iter != m_OTCodecs.end())
	{
		if(((*iter).first & eListOfCodecsToSearchInto) == (*iter).first)
		{
			(*iter).second->releaseRef();
			m_OTCodecs.erase(iter);
			bFound = true;
			goto again;
		}
		++iter;
	}
	return bFound;
}

/**
Adds a new codec to the list of supported codecs.
/!\Function not thread safe
@param oCodec The codec to add. Must not be NULL.
@return True if codec successfully added, False otherwise
*/
bool OTMixerMgr::addCodec(OTObjectWrapper<OTCodec*> oCodec)
{
	OT_ASSERT(*oCodec);
	
	std::map<OTCodec_Type_t, OTObjectWrapper<OTCodec*> >::iterator iter = m_OTCodecs.find(oCodec->getType());
	if(iter != m_OTCodecs.end())
	{
		OT_DEBUG_ERROR("Codec with type = %d already exist", oCodec->getType());
		return false;
	}
	m_OTCodecs.insert(std::pair<OTCodec_Type_t, OTObjectWrapper<OTCodec*> >(oCodec->getType(), oCodec));
	return true;
}

/**
Executes action on all codecs.
/!\Function not thread safe
@return True if codec successfully added, False otherwise
*/
bool OTMixerMgr::executeActionOnCodecs(OTCodecAction_t eAction)
{
	std::map<OTCodec_Type_t, OTObjectWrapper<OTCodec*> >::iterator iter = m_OTCodecs.begin();
	while(iter != m_OTCodecs.end())
	{
		(*iter).second->executeAction(eAction);
		++iter;
	}
	return true;
}

OTObjectWrapper<OTMixerMgr*> OTMixerMgr::New(OTMediaType_t eMediaType, OTObjectWrapper<OTBridgeInfo*> oBridgeInfo)
{
	OTObjectWrapper<OTMixerMgr*> pOTMixer;

	switch(eMediaType)
	{
		case OTMediaType_Audio:
			{
				pOTMixer = new OTMixerMgrAudio(oBridgeInfo);
				break;
			}
		case OTMediaType_Video:
			{
				pOTMixer = new OTMixerMgrVideo(oBridgeInfo);
				break;
			}
	}

	if(pOTMixer && !pOTMixer->isValid())
	{
		OTObjectSafeRelease(pOTMixer);
	}
	
	return pOTMixer;
}