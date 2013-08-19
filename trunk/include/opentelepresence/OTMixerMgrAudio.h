/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_MIXERMGRAUDIO_H
#define OPENTELEPRESENCE_MIXERMGRAUDIO_H

#include "OpenTelepresenceConfig.h"
#include "opentelepresence/OTObject.h"
#include "opentelepresence/OTMixerMgr.h"
#include "opentelepresence/mixers/OTMixerAudio.h"
#include "opentelepresence/OTProxyPluginConsumerAudio.h"
#include "opentelepresence/OTProxyPluginProducerAudio.h"

#include "tsk_mutex.h"
#include "tsk_thread.h"
#include "tsk_condwait.h"

#include <map>

class OTMixerMgrAudio : public OTMixerMgr
{
public:
	OTMixerMgrAudio(OTObjectWrapper<OTBridgeInfo*> oBridgeInfo);
	~OTMixerMgrAudio();
	virtual OT_INLINE const char* getObjectId() { return "OTMixerMgrAudio"; }

	// OTMixerOTMixerMgr
	virtual int start();
	virtual bool isStarted();
	virtual int pause();
	virtual bool isPaused();
	virtual int flush();
	virtual int stop();
	virtual int attachMediaPlugins(OTObjectWrapper<OTSessionInfo*> pOTSessionInfo);
	virtual int deAttachMediaPlugins(OTObjectWrapper<OTSessionInfo*> pOTSessionInfo);

protected:
	virtual bool isValid();

private:
	static void* TSK_STDCALL pullThreadFunc(void *arg);
	void resetConsumersJitter();
	int mixAndSend();

#if defined(_MSC_VER)
#	pragma warning( push )
#	pragma warning( disable : 4251 )
#endif

private:
	bool m_bValid;
	bool m_bStarted;
	bool m_bPaused;
	
	tsk_mutex_handle_t *m_phProducersMutex;
	tsk_mutex_handle_t *m_phConsumersMutex;

	std::map<uint64_t, OTObjectWrapper<OTProxyPluginConsumerAudio*> > m_oConsumers;
	std::map<uint64_t, OTObjectWrapper<OTProxyPluginProducerAudio*> > m_oProducers;

	OTObjectWrapper<OTMixerAudio*> m_oMixerAudio;

	uint64_t m_nLastTimerPull;
	tsk_thread_handle_t* m_phPullThread[1];
	tsk_condwait_handle_t* m_phPullCond;
	
#if defined(_MSC_VER)
#	pragma warning( pop )
#endif

};

#endif /* OPENTELEPRESENCE_MIXERMGRAUDIO_H */
