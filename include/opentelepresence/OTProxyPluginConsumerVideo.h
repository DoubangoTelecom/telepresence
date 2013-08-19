/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_PLUGINCONSUMERVIDEO_H
#define OPENTELEPRESENCE_PLUGINCONSUMERVIDEO_H

#include "OpenTelepresenceConfig.h"
#include "opentelepresence/OTProxyPlugin.h"
#include "opentelepresence/OTFrameVideo.h"
#include "opentelepresence/filters/OTOverlay.h"

#include "opentelepresence/OTProxyPluginConsumer.h"

#include "tinydav/video/tdav_consumer_video.h"

class OTProxyPluginConsumerVideoVideoCallback;

class OTProxyPluginConsumerVideo : public OTProxyPluginConsumer
{
	friend class OTProxyPluginConsumerVideoVideoCallback;
public:
	OTProxyPluginConsumerVideo(uint64_t nId, const ProxyVideoConsumer* pcConsumer);
	virtual ~OTProxyPluginConsumerVideo();
	OT_INLINE virtual const char* getObjectId() { return "OTProxyPluginConsumerVideo"; }

public:
	OT_INLINE bool setDisplaySize(int nWidth, int nHeight){ 
		return m_pcWrappedConsumer ? const_cast<ProxyVideoConsumer*>(m_pcWrappedConsumer)->setDisplaySize(nWidth, nHeight) : false;
	}
	OT_INLINE bool reset(){ 
		return m_pcWrappedConsumer ? const_cast<ProxyVideoConsumer*>(m_pcWrappedConsumer)->reset() : false;
	}
	OT_INLINE int getFps() { return m_nFPS; }
	OT_INLINE int getHeight() { return m_nHeight; }
	OT_INLINE int getWidth() { return m_nWidth; }
	OT_INLINE uint32_t getHeldBufferSize() { return m_OTHeldFrameVideo ? m_OTHeldFrameVideo->getValidDataSize() : 0; }
	OT_INLINE OTObjectWrapper<OTFrameVideo *> getHeldFrameVideo(){ return m_OTHeldFrameVideo; }

	bool drawOverlay(bool bListener, 
		const void* pcInBufferPtr, size_t nInBufferSize, size_t nInWidth, size_t nInHeight, 
		size_t nOutWidth, size_t nOutHeight, void* pOutFrame
		);
	

private:
	int prepareCallback(int nWidth, int nHeight, int nFps);
	int startCallback();
	int consumeCallback(const ProxyVideoFrame* frame);
	int pauseCallback();
	int stopCallback();

private:
	OTProxyPluginConsumerVideoVideoCallback* m_pCallback;
	const ProxyVideoConsumer* m_pcWrappedConsumer;
	int32_t m_nWidth, m_nHeight, m_nFPS, m_nPtime;
	OTObjectWrapper<OTFrameVideo *>m_OTHeldFrameVideo;
	OTObjectWrapper<OTOverlayVideoText *> m_oOverlaySpeakerText;
	OTObjectWrapper<OTOverlayVideoWatermark *> m_oOverlaySpeakerWatermark;
	OTObjectWrapper<OTOverlayVideoText *> m_oOverlayListenerText;
};

//
//	OTProxyPluginConsumerVideoVideoCallback
//
class OTProxyPluginConsumerVideoVideoCallback : public ProxyVideoConsumerCallback {
public:
	OTProxyPluginConsumerVideoVideoCallback(OTObjectWrapper<OTProxyPluginConsumerVideo*>pConsumer)
	  : ProxyVideoConsumerCallback(), m_pConsumer(pConsumer){
	}
	virtual ~OTProxyPluginConsumerVideoVideoCallback(){
		OTObjectSafeRelease(m_pConsumer);
	}
public: /* Overrides */
	virtual int prepare(int width, int height, int fps) { 
		return m_pConsumer->prepareCallback(width, height, fps); 
	}
	virtual int start() { 
		return m_pConsumer->startCallback(); 
	}
	virtual int consume(const ProxyVideoFrame* frame) { 
		return m_pConsumer->consumeCallback(frame); 
	}
	virtual int pause() { 
		return m_pConsumer->pauseCallback(); 
	}
	virtual int stop() { 
		return m_pConsumer->stopCallback(); 
	}
private:
	OTObjectWrapper<OTProxyPluginConsumerVideo*> m_pConsumer;
};


#endif /* OPENTELEPRESENCE_PLUGINCONSUMERVIDEO_H */
