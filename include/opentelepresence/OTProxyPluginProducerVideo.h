/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_PLUGINPRODUCERVIDEO_H
#define OPENTELEPRESENCE_PLUGINPRODUCERVIDEO_H

#include "OpenTelepresenceConfig.h"
#include "opentelepresence/OTProxyPlugin.h"
#include "opentelepresence/OTFrameVideo.h"

#include "opentelepresence/OTProxyPluginProducer.h"

class OTProxyPluginProducerVideoCallback;

//
//	OTProxyPluginProducerVideo
//
class OTProxyPluginProducerVideo : public OTProxyPluginProducer
{
	friend class OTProxyPluginProducerVideoCallback;
public:
	OTProxyPluginProducerVideo(uint64_t nId, const ProxyVideoProducer* pcProducer);
	virtual ~OTProxyPluginProducerVideo();

	// Encode then send
	int send(OTObjectWrapper<OTFrameVideo *> pOTOTFrameVideo);
	// Send "AS IS"
	int sendRaw(const void* pcBuffer, unsigned nSize, unsigned nDuration, bool bMarker){
		return (m_pcWrappedProducer && isStarted()) ? const_cast<ProxyVideoProducer*>(m_pcWrappedProducer)->sendRaw(pcBuffer, nSize, nDuration, bMarker) : 0;
	}
	bool setActualCameraOutputSize(unsigned nWidth, unsigned nHeight){
		return (m_pcWrappedProducer && isStarted()) ? const_cast<ProxyVideoProducer*>(m_pcWrappedProducer)->setActualCameraOutputSize(nWidth, nHeight) : 0;
	}
	
	OT_INLINE int getFps() { return m_nFPS; }
	OT_INLINE int getHeight() { return m_nHeight; }
	OT_INLINE int getWidth() { return m_nWidth; }

private:
	int prepareCallback(int nWidth, int nHeight, int nFps);
	int startCallback();
	int pauseCallback();
	int stopCallback();

	static void* TSK_STDCALL senderThread(void *arg);

private:
	OTProxyPluginProducerVideoCallback* m_pCallback;
	const ProxyVideoProducer* m_pcWrappedProducer;
	int m_nWidth, m_nHeight, m_nFPS;
	void* m_phSenderThread[1];
	tsk_semaphore_handle_t *m_phSenderSemaphore;
	tsk_mutex_handle_t *m_phSenderMutex;
	tsk_buffer_t *m_pSenderBuffer;
	tsk_size_t m_nSenderBufferLimit;
};


//
//	OTProxyPluginProducerVideoCallback
//
class OTProxyPluginProducerVideoCallback : public ProxyVideoProducerCallback {
public:
	OTProxyPluginProducerVideoCallback(OTObjectWrapper<OTProxyPluginProducerVideo*>pOTProducer)
	  : ProxyVideoProducerCallback(), m_pOTProducer(pOTProducer){
	}
	virtual ~OTProxyPluginProducerVideoCallback(){
		OTObjectSafeRelease(m_pOTProducer);
	}
public: /* Overrides */
	virtual int prepare(int width, int height, int fps) { 
		return m_pOTProducer->prepareCallback(width, height, fps); 
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
	OTObjectWrapper<OTProxyPluginProducerVideo*> m_pOTProducer;
};


#endif /* OPENTELEPRESENCE_PLUGINPRODUCERVIDEO_H */
