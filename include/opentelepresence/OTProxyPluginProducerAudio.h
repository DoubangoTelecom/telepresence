/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_PLUGINPRODUCERAUDIO_H
#define OPENTELEPRESENCE_PLUGINPRODUCERAUDIO_H

#include "OpenTelepresenceConfig.h"
#include "opentelepresence/OTProxyPluginProducer.h"
#include "opentelepresence/OTFrameAudio.h"

#include "tsk_semaphore.h"
#include "tsk_mutex.h"
#include "tsk_buffer.h"

class OTProxyPluginProducerAudioCallback;

//
//	OTProxyPluginProducerAudio
//
class OTProxyPluginProducerAudio : public OTProxyPluginProducer
{
	friend class OTProxyPluginProducerAudioCallback;
public:
	OTProxyPluginProducerAudio(uint64_t nId, const ProxyAudioProducer* pcProducer);
	virtual ~OTProxyPluginProducerAudio();
	int push(const void* buffer, unsigned size);
	OT_INLINE int getPtime() { return m_nPtime; }
	OT_INLINE int getRate() { return m_nRate; }
	OT_INLINE int getChannels() { return m_nChannels; }

	static void* TSK_STDCALL senderThread(void *arg);

private:
	int prepareCallback(int ptime, int rate, int channels);
	int startCallback();
	int pauseCallback();
	int stopCallback();

private:
	OTProxyPluginProducerAudioCallback* m_pCallback;
	const ProxyAudioProducer* m_pcWrappedProducer;
	void* m_phSenderThread[1];
	tsk_semaphore_handle_t *m_phSenderSemaphore;
	OTObjectWrapper<OTFrameAudio *> m_oFrame;
	int m_nPtime;
	int m_nRate;
	int m_nChannels;
	int m_nBitsPerSample;
};


//
//	OTProxyPluginProducerAudioCallback
//
class OTProxyPluginProducerAudioCallback : public ProxyAudioProducerCallback {
public:
	OTProxyPluginProducerAudioCallback(OTObjectWrapper<OTProxyPluginProducerAudio*>pOTProducer)
	  : ProxyAudioProducerCallback(), m_pOTProducer(pOTProducer){
	}
	virtual ~OTProxyPluginProducerAudioCallback(){
		OTObjectSafeRelease(m_pOTProducer);
	}
public: /* Overrides */
	virtual int prepare(int ptime, int rate, int channels) { 
		return m_pOTProducer->prepareCallback(ptime, rate, channels); 
	}
	virtual int start() { 
		return m_pOTProducer->startCallback(); 
	}
	virtual int pause() { 
		return m_pOTProducer->pauseCallback(); 
	}
	virtual int stop() { 
		return m_pOTProducer->stopCallback(); 
	}
private:
	OTObjectWrapper<OTProxyPluginProducerAudio*> m_pOTProducer;
};

#endif /* OPENTELEPRESENCE_PLUGINPRODUCERAUDIO_H */
