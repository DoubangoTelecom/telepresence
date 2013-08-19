/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_FILTER_H
#define OPENTELEPRESENCE_FILTER_H

#include "OpenTelepresenceConfig.h"
#include "opentelepresence/OTObject.h"
#include "opentelepresence/OTCommon.h"

#include <string>

class OTFilter : public OTObject
{
protected:
	OTFilter(OTMediaType_t eMediaType, std::string strDescription);
public:
	virtual ~OTFilter();
	virtual OT_INLINE const char* getObjectId() { return "OTFilter"; }

	virtual bool isValid() = 0;
	
protected:
	OTMediaType_t m_eMediaType;
	std::string m_strDescription;
};


class OTFilterVideo : public OTFilter
{
protected:
	OTFilterVideo(std::string strDescription, size_t nInWidth, size_t nInHeight, size_t nOutWidth, size_t nOutHeight, size_t nFps);
public:
	virtual ~OTFilterVideo();
	virtual OT_INLINE const char* getObjectId() { return "OTFilterVideo"; }

	virtual bool filterFrame(
			const void* pcInBufferPtr, size_t nInBufferSize, size_t nInWidth, size_t nInHeight, size_t nOutWidth, size_t nOutHeight,
			void* pOutFrame
		) = 0;

public:
	static OTObjectWrapper<OTFilterVideo*> New(
		std::string strDescription, 
		size_t nInWidth, size_t nInHeight, 
		size_t nOutWidth, size_t nOutHeight, 
		size_t nFps = OPENTELEPRESENCE_VIDEO_FPS_DEFAULT);
};

#endif /* OPENTELEPRESENCE_FILTER_H */
