/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_PROCESS_H
#define OPENTELEPRESENCE_PROCESS_H

#include "OpenTelepresenceConfig.h"
#include "opentelepresence/OTObject.h"
#include "opentelepresence/OTCommon.h"

#include <string>

class OTProcess : public OTObject
{
protected:
	OTProcess(std::string strProgram, std::string strArgs);
public:
	virtual ~OTProcess();
	virtual OT_INLINE const char* getObjectId() { return "OTProcess"; }
	virtual bool isValid() = 0;
	virtual bool start() = 0;
	virtual bool isStarted() = 0;
	virtual bool stop() = 0;

	static OTObjectWrapper<OTProcess*> New(std::string strProgram, std::string strArgs);
	
protected:
	
};

#endif /* OPENTELEPRESENCE_PROCESS_H */
