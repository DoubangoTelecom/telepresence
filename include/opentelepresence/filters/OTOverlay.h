/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_FILTER_OVERLAY_H
#define OPENTELEPRESENCE_FILTER_OVERLAY_H

#include "OpenTelepresenceConfig.h"
#include "opentelepresence/OTObject.h"
#include "opentelepresence/OTCommon.h"
#include "opentelepresence/filters/OTFilter.h"

#include <string>

class OTOverlayVideoWatermark : public OTObject
{
protected:
	OTOverlayVideoWatermark(size_t nInWidth, size_t nInHeight, size_t nOutWidth, size_t nOutHeight, std::string strImagePath);
public:
	virtual ~OTOverlayVideoWatermark();
	virtual OT_INLINE const char* getObjectId() { return "OTOverlayVideoWatermark"; }

	virtual bool draw(
			const void* pcInBufferPtr, size_t nInBufferSize, size_t nInWidth, size_t nInHeight, size_t nOutWidth, size_t nOutHeight,
			void* pOutFrame
		);

	static OTObjectWrapper<OTOverlayVideoWatermark*> New(size_t nInWidth, size_t nInHeight, size_t nOutWidth, size_t nOutHeight, std::string strImagePath);

private:
	size_t m_nInWidth, m_nInHeight, m_nOutWidth, m_nOutHeight;
	OTObjectWrapper<OTFilterVideo*> m_oFilter;
	std::string m_strImagePath;
	bool m_bTriedToCreateFilter;
};

class OTOverlayVideoText : public OTObject
{
protected:
	OTOverlayVideoText(size_t nInWidth, size_t nInHeight, size_t nOutWidth, size_t nOutHeight, std::string strDisplayName, std::string strJobTitle, std::string strCopyright, size_t nNameFontSize, size_t nCopyrightFontSize);
public:
	virtual ~OTOverlayVideoText();
	virtual OT_INLINE const char* getObjectId() { return "OTOverlayVideoText"; }
	
	virtual OT_INLINE void setFontsFolderPath(std::string strPath) { m_strFontsFolderPath = strPath; }
	virtual OT_INLINE void setCopyrightFontFileName(std::string strPath){ m_strCopyrightFontFileName = strPath; }
	virtual OT_INLINE void setSpeakerNameFontFileName(std::string strPath){ m_strSpeakerNameFontFileName = strPath; }

	virtual bool isValid();
	virtual bool draw(
			const void* pcInBufferPtr, size_t nInBufferSize, size_t nInWidth, size_t nInHeight, size_t nOutWidth, size_t nOutHeight,
			void* pOutFrame
		);
	
	static OTObjectWrapper<OTOverlayVideoText*> New(
			size_t nInWidth, size_t nInHeight, 
			size_t nOutWidth, size_t nOutHeight,
			std::string strDisplayName, 
			std::string strJobTitle,
			std::string strCopyright = OPENTELEPRESENCE_VIDEO_OVERLAY_TEXT_COPYRIGTH, 
			size_t nNameFontSize = OPENTELEPRESENCE_VIDEO_OVERLAY_FONTSIZE_COPYRIGTH, 
			size_t nCopyrightFontSize = OPENTELEPRESENCE_VIDEO_OVERLAY_FONTSIZE_NAME
		);

private:
	bool _updateFilter( 
		size_t nInWidth, size_t nInHeight, 
		size_t nOutWidth, size_t nOutHeight, 
		size_t nFps = OPENTELEPRESENCE_VIDEO_FPS_DEFAULT);
	
protected:
	size_t m_nInWidth, m_nInHeight, m_nOutWidth, m_nOutHeight;
	std::string m_strDisplayNameText;
	std::string m_strJobTitleText;
	std::string m_strFontsFolderPath;
	std::string m_strCopyrightText;
	std::string m_strCopyrightFontFileName;
	std::string m_strSpeakerNameFontFileName;
	size_t m_nCopyrightFontSize;
	size_t m_nSpeakerNameFontSize;

	OTObjectWrapper<OTFilterVideo*> m_oFilter;
};

#endif /* OPENTELEPRESENCE_FILTER_OVERLAY_H */
