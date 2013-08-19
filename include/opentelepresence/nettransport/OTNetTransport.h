/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_NET_TRANSPORT_H
#define OPENTELEPRESENCE_NET_TRANSPORT_H

#include "OpenTelepresenceConfig.h"
#include "opentelepresence/OTObject.h"
#include "opentelepresence/OTCommon.h"
#include "opentelepresence/OTMutex.h"

#include "tnet_transport.h"

#include <map>
#include <string>

class OTNetTransport;
class OTNetTransportCallback;

//
//	OTNetPeer
//
class OTNetPeer : public OTObject
{
	friend class OTNetTransport;
	friend class OTNetTransportCallback;
public:
	OTNetPeer(OTNetFd nFd, bool bConnected = false, const void* pcData = NULL, size_t nDataSize = 0)
	{
		m_bConnected = bConnected;
		m_nFd = nFd;
		m_pWrappedBuffer = tsk_buffer_create(pcData, nDataSize);
		m_bRawContent = false;
	}
	virtual ~OTNetPeer()
	{
		TSK_OBJECT_SAFE_FREE(m_pWrappedBuffer);
	}
	virtual OT_INLINE OTNetFd getFd(){ return  m_nFd; }
	virtual OT_INLINE bool isConnected(){ return  m_bConnected; }
	virtual OT_INLINE const void* getDataPtr() { return m_pWrappedBuffer ? m_pWrappedBuffer->data : NULL; }
	virtual OT_INLINE size_t getDataSize() { return m_pWrappedBuffer ? m_pWrappedBuffer->size : 0; }
	virtual OT_INLINE bool isRawContent(){ return  m_bRawContent; }
	virtual OT_INLINE void setRawContent(bool bRawContent){ m_bRawContent = bRawContent; }
	virtual bool sendData(const void* pcDataPtr, size_t nDataSize);
	virtual OT_INLINE bool isStream() = 0;

protected:
	virtual OT_INLINE void setConnected(bool bConnected) { m_bConnected = bConnected; }

protected:
	bool m_bConnected;
	bool m_bRawContent;
	OTNetFd m_nFd;
	tsk_buffer_t* m_pWrappedBuffer;
};


//
//	OTNetPeerDgram
//
class OTNetPeerDgram : public OTNetPeer
{
public:
	OTNetPeerDgram(OTNetFd nFd, const void* pcData = NULL, size_t nDataSize = 0)
		:OTNetPeer(nFd, false, pcData, nDataSize)
	{
	}
	virtual ~OTNetPeerDgram()
	{
	}
	virtual OT_INLINE const char* getObjectId() { return "OTNetPeerDgram"; }
	virtual OT_INLINE bool isStream(){ return false; }
};

//
//	OTNetPeerStream
//
class OTNetPeerStream : public OTNetPeer
{
public:
	OTNetPeerStream(OTNetFd nFd, bool bConnected = false, const void* pcData = NULL, size_t nDataSize = 0)
		:OTNetPeer(nFd, bConnected, pcData, nDataSize)
	{
	}
	virtual ~OTNetPeerStream()
	{
	}
	virtual OT_INLINE const char* getObjectId() { return "OTNetPeerStream"; }
	virtual OT_INLINE bool isStream(){ return true; }
	virtual OT_INLINE bool appenData(const void* pcData, size_t nDataSize){ return m_pWrappedBuffer ? tsk_buffer_append(m_pWrappedBuffer, pcData, nDataSize) == 0 : false; }
	virtual OT_INLINE bool remoteData(size_t nPosition, size_t nSize){ return m_pWrappedBuffer ? tsk_buffer_remove(m_pWrappedBuffer, nPosition, nSize) == 0 : false; }
	virtual OT_INLINE bool cleanupData(){ return m_pWrappedBuffer ? tsk_buffer_cleanup(m_pWrappedBuffer) == 0 : false; }
};

//
//	OTNetTransport
//
class OTNetTransportCallback : public OTObject
{
public:
	OTNetTransportCallback()
	{
	}
	virtual ~OTNetTransportCallback()
	{
	}
	virtual bool onData(OTObjectWrapper<OTNetPeer*> oPeer, size_t &nConsumedBytes) = 0;
	virtual bool onConnectionStateChanged(OTObjectWrapper<OTNetPeer*> oPeer) = 0;
};

//
//	OTNetTransport
//
class OTNetTransport : public OTObject
{
protected:
	OTNetTransport(OTNetTransporType_t eType, const char* pcLocalIP, unsigned short nLocalPort);
public:
	virtual ~OTNetTransport();
	virtual OT_INLINE OTNetTransporType_t getType() { return m_eType; }
	virtual OT_INLINE bool isStarted(){ return m_bStarted; }
	virtual OT_INLINE bool isValid(){ return m_bValid; }
	virtual OT_INLINE void setCallback(OTObjectWrapper<OTNetTransportCallback*> oCallback) { m_oCallback = oCallback; }

	virtual bool setSSLCertificates(const char* pcPrivateKey, const char* pcPublicKey, const char* pcCA, bool bVerify = false);
	virtual bool start();
	virtual OTNetFd connectTo(const char* pcHost, unsigned short nPort);
	virtual bool isConnected(OTNetFd nFd);
	virtual bool sendData(OTNetFd nFdFrom, const void* pcDataPtr, size_t nDataSize);
	virtual bool close(OTNetFd nFd);
	virtual bool stop();

private:
	OTObjectWrapper<OTNetPeer*> getPeerByFd(OTNetFd nFd);
	void insertPeer(OTObjectWrapper<OTNetPeer*> oPeer);
	void removePeer(OTNetFd nFd);
	static int OTNetTransportCb_Stream(const tnet_transport_event_t* e);

protected:
	tnet_transport_handle_t* m_pWrappedTransport;
	OTNetTransporType_t m_eType;
	bool m_bValid, m_bStarted;
	std::map<OTNetFd, OTObjectWrapper<OTNetPeer*> > m_Peers;
	OTObjectWrapper<OTNetTransportCallback*> m_oCallback;
	OTObjectWrapper<OTMutex*> m_oPeersMutex;
};


#endif /* OPENTELEPRESENCE_NET_TRANSPORT_H */
