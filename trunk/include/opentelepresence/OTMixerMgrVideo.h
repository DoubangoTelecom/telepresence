/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_MIXERMGRVIDEO_H
#define OPENTELEPRESENCE_MIXERMGRVIDEO_H

#include "OpenTelepresenceConfig.h"
#include "opentelepresence/OTObject.h"
#include "opentelepresence/OTMixerMgr.h"
#include "opentelepresence/mixers/OTMixerVideo.h"
#include "opentelepresence/OTProxyPluginConsumerVideo.h"
#include "opentelepresence/OTProxyPluginProducerVideo.h"

#include "tsk_thread.h"
#include "tsk_condwait.h"

#include <map>

class OTMixerMgrVideo : public OTMixerMgr
{
public:
	OTMixerMgrVideo(OTObjectWrapper<OTBridgeInfo*> oBridgeInfo);
	~OTMixerMgrVideo();
	OT_INLINE virtual const char* getObjectId() { return "OTMixerMgrVideo"; }

	// OTMixerOTMixerMgr
	virtual int start();
	virtual bool isStarted();
	virtual int pause();
	virtual bool isPaused();
	virtual int flush();
	virtual int stop();
	virtual int attachMediaPlugins(OTObjectWrapper<OTSessionInfo*> pOTSessionInfo);
	virtual int deAttachMediaPlugins(OTObjectWrapper<OTSessionInfo*> pOTSessionInfo);

private:
	static int rtcpOnEventCb(const void* pcUsrData, enum tmedia_rtcp_event_type_e eEventType, uint32_t nSsrcMedia);
	static void* TSK_STDCALL pullThreadFunc(void *arg);
	void resetConsumersJitter();
	int mixAndSend();

protected:
	virtual bool isValid();

#if defined(_MSC_VER)
#	pragma warning( push )
#	pragma warning( disable : 4251 )
#endif

private:
	bool m_bValid;
	bool m_bStarted;
	bool m_bPaused;
	bool m_bRequestedIntraViaRtcp;

	uint64_t m_nLastTimeRTPTimeoutChecked;

	uint64_t m_nLastTimeIdrSent;

	void* m_pMixedBufferPtr;
	uint32_t m_nMixedBufferSize;

	
	tsk_mutex_handle_t *m_phProducersMutex;
	tsk_mutex_handle_t *m_phConsumersMutex;

	std::map<uint64_t, OTObjectWrapper<OTProxyPluginConsumerVideo*> > m_oConsumers;
	std::map<uint64_t, OTObjectWrapper<OTProxyPluginProducerVideo*> > m_oProducers;

	OTObjectWrapper<OTMixerVideo*> m_oMixerVideo;

	tsk_thread_handle_t* m_phPullThread[1];
	tsk_condwait_handle_t* m_phPullCond;

#if defined(_MSC_VER)
#	pragma warning( pop )
#endif
};

#endif /* OPENTELEPRESENCE_MIXERMGRVIDEO_H */
