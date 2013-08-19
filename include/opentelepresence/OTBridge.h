/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_BRIDGE_H
#define OPENTELEPRESENCE_BRIDGE_H

#include "OpenTelepresenceConfig.h"

#include "opentelepresence/OTBridgeInfo.h"
#include "opentelepresence/OTMixerMgrMgr.h"
#include "opentelepresence/cfg/OTCfg.h"

#include "tinydav/tdav.h"

#include "tsk_mutex.h"

#include <list>
#include <map>

class OTSipSessionAV;
class OTEngineInfo;

class OTBridge : public OTObject
{	
	friend class OTSipCallback;
public:
	OTBridge(std::string strId, OTObjectWrapper<OTEngineInfo*> oEngineInfo);
	virtual ~OTBridge();
	virtual OT_INLINE const char* getObjectId() { return "OTBridge"; }

	virtual OT_INLINE std::string getId(){ return  m_strId; }
	virtual int start();
	virtual OT_INLINE bool isRunning(){ return m_bRunning; }
	virtual int stop();
	virtual int hangUpSession(uint64_t nSessionId);
	virtual int addAVCall(OTObjectWrapper<OTSipSessionAV*> oAVCall);
	virtual int removeAVCall(uint64_t nSessionId);
	virtual OTObjectWrapper<OTSipSessionAV*> findCallByUserId(std::string strUserId);
	virtual OTObjectWrapper<OTSipSessionAV*> findCallBySessionId(uint64_t nSessionId);
	virtual size_t getNumberOfActiveAVCalls();
	virtual OT_INLINE OTObjectWrapper<OTBridgeInfo*> getInfo(){ return m_oInfo; }

protected:
	virtual OT_INLINE bool isValid(){ return m_bValid; }

#if defined(_MSC_VER)
#	pragma warning( push )
#	pragma warning( disable : 4251 )
#endif

private:
	OTObjectWrapper<OTBridgeInfo*> m_oInfo;
	std::string m_strId;
	bool m_bValid;
	bool m_bRunning;

	std::map<uint64_t, OTObjectWrapper<OTSipSessionAV*> > m_oAVCalls;
	OTObjectWrapper<OTMixerMgrMgr*> m_oMixerMgrMgr;
	OTMapOfCfgParams m_oMapOfCfgParams;

#if defined(_MSC_VER)
#	pragma warning( pop )
#endif

};

#if defined(_MSC_VER)
template class OTObjectWrapper<OTBridge*>;
#endif

#endif /* OPENTELEPRESENCE_BRIDGE_H */
