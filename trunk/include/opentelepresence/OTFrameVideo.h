/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_FRAMEVIDEO_H
#define OPENTELEPRESENCE_FRAMEVIDEO_H

#include "OpenTelepresenceConfig.h"
#include "opentelepresence/OTFrame.h"

class OTFrameVideo : public OTFrame
{
protected:
	OTFrameVideo(bool bOwnBuffer, const void *pBufferPtr, uint32_t nBufferSize);
public:
	virtual ~OTFrameVideo();
	virtual OT_INLINE const char* getObjectId() { return "OTFrameVideo"; }

	virtual OT_INLINE unsigned getWidth() { return  m_nWidth; }
	virtual OT_INLINE void setWidth(unsigned nWidth) {  m_nWidth = nWidth; }
	virtual OT_INLINE unsigned getHeight() { return  m_nHeight; }
	virtual OT_INLINE void setHeight(unsigned nHeight) {  m_nHeight = nHeight; }

	static OTObjectWrapper<OTFrameVideo*> New(bool bOwnBuffer, const void *pBufferPtr, uint32_t nBufferSize);

private:
	unsigned m_nWidth;
	unsigned m_nHeight;
};

#endif /* OPENTELEPRESENCE_FRAMEVIDEO_H */
