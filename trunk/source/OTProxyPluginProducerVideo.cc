/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#include "opentelepresence/OTProxyPluginProducerVideo.h"

OTProxyPluginProducerVideo::OTProxyPluginProducerVideo(uint64_t nId, const ProxyVideoProducer* pcProducer)
: OTProxyPluginProducer(OTMediaType_Video, nId, dynamic_cast<const ProxyPlugin*>(pcProducer)),
m_nWidth(OPENTELEPRESENCE_VIDEO_WIDTH_DEFAULT), m_nHeight(OPENTELEPRESENCE_VIDEO_HEIGHT_DEFAULT), m_nFPS(OPENTELEPRESENCE_VIDEO_FPS_DEFAULT)
{
	m_phSenderThread[0] = tsk_null;
	m_phSenderSemaphore = tsk_null;
	m_pcWrappedProducer = pcProducer;
	m_nSenderBufferLimit = 0;
	m_phSenderMutex = tsk_null;
	m_pSenderBuffer = tsk_null;
	m_pcWrappedProducer = pcProducer;
	m_pCallback = new OTProxyPluginProducerVideoCallback(this);
	const_cast<ProxyVideoProducer*>(m_pcWrappedProducer)->setCallback(m_pCallback);
}

OTProxyPluginProducerVideo::~OTProxyPluginProducerVideo()
{
	OT_DEBUG_INFO("*** OTProxyPluginProducerVideo destroyed ***");
	m_pcWrappedProducer = NULL;
	if(m_pCallback)
	{
		delete m_pCallback, m_pCallback = NULL;
	}
	if(m_phSenderThread[0])
	{
		tsk_thread_join(&(m_phSenderThread[0]));
	}
	if(m_phSenderSemaphore)
	{
		tsk_semaphore_destroy(&m_phSenderSemaphore);
	}
	if(m_phSenderMutex)
	{
		tsk_mutex_destroy(&m_phSenderMutex);
	}
	TSK_OBJECT_SAFE_FREE(m_pSenderBuffer);
}

int OTProxyPluginProducerVideo::prepareCallback(int nWidth, int nHeight, int nFps)
{
	m_nWidth = nWidth;
	m_nHeight = nHeight;
	m_nFPS = nFps;

	// call parent
	return OTProxyPlugin::prepare();
}

int OTProxyPluginProducerVideo::startCallback()
{
	// create thread if not already done
	if(!m_phSenderSemaphore && !(m_phSenderSemaphore = tsk_semaphore_create()))
	{
		OT_DEBUG_ERROR("Failed to create semaphore");
		return -1;
	}

	// create mutex
	if(!m_phSenderMutex && !(m_phSenderMutex = tsk_mutex_create_2(tsk_false)))
	{
		OT_DEBUG_ERROR("Failed to create mutex");
		return -1;
	}

	// create buffer
	if(!m_pSenderBuffer && !(m_pSenderBuffer = tsk_buffer_create_null()))
	{
		OT_DEBUG_ERROR("Failed to create buffer");
		return -1;
	}

	// create thread if not already done
	if(!m_phSenderThread[0])
	{
		int ret = tsk_thread_create(&m_phSenderThread[0], OTProxyPluginProducerVideo::senderThread, this);
		if(ret)
		{
			OT_DEBUG_ERROR("tsk_thread_create failed with error code = %d", ret);
			return ret;
		}
	}
	
	// call parent
	return OTProxyPlugin::start();
}

int OTProxyPluginProducerVideo::pauseCallback()
{

	// call parent
	return OTProxyPlugin::pause();
}

int OTProxyPluginProducerVideo::stopCallback()
{
	int ret;

	// call parent
	if(ret = OTProxyPlugin::stop())
	{
		OT_DEBUG_ERROR("Failed to stop producer");
		return ret;
	}
	// now isStarted() will return false which means that the thread will exit
	if(m_phSenderSemaphore){
		if((ret = tsk_semaphore_increment(m_phSenderSemaphore)))
		{
			OT_DEBUG_ERROR("tsk_semaphore_increment() failed with error code=%d", ret);
			return ret;
		}
	}
	if(m_phSenderThread[0]){
		if((ret = tsk_thread_join(&(m_phSenderThread[0]))))
		{
			OT_DEBUG_ERROR("tsk_thread_join() failed with error code=%d", ret);
			return ret;
		}
	}
	

	return ret;
}

int OTProxyPluginProducerVideo::send(OTObjectWrapper<OTFrameVideo *> pOTOTFrameVideo)
{
	int ret = 0;

	if(!m_pSenderBuffer || !m_phSenderMutex || !m_phSenderSemaphore){
		OT_DEBUG_ERROR("Invalid state");
		return -1;
	}
	if(!pOTOTFrameVideo || !pOTOTFrameVideo->isValid())
	{
		OT_DEBUG_ERROR("Invalid parameter");
		return -1;
	}
	if(m_pcWrappedProducer && isValid())
	{
		ProxyVideoProducer* pWrappedProducer = const_cast<ProxyVideoProducer*>(m_pcWrappedProducer);
		pWrappedProducer->setActualCameraOutputSize(pOTOTFrameVideo->getWidth(), pOTOTFrameVideo->getHeight());

		tsk_mutex_lock(m_phSenderMutex);

		if((ret = tsk_buffer_copy(m_pSenderBuffer, 0, pOTOTFrameVideo->getBufferPtr(), pOTOTFrameVideo->getValidDataSize())))
		{
			OT_DEBUG_ERROR("tsk_buffer_copy() failed with error code=%d", ret);
			goto done;
		}
		m_nSenderBufferLimit = pOTOTFrameVideo->getValidDataSize();

		if((ret = tsk_semaphore_increment(m_phSenderSemaphore))){
			OT_DEBUG_ERROR("tsk_semaphore_increment() failed with error code=%d", ret);
			goto done;
		}

done:
		tsk_mutex_unlock(m_phSenderMutex);
		return ret;
	}
	OT_DEBUG_ERROR("Invalid wrapped plugin");
	return ret;
}

void* TSK_STDCALL OTProxyPluginProducerVideo::senderThread(void *arg)
{
	OTProxyPluginProducerVideo *This = (OTProxyPluginProducerVideo*)arg;

	while(This->isStarted() && This->m_pcWrappedProducer && This->m_phSenderMutex)
	{
		tsk_semaphore_decrement(This->m_phSenderSemaphore);
		if(!This->isStarted() || !This->m_pcWrappedProducer || !This->m_phSenderMutex)
		{
			break;
		}
		
		tsk_mutex_lock(This->m_phSenderMutex);
		const_cast<ProxyVideoProducer*>(This->m_pcWrappedProducer)->push(TSK_BUFFER_DATA(This->m_pSenderBuffer), TSK_BUFFER_SIZE(This->m_pSenderBuffer));
		tsk_mutex_unlock(This->m_phSenderMutex);
	}

	OT_DEBUG_INFO("OTProxyPluginProducerAudio::SenderThread::Exit");
	
	return tsk_null;
}
