/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#if !defined(OPENTELEPRESENCE_MUTEX_H)
#define OPENTELEPRESENCE_MUTEX_H

#include "OpenTelepresenceConfig.h"
#include "opentelepresence/OTObject.h"

#include "tsk_mutex.h"

class OTMutex : public OTObject
{	
public:
	OTMutex(bool bRecursive = true);
	virtual ~OTMutex();
	virtual OT_INLINE const char* getObjectId() { return "OTMutex"; }
	bool lock();
	bool unlock();

private:
	tsk_mutex_handle_t* m_phOTMutex;
};

#endif /* OPENTELEPRESENCE_MUTEX_H */
