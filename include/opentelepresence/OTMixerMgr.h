/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_MIXERMGR_H
#define OPENTELEPRESENCE_MIXERMGR_H

#include "OpenTelepresenceConfig.h"

#include "opentelepresence/OTObject.h"
#include "opentelepresence/OTCommon.h"
#include "opentelepresence/OTSessionInfo.h"
#include "opentelepresence/OTCodec.h"
#include "opentelepresence/OTBridgeInfo.h"
#include "opentelepresence/recorder/OTRecorder.h"

#include <map>

class OTMixerMgr : public OTObject
{
public:
	OTMixerMgr(OTMediaType_t eMediaType, OTObjectWrapper<OTBridgeInfo*> oBridgeInfo);
	~OTMixerMgr();
	virtual OT_INLINE const char* getObjectId() { return "OTMixerMgr"; }
	
	virtual OT_INLINE OTMediaType_t getMediaType() { return m_eMediaType; }
	virtual OT_INLINE uint64_t getBridgeId() { return m_nBridgeId; }
	virtual OT_INLINE void setRecorder(OTObjectWrapper<OTRecorder*> oRecorder){ m_oRecorder = oRecorder; }

	virtual int start()=0;
	virtual bool isStarted()=0;
	virtual int pause()=0;
	virtual bool isPaused()=0;
	virtual int flush()=0;
	virtual int stop()=0;
	virtual int attachMediaPlugins(OTObjectWrapper<OTSessionInfo*> pOTSessionInfo)=0;
	virtual int deAttachMediaPlugins(OTObjectWrapper<OTSessionInfo*> pOTSessionInfo)=0;

	static OTObjectWrapper<OTMixerMgr*> New(OTMediaType_t eMediaType, OTObjectWrapper<OTBridgeInfo*> oBridgeInfo);

protected:
	virtual bool isValid()=0;
	virtual OTObjectWrapper<OTCodec*> findBestCodec(OTCodec_Type_t eListOfCodecsToSearchInto);
	virtual bool removeCodecs(OTCodec_Type_t eListOfCodecsToSearchInto);
	virtual bool addCodec(OTObjectWrapper<OTCodec*> oCodec);
	virtual bool executeActionOnCodecs(OTCodecAction_t eAction);

protected:
	OTMediaType_t m_eMediaType;
	uint64_t m_nBridgeId;
	std::map<OTCodec_Type_t, OTObjectWrapper<OTCodec*> > m_OTCodecs;
	OTObjectWrapper<OTBridgeInfo*> m_oBridgeInfo;
	OTObjectWrapper<OTRecorder*> m_oRecorder;
};

#endif /* OPENTELEPRESENCE_MIXERMGR_H */
