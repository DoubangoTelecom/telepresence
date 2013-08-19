/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_PROXYPLUGINMGR_H
#define OPENTELEPRESENCE_PROXYPLUGINMGR_H

#include "OpenTelepresenceConfig.h"
#include "opentelepresence/OTProxyPlugin.h"

#include "tsk_mutex.h"

#include <map>

class OTProxyPluginMgrCallback;
typedef std::map<uint64_t, OTObjectWrapper<OTProxyPlugin*> > OTMapOfPlugins;

//
//	OTProxyPluginMgr
//
class OTProxyPluginMgr
{
private:
	static ProxyPluginMgr* g_pPluginMgr;
	static OTProxyPluginMgrCallback* g_pPluginMgrCallback;
	static OTMapOfPlugins* g_pPlugins;
	static tsk_mutex_handle_t* g_phMutex;
	static bool g_bInitialized;

public:
	static void initialize();
	static void deInitialize();
	
	static inline OTObjectWrapper<OTProxyPlugin*> findPlugin(uint64_t nId)
	{
		OTMapOfPlugins::iterator it = OTProxyPluginMgr::g_pPlugins->find(nId);
		OTObjectWrapper<OTProxyPlugin*> pPlugin = NULL;
		if(it != OTProxyPluginMgr::g_pPlugins->end())
		{
			pPlugin = it->second;
		}
		return pPlugin;
	}

	static inline void erasePlugin(uint64_t nId){
		OTMapOfPlugins::iterator it;
		if((it = OTProxyPluginMgr::g_pPlugins->find(nId)) != OTProxyPluginMgr::g_pPlugins->end())
		{
			OTObjectWrapper<OTProxyPlugin*> pPlugin = it->second;
			OTProxyPluginMgr::g_pPlugins->erase(it);
			pPlugin->invalidate();
			pPlugin->releaseRef(); 
		}
	}

	static inline OTMapOfPlugins* getPlugins()
	{
		return OTProxyPluginMgr::g_pPlugins;
	}
	
	static inline tsk_mutex_handle_t* getMutex()
	{
		return OTProxyPluginMgr::g_phMutex;
	}

	static inline ProxyPluginMgr* getPluginMgr()
	{
		return const_cast<ProxyPluginMgr* > (OTProxyPluginMgr::g_pPluginMgr);
	}
};

//
//	OTProxyPluginMgrCallback
//
class OTProxyPluginMgrCallback : public ProxyPluginMgrCallback
{
public:
	OTProxyPluginMgrCallback();
	virtual ~OTProxyPluginMgrCallback();

public: /* override */
	virtual int OnPluginCreated(uint64_t id, enum twrap_proxy_plugin_type_e type);
	virtual int OnPluginDestroyed(uint64_t id, enum twrap_proxy_plugin_type_e type);
};

#endif /* OPENTELEPRESENCE_PROXYPLUGINMGR_H */
