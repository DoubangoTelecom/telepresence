/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_PLUGIN_H
#define OPENTELEPRESENCE_PLUGIN_H

#include "OpenTelepresenceConfig.h"
#include "opentelepresence/OTObject.h"
#include "opentelepresence/OTCommon.h"
#include "opentelepresence/OTSessionInfo.h"

#include "ProxyPluginMgr.h"

#include "tsk_mutex.h"

class OTProxyPlugin : public OTObject
{
protected:
	OTProxyPlugin(OTMediaType_t eMediaType, uint64_t nId, const ProxyPlugin* pcProxyPlugin);

public:
	virtual ~OTProxyPlugin();
	OT_INLINE virtual const char* getObjectId() { return "OTProxyPlugin"; }
	OT_INLINE bool operator ==(const OTProxyPlugin& other) const{
		return m_nId == other.m_nId;
	}
	OT_INLINE virtual uint64_t getId() { return m_nId; }
	OT_INLINE virtual OTMediaType_t getMediaType() { return m_eMediaType; }
	OT_INLINE virtual void invalidate(){ m_bValid = false; }
	OT_INLINE virtual bool isValid() { return m_bValid; }
	OT_INLINE virtual bool isPrepared() { return m_bPrepared; }
	OT_INLINE virtual bool isPaused() { return m_bPaused; }
	OT_INLINE virtual bool isStarted() { return m_bStarted; }

	OT_INLINE virtual bool lock() { 
		if(m_phMutex){
			return (tsk_mutex_lock(m_phMutex) == 0);
		}
		return false;
	}
	OT_INLINE virtual bool unLock() { 
		if(m_phMutex){
			return (tsk_mutex_unlock(m_phMutex) == 0);
		}
		return false;
	}

	OT_INLINE virtual void setSessionInfo(OTObjectWrapper<OTSessionInfo*> oSessionInfo) { m_oSessionInfo = oSessionInfo; }
	OT_INLINE OTObjectWrapper<OTSessionInfo*> getSessionInfo(){ return m_oSessionInfo; }

	// short path (to avoid using getSessionInfo() which create a copy)
	// these functions will be used inside delay-sensitive blocks
	OT_INLINE uint64_t getSessionInfoProducerId(OTMediaType_t eMediaType){ return m_oSessionInfo->getProducerId(eMediaType); }
	OT_INLINE uint64_t getSessionInfoConsumerId(OTMediaType_t eMediaType){ return m_oSessionInfo->getConsumerId(eMediaType); }
	OT_INLINE void setSessionInfoSpeaker(bool bSpeaker) { m_oSessionInfo->setSpeaker(bSpeaker); }
	OT_INLINE void setSessionInfoSpeaking(bool bSpeaking) { m_oSessionInfo->setSpeaking(bSpeaking); }

protected:
	OT_INLINE int prepare() { m_bPrepared = true; return 0; }
	OT_INLINE int start() { m_bStarted = true; return 0; }
	OT_INLINE int pause() { m_bPaused = true; return 0; }
	OT_INLINE int stop() { m_bStarted = false; return 0; }

protected:
	bool m_bValid;
	bool m_bPrepared;
	bool m_bStarted;
	bool m_bPaused;
	uint64_t m_nId;
	OTMediaType_t m_eMediaType;
	const ProxyPlugin* m_pcProxyPlugin;
	OTObjectWrapper<OTSessionInfo*> m_oSessionInfo;
	tsk_mutex_handle_t *m_phMutex;
	
};

#endif /* OPENTELEPRESENCE_PLUGIN_H */
