/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_MIXERMGRMGR_H
#define OPENTELEPRESENCE_MIXERMGRMGR_H

#include "OpenTelepresenceConfig.h"

#include "opentelepresence/OTObject.h"
#include "opentelepresence/OTCommon.h"
#include "opentelepresence/OTMixerMgr.h"
#include "opentelepresence/OTSessionInfo.h"
#include "opentelepresence/OTBridgeInfo.h"
#include "opentelepresence/recorder/OTRecorder.h"

class OTMixerMgrMgr : public OTObject
{
public:
	OTMixerMgrMgr(OTMediaType_t eMediaType, OTObjectWrapper<OTBridgeInfo*> oBridgeInfo);
	~OTMixerMgrMgr();
	virtual OT_INLINE const char* getObjectId() { return "OTMixerMgrMgr"; }

	inline OTMediaType_t getMediaType() { return m_eMediaType; }
	virtual bool isValid();

	int start(OTMediaType_t eMediaType);
	bool isStarted(OTMediaType_t eMediaType);
	int pause(OTMediaType_t eMediaType);
	bool isPaused(OTMediaType_t eMediaType);
	int flush(OTMediaType_t eMediaType);
	int stop(OTMediaType_t eMediaType);
	int attachMediaPlugins(OTObjectWrapper<OTSessionInfo*> pOTSessionInfo);
	int deAttachMediaPlugins(OTObjectWrapper<OTSessionInfo*> pOTSessionInfo);

	static OTObjectWrapper<OTMixerMgrMgr*> New(OTMediaType_t eMediaType, OTObjectWrapper<OTBridgeInfo*> oBridgeInfo);

private:
	OTObjectWrapper<OTMixerMgr*> m_pOTMixerMgrAudio;
	OTObjectWrapper<OTMixerMgr*> m_pOTMixerMgrVideo;
	OTObjectWrapper<OTRecorder*> m_oRecorder;
	OTMediaType_t m_eMediaType;
	bool m_bValid;
};

#endif /* OPENTELEPRESENCE_MIXERMGRMGR_H */
