/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_PLUGINCONSUMERAUDIO_H
#define OPENTELEPRESENCE_PLUGINCONSUMERAUDIO_H

#include "OpenTelepresenceConfig.h"
#include "opentelepresence/OTProxyPlugin.h"

#include "opentelepresence/OTProxyPluginConsumer.h"

#include "tinydav/audio/tdav_consumer_audio.h"

class OTProxyPluginConsumerAudioCallback;

//
//	OTProxyPluginConsumerAudio
//
class OTProxyPluginConsumerAudio : public OTProxyPluginConsumer
{
	friend class OTProxyPluginConsumerAudioCallback;
public:
	OTProxyPluginConsumerAudio(uint64_t nId, const ProxyAudioConsumer* pcConsumer);
	virtual ~OTProxyPluginConsumerAudio();

	OT_INLINE unsigned pull(void* output, unsigned size){
		return (m_pcWrappedConsumer && m_bStarted) ? const_cast<ProxyAudioConsumer*>(m_pcWrappedConsumer)->pull(output, size) : 0;
	}
	unsigned pullAndHold();
	unsigned copyFromHeldBuffer(void* output, unsigned size);
	OT_INLINE bool reset(){
		m_nRTPPktPullCount = m_nRTPPktPullTime = 0;
		return m_pcWrappedConsumer ? const_cast<ProxyAudioConsumer*>(m_pcWrappedConsumer)->reset() : false;
	}
	OT_INLINE int getPtime() { return m_nPtime; }
	OT_INLINE int getRate() { return m_nRate; }
	OT_INLINE int getChannels() { return m_nChannels; }
	OT_INLINE double getVolumeAvg() { return m_nVolumeAvg; }
	OT_INLINE double getVolumeNow() { return m_nVolumeNow; }

private:
	int prepareCallback(int ptime, int rate, int channels);
	int startCallback();
	int pauseCallback();
	int stopCallback();

private:
	OTProxyPluginConsumerAudioCallback* m_pCallback;
	const ProxyAudioConsumer* m_pcWrappedConsumer;
	int m_nPtime;
	int m_nRate;
	int m_nChannels;
	int m_nBitsPerSample;
	void* m_pHeldBufferPtr;
	double m_nVolumeNow;
	double m_nVolumeAvg;
	uint32_t m_nMaxLatency;
	uint32_t m_nVolumeComputeCount;
	uint32_t m_nHeldBufferSizeInBytes;
	uint32_t m_nHeldBufferSizeInSamples;
	uint32_t m_nHeldBufferPos;

	uint64_t m_nRTPPktPullCount;
	uint64_t m_nRTPPktPullTime;

	bool m_bSettingsChanged;
};

//
//	OTProxyPluginConsumerAudioCallback
//
class OTProxyPluginConsumerAudioCallback : public ProxyAudioConsumerCallback {
public:
	OTProxyPluginConsumerAudioCallback(OTObjectWrapper<OTProxyPluginConsumerAudio*>pConsumer)
	  : ProxyAudioConsumerCallback(), m_pConsumer(pConsumer){
	}
	virtual ~OTProxyPluginConsumerAudioCallback(){
		OTObjectSafeRelease(m_pConsumer);
	}
public: /* Overrides */
	virtual int prepare(int ptime, int rate, int channels) { 
		return m_pConsumer->prepareCallback(ptime, rate, channels); 
	}
	virtual int start() { 
		return m_pConsumer->startCallback();
	}
	virtual int pause() { 
		return m_pConsumer->pauseCallback();
	}
	virtual int stop() { 
		return m_pConsumer->stopCallback();
	}
	virtual bool isPivotSettings() { 
		return true; // Do not override PIVOT settings
	}
private:
	OTObjectWrapper<OTProxyPluginConsumerAudio*> m_pConsumer;
};


#endif /* OPENTELEPRESENCE_PLUGINCONSUMERAUDIO_H */
