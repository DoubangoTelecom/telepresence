/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#include "opentelepresence/filters/OTOverlay.h"

#include <assert.h>
#include <sys/stat.h> /* stat() */

#include "tsk_string.h"

//
//	OTOverlayVideoWatermark
//
OTOverlayVideoWatermark::OTOverlayVideoWatermark(size_t nInWidth, size_t nInHeight, size_t nOutWidth, size_t nOutHeight, std::string strImagePath)
: m_nInWidth(nInWidth)
, m_nInHeight(nInHeight)
, m_nOutWidth(nOutWidth)
, m_nOutHeight(nOutHeight)
, m_strImagePath(strImagePath)
, m_bTriedToCreateFilter(false)
{

}

OTOverlayVideoWatermark::~OTOverlayVideoWatermark()
{
	OT_DEBUG_INFO("*** OTOverlayVideoWatermark destroyed ***");
}


bool OTOverlayVideoWatermark::draw(
			const void* pcInBufferPtr, size_t nInBufferSize, size_t nInWidth, size_t nInHeight, size_t nOutWidth, size_t nOutHeight,
			void* pOutFrame
		)
{
	if(m_nInWidth != nInWidth || m_nInHeight != nInHeight || m_nOutWidth != nOutWidth || m_nOutHeight != nOutHeight)
	{
		OT_DEBUG_INFO_EX(kOTMobuleNameFFmpegOverlay, "Filter (watermark) size changed");
		m_nInWidth = nInWidth;
		m_nInHeight = nInHeight;
		m_nOutWidth = nOutWidth;
		m_nOutHeight = nOutHeight;
		
		OTObjectSafeRelease(m_oFilter);
		m_bTriedToCreateFilter = false;
	}

	if(!m_bTriedToCreateFilter)
	{
		OT_DEBUG_INFO_EX(kOTMobuleNameFFmpegOverlay, "Create filter (watermark)");
		m_bTriedToCreateFilter = true;
		struct stat _stat;
		if(stat(m_strImagePath.c_str(), &_stat) == 0 && _stat.st_size > 0)
		{
			m_oFilter = OTFilterVideo::New(std::string("movie=")+m_strImagePath+std::string(" [watermark]; [in][watermark] overlay=main_w-overlay_w-10:10 [out]"), nInWidth, nInHeight, nOutWidth, nOutHeight);
			if(m_oFilter && !m_oFilter->isValid())
			{
				OT_DEBUG_ERROR_EX(kOTMobuleNameFFmpegOverlay, "Filter created but not valid");
				OTObjectSafeRelease(m_oFilter);
				return false;
			}
		}
		else
		{
			OT_DEBUG_ERROR_EX(kOTMobuleNameFFmpegOverlay, "Cannot stat() file with path = %s", m_strImagePath.c_str());
			return false;
		}
	}

	if(m_oFilter)
	{
		return m_oFilter->filterFrame(pcInBufferPtr, nInBufferSize, nInWidth, nInHeight, nOutWidth, nOutHeight, pOutFrame);
	}
	return false;
}

OTObjectWrapper<OTOverlayVideoWatermark*> OTOverlayVideoWatermark::New(size_t nInWidth, size_t nInHeight, size_t nOutWidth, size_t nOutHeight, std::string strImagePath)
{
	return new OTOverlayVideoWatermark(nInWidth, nInHeight, nOutWidth, nOutHeight, strImagePath);
}




//
//	OTOverlayVideoText
//

OTOverlayVideoText::OTOverlayVideoText(size_t nInWidth, size_t nInHeight, size_t nOutWidth, size_t nOutHeight, std::string strDisplayName, std::string strJobTitle, std::string strCopyright, size_t nNameFontSize, size_t nCopyrightFontSize)
: m_nInWidth(nInWidth)
, m_nInHeight(nInHeight)
, m_nOutWidth(nOutWidth)
, m_nOutHeight(nOutHeight)
, m_strDisplayNameText(strDisplayName)
, m_strJobTitleText(strJobTitle)
, m_strCopyrightText(strCopyright)
, m_strFontsFolderPath(OPENTELEPRESENCE_VIDEO_OVERLAY_PATH_FONTFOLDER)
, m_strCopyrightFontFileName(OPENTELEPRESENCE_VIDEO_OVERLAY_FILENAME_FONT_COPYRIGHT)
, m_strSpeakerNameFontFileName(OPENTELEPRESENCE_VIDEO_OVERLAY_FILENAME_FONT_SPEAKER_NAME)
, m_nCopyrightFontSize(nCopyrightFontSize)
, m_nSpeakerNameFontSize(nNameFontSize)
{

}

OTOverlayVideoText::~OTOverlayVideoText()
{
	OT_DEBUG_INFO("*** OTOverlayVideoText destroyed ***");
}

bool OTOverlayVideoText::_updateFilter( 
		size_t nInWidth, size_t nInHeight, 
		size_t nOutWidth, size_t nOutHeight, 
		size_t nFps)
{
	OT_DEBUG_INFO_EX(kOTMobuleNameFFmpegOverlay, "Create filter (text)");

	OTObjectSafeRelease(m_oFilter);

	std::string strFilterDesc;
	tsk_istr_t iNum;

	// copyright message
	if(!m_strCopyrightText.empty() && !m_strFontsFolderPath.empty() && !m_strCopyrightFontFileName.empty())
	{
		std::string strFontFilePath = m_strFontsFolderPath+"/"+m_strCopyrightFontFileName;
		struct stat _stat;
		if(stat(strFontFilePath.c_str(), &_stat) == 0 && _stat.st_size > 0)
		{
			tsk_itoa(m_nCopyrightFontSize, &iNum);
			strFilterDesc = "drawtext=fontfile="+strFontFilePath+": text='"+ m_strCopyrightText +"': x=5: y=5: shadowx=0: shadowy=0: fontsize="+std::string(iNum)+": fontcolor=white@1.0: box=0: boxcolor=gray@0.5";
		}
		else
		{
			OT_DEBUG_ERROR_EX(kOTMobuleNameFFmpegOverlay, "Cannot stat() file with path = %s", strFontFilePath.c_str());
		}
	}
	if((!m_strJobTitleText.empty() || !m_strDisplayNameText.empty()) && !m_strFontsFolderPath.empty() && !m_strSpeakerNameFontFileName.empty())
	{
		std::string strFontFilePath = m_strFontsFolderPath+"/"+m_strSpeakerNameFontFileName;
		struct stat _stat;
		if(stat(strFontFilePath.c_str(), &_stat) == 0 && _stat.st_size > 0)
		{
			if(!strFilterDesc.empty())
			{
				strFilterDesc += ", ";
			}
			tsk_itoa(m_nSpeakerNameFontSize, &iNum);
			// x=(w-text_w)/2:y=(h-text_h-line_h)/2
			strFilterDesc += "drawtext=fontfile="+strFontFilePath+": text='\n\t"+(m_strDisplayNameText.empty() ? std::string("") : m_strDisplayNameText)
				+ (m_strJobTitleText.empty() ? std::string("") : "\n\t"+m_strJobTitleText) + "': x=16:y=(h-text_h-16): fontsize="+std::string(iNum)+": fontcolor=white@1.0: box=1: boxcolor=blue@0.6";
		}
		else
		{
			OT_DEBUG_ERROR_EX(kOTMobuleNameFFmpegOverlay, "Cannot stat() file with path = %s", strFontFilePath.c_str());
		}
	}

	if(nInWidth != nOutWidth || nInHeight != nOutHeight || strFilterDesc.empty())
	{
		tsk_istr_t iH, iW;
		tsk_itoa(nOutWidth, &iW); // "-1" to keep aspect ratio
		tsk_itoa(nOutHeight, &iH);
		
		strFilterDesc = "scale=w=" + std::string(iW) + ":h=" + std::string(iH) + (strFilterDesc.empty() ? "" : (", " + strFilterDesc));
	}

	OTObjectWrapper<OTFilterVideo*> oFilter = OTFilterVideo::New(strFilterDesc, nInWidth, nInHeight, nOutWidth, nOutHeight);
	if(oFilter && oFilter->isValid())
	{
		m_nInWidth = nInWidth;
		m_nInHeight = nInHeight;
		m_nOutWidth = nOutWidth;
		m_nOutHeight = nOutHeight;

		m_oFilter = oFilter;
		return oFilter;
	}
	else
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameFFmpegOverlay, "Failed to update overlay");
	}
	return false;
}

bool OTOverlayVideoText::isValid()
{
	return (m_oFilter && m_oFilter->isValid());
}

bool OTOverlayVideoText::draw(
			const void* pcInBufferPtr, size_t nInBufferSize, size_t nInWidth, size_t nInHeight, size_t nOutWidth, size_t nOutHeight,
			void* pOutFrame
		)
{
	if(!isValid() && !_updateFilter(nInWidth, nInHeight, nOutWidth, nOutHeight))
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameFFmpegOverlay, "Not valid");
		return false;
	}

	if(m_nInWidth != nInWidth || m_nInHeight != nInHeight || m_nOutWidth != nOutWidth || m_nOutHeight != nOutHeight)
	{
		if(!_updateFilter(nInWidth, nInHeight, nOutWidth, nOutHeight))
		{
			return false;
		}
	}

	return m_oFilter->filterFrame(pcInBufferPtr, nInBufferSize, nInWidth, nInHeight, nOutWidth, nOutHeight, pOutFrame);
}

OTObjectWrapper<OTOverlayVideoText*> OTOverlayVideoText::New(
		size_t nInWidth, size_t nInHeight, 
		size_t nOutWidth, size_t nOutHeight,
		std::string strDisplayName, 
		std::string strJobTitle,
		std::string strCopyright, 
		size_t nNameFontSize, 
		size_t nCopyrightFontSize
	)
{
	return new OTOverlayVideoText(nInWidth, nInHeight, nOutWidth, nOutHeight, strDisplayName, strJobTitle, strCopyright, nNameFontSize, nCopyrightFontSize);
}

