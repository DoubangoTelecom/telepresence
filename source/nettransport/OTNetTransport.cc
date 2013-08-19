/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#include "opentelepresence/nettransport/OTNetTransport.h"

#include "tsk_debug.h"

#include <assert.h>

#if !defined(kOTMaxStreamBufferSize)
#	define kOTMaxStreamBufferSize 0xFFFF
#endif

//
//	OTNetPeer
//

// IMPORTANT: data sent using this function will never be encrypted
bool OTNetPeer::sendData(const void* pcDataPtr, size_t nDataSize)
{
	if(!pcDataPtr || !nDataSize)
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameNetTransport, "Invalid parameter");
		return false;
	}
	return (tnet_sockfd_send(getFd(), pcDataPtr, nDataSize, 0) == nDataSize);
}

//
//	OTNetTransport
//
OTNetTransport::OTNetTransport(OTNetTransporType_t eType, const char* pcLocalIP, unsigned short nLocalPort)
: m_bValid(false)
, m_bStarted(false)
{
	m_eType = eType;
	const char *pcDescription;
	tnet_socket_type_t eSocketType;
	bool bIsIPv6 = false;
	
	if(pcLocalIP && nLocalPort)
	{
		bIsIPv6 = (tnet_get_family(pcLocalIP, nLocalPort) == AF_INET6);
	}

	switch(eType)
	{
		case OTNetTransporType_TCP:
			{
				pcDescription = bIsIPv6 ? "TCP/IPv6 transport" : "TCP/IPv4 transport";
				eSocketType = bIsIPv6 ? tnet_socket_type_tcp_ipv6 : tnet_socket_type_tcp_ipv4;
				break;
			}
		case OTNetTransporType_TLS:
			{
				pcDescription = bIsIPv6 ? "TLS/IPv6 transport" : "TLS/IPv4 transport";
				eSocketType = bIsIPv6 ? tnet_socket_type_tls_ipv6 : tnet_socket_type_tls_ipv4;
				break;
			}
		default:
			{
				OT_ASSERT(false);
				return;
			}
	}

	if((m_pWrappedTransport = tnet_transport_create(pcLocalIP, nLocalPort, eSocketType, pcDescription)))
	{
		if(TNET_SOCKET_TYPE_IS_STREAM(eSocketType))
		{
			tnet_transport_set_callback(m_pWrappedTransport, OTNetTransport::OTNetTransportCb_Stream, this);
		}
		else
		{
			OT_ASSERT(false);
			return;
		}
	}

	m_oPeersMutex = new OTMutex();
		
	m_bValid = (m_oPeersMutex && m_pWrappedTransport);
}

OTNetTransport::~OTNetTransport()
{
	stop();
	TSK_OBJECT_SAFE_FREE(m_pWrappedTransport);
	OT_DEBUG_INFO("*** OTNetTransport destroyed ***");
}

bool OTNetTransport::setSSLCertificates(const char* pcPrivateKey, const char* pcPublicKey, const char* pcCA, bool bVerify /*= false*/)
{
	return (tnet_transport_tls_set_certs(m_pWrappedTransport, pcCA, pcPublicKey, pcPrivateKey, (bVerify ? tsk_true : tsk_false)) == 0);
}

bool OTNetTransport::start()
{
	m_bStarted = (tnet_transport_start(m_pWrappedTransport) == 0);
	return m_bStarted;
}

OTNetFd OTNetTransport::connectTo(const char* pcHost, unsigned short nPort)
{
	if(!pcHost || !nPort)
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameNetTransport, "Invalid parameter");
		return TNET_INVALID_FD;
	}
	if(!isValid())
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameNetTransport, "Transport not valid");
		return TNET_INVALID_FD;
	}
	if(!isStarted())
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameNetTransport, "Transport not started");
		return TNET_INVALID_FD;
	}

	return tnet_transport_connectto_2(m_pWrappedTransport, pcHost, nPort);
}

bool OTNetTransport::isConnected(OTNetFd nFd)
{
	OTObjectWrapper<OTNetPeer*> oPeer = getPeerByFd(nFd);
	return (oPeer && oPeer->isConnected());
}

bool OTNetTransport::sendData(OTNetFd nFdFrom, const void* pcDataPtr, size_t nDataSize)
{
	if(!pcDataPtr || !nDataSize || !OTNetFd_IsValid(nFdFrom))
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameNetTransport, "Invalid parameter");
		return false;
	}
	return (tnet_transport_send(m_pWrappedTransport, nFdFrom, pcDataPtr, nDataSize) == nDataSize);
}

bool OTNetTransport::close(OTNetFd nFd)
{
	return (tnet_transport_remove_socket(m_pWrappedTransport, &nFd) == 0);
}

bool OTNetTransport::stop()
{
	m_bStarted = false;
	return (tnet_transport_shutdown(m_pWrappedTransport) == 0);
}

OTObjectWrapper<OTNetPeer*> OTNetTransport::getPeerByFd(OTNetFd nFd)
{
	m_oPeersMutex->lock();
	OTObjectWrapper<OTNetPeer*> m_Peer = NULL;

	std::map<OTNetFd, OTObjectWrapper<OTNetPeer*> >::iterator iter = m_Peers.find(nFd);
	if(iter != m_Peers.end()){
		m_Peer = iter->second;
	}
	m_oPeersMutex->unlock();

	return m_Peer;
}

void OTNetTransport::insertPeer(OTObjectWrapper<OTNetPeer*> oPeer)
{
	m_oPeersMutex->lock();
	if(oPeer){
		m_Peers.insert( std::pair<OTNetFd, OTObjectWrapper<OTNetPeer*> >(oPeer->getFd(), oPeer) );
	}
	m_oPeersMutex->unlock();
}

void OTNetTransport::removePeer(OTNetFd nFd)
{
	m_oPeersMutex->lock();
	std::map<OTNetFd, OTObjectWrapper<OTNetPeer*> >::iterator iter;
	if((iter = m_Peers.find(nFd)) != m_Peers.end()){
		OTObjectWrapper<OTNetPeer*> oPeer = iter->second;
		m_Peers.erase(iter);
	}
	m_oPeersMutex->unlock();
}

int OTNetTransport::OTNetTransportCb_Stream(const tnet_transport_event_t* e)
{
	OTObjectWrapper<OTNetPeer*> oPeer = NULL;
	OTNetTransport* This = (OTNetTransport*)e->callback_data;

	switch(e->type)
	{	
		case event_closed:
			{
				oPeer = (This)->getPeerByFd(e->local_fd);
				if(oPeer)
				{
					oPeer->setConnected(false);
					(This)->removePeer(e->local_fd);
					if((This)->m_oCallback)
					{
						(This)->m_oCallback->onConnectionStateChanged(oPeer);
					}
				}
				break;
			}
		case event_connected:
		case event_accepted:
			{
				oPeer = (This)->getPeerByFd(e->local_fd);
				if(oPeer)
				{
					oPeer->setConnected(true);
				}
				else
				{
					oPeer = new OTNetPeerStream(e->local_fd, true);
					(This)->insertPeer(oPeer);
				}
				if((This)->m_oCallback)
				{
					(This)->m_oCallback->onConnectionStateChanged(oPeer);
				}
				break;
			}

		case event_data:
			{
				oPeer = (This)->getPeerByFd(e->local_fd);
				if(!oPeer)
				{
					OT_DEBUG_ERROR_EX(kOTMobuleNameNetTransport, "Data event but no peer found!");
					return -1;
				}
				
				size_t nConsumedBytes = oPeer->getDataSize();
				if((nConsumedBytes + e->size) > kOTMaxStreamBufferSize)
				{
					OT_DEBUG_ERROR_EX(kOTMobuleNameNetTransport, "Stream buffer too large[%u > %u]. Did you forget to consume the bytes?", (nConsumedBytes + e->size), kOTMaxStreamBufferSize);
					dynamic_cast<OTNetPeerStream*>(*oPeer)->cleanupData();
				}
				else
				{
					if((This)->m_oCallback)
					{
						if(dynamic_cast<OTNetPeerStream*>(*oPeer)->appenData(e->data, e->size))
						{
							nConsumedBytes += e->size;
						}
						(This)->m_oCallback->onData(oPeer, nConsumedBytes);
					}
					if(nConsumedBytes)
					{
						dynamic_cast<OTNetPeerStream*>(*oPeer)->remoteData(0, nConsumedBytes);
					}
				}
				break;
			}

		case event_error:
		default:
			{
				break;
			}
	}

	return 0;
}
