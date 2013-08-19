/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_DOC_STREAMER_H
#define OPENTELEPRESENCE_DOC_STREAMER_H

#include "OpenTelepresenceConfig.h"
#include "opentelepresence/OTObject.h"
#include "opentelepresence/OTCommon.h"
#include "opentelepresence/OTBridgeInfo.h"

#include <string>
#include <map>

class OTDocStreamer;

class OTDocStreamerCallback
{
public:
	OTDocStreamerCallback(){}
	virtual ~OTDocStreamerCallback(){}

	virtual void onStateChanged(OTDocStreamerState_t eState, OTDocStreamer* oStreamer) const= 0;
	virtual void onError(bool bIsFatal, OTDocStreamer* oStreamer) const= 0;
};

class OTDocStreamer : public OTObject
{
protected:
	OTDocStreamer(OTDocStreamerType_t eType, OTObjectWrapper<OTBridgeInfo*> oBridgeInfo, const OTDocStreamerCallback* pcCallback = NULL);
public:
	virtual ~OTDocStreamer();
	virtual OT_INLINE const char* getObjectId() { return "OTDocStreamer"; }

	virtual OT_INLINE OTDocStreamerState_t getState() { return m_eState; }
	virtual OT_INLINE void setCallback(OTDocStreamerCallback* pcCallback) { m_pcCallback = pcCallback; }
	virtual bool isValid() = 0;
	virtual bool open(std::string strDocPath) = 0;
	virtual bool close() = 0;
	virtual size_t getPagesCount() = 0;
	virtual size_t getPageIndex() = 0;
	virtual bool exportPage(size_t nPageIndex) = 0;
	virtual bool getCurrentFrame(size_t* pnPageIndex, const void** ppcPtr, size_t* pnWidth = NULL, size_t* pnHeight = NULL) = 0;

	virtual OT_INLINE bool isOpened() { return (m_eState & OTDocStreamerState_Opened); }
	virtual OT_INLINE uint64_t getId() { return m_uId; }
	virtual OT_INLINE size_t getWidth() { return m_oBridgeInfo->getVideoWidth(); }
	virtual OT_INLINE size_t getHeight() { return m_oBridgeInfo->getVideoHeight(); }
	virtual OT_INLINE OTDocStreamerType_t getType() { return m_eType; }

	static std::string buildCommandArgs(unsigned short nLocalPort);
	static std::string buildConnectionString(unsigned short nLocalPort);
	static bool isSupported();
	static OTObjectWrapper<OTDocStreamer*> New(OTObjectWrapper<OTBridgeInfo*> oBridgeInfo, const OTDocStreamerCallback* pcCallback = NULL);

	static bool PredIsOpened(const std::pair<uint64_t, OTObjectWrapper<OTDocStreamer*> > &pcOTStreamer) { return pcOTStreamer.second->isOpened(); }

protected:
	virtual OT_INLINE void setState(OTDocStreamerState_t eState) 
	{ 
		if(m_eState != eState)
		{
			m_eState = eState;
			if(m_pcCallback)
			{
				m_pcCallback->onStateChanged(eState, this);
			}
		}
	}

protected:
	uint64_t m_uId;
	OTObjectWrapper<OTBridgeInfo*> m_oBridgeInfo;
	OTDocStreamerState_t m_eState;
	const OTDocStreamerCallback* m_pcCallback;

private:
	OTDocStreamerType_t m_eType;
	static uint64_t g_uId;
};



#endif /* OPENTELEPRESENCE_DOC_STREAMER_H */
