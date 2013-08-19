/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_DOC_STREAMER_OPENOFFICE_H
#define OPENTELEPRESENCE_DOC_STREAMER_OPENOFFICE_H

#include "OTDocStreamer.h"
#include "opentelepresence/OTMutex.h"

#if HAVE_OPENOFFICE

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>

typedef enum OO_THREAD_INDEX_E
{
	OO_THREAD_INDEX_OPEN,
	OO_THREAD_INDEX_EXPORT,
	OO_THREAD_INDEX_COUNT
}
OO_THREAD_INDEX_T;

class OTDocStreamerOpenOffice : public OTDocStreamer
{	
public:
	OTDocStreamerOpenOffice(OTObjectWrapper<OTBridgeInfo*> oBridgeInfo, const OTDocStreamerCallback* pcCallback = NULL);
	virtual ~OTDocStreamerOpenOffice();
	virtual OT_INLINE const char* getObjectId() { return "OTDocStreamerOpenOffice"; }

	// @Overrides (OTDocStreamer)
	virtual bool isValid();
	virtual bool open(std::string strDocPath);
	virtual bool close();
	virtual size_t getPagesCount();
	virtual size_t getPageIndex();
	virtual bool exportPage(size_t nPageIndex);
	virtual bool getCurrentFrame(size_t* pnPageIndex, const void** ppcPtr, size_t* pnWidth = NULL, size_t* pnHeight = NULL);

	static std::string buildCommandArgs(unsigned short nLocalPort);
	static std::string buildConnectionString(unsigned short nLocalPort);

private:
	bool _decode();
	bool _close();
	static void* TSK_STDCALL threadOpen(void *pArg);
	static void* TSK_STDCALL threadExport(void *pArg);

private:
	std::string m_strInFilePath;
	rtl::OUString m_strInFileUrl;
	bool m_bValid;
	int32_t m_nPageIndex;
	size_t m_nPagesCount;
	void* m_ppThreads[OO_THREAD_INDEX_COUNT];
	OTObjectWrapper<OTMutex*>m_oMutex;
	std::string m_strJpegFilePath;
	rtl::OUString m_strJpegFileUrl;
	void* m_pYuvBufferPtr;
	size_t m_nYuvBufferSize;
	size_t m_nYuvBufferWidth;
	size_t m_nYuvBufferHeight;
	bool m_bYuvBufferGotPict;

	com::sun::star::uno::Reference< com::sun::star::drawing::XDrawPagesSupplier > m_xDrawPagesSupplier;
	com::sun::star::uno::Reference< com::sun::star::drawing::XDrawPages > m_xDrawPages;
	com::sun::star::uno::Reference< com::sun::star::drawing::XDrawPage > m_xDrawPage;
	com::sun::star::uno::Reference< com::sun::star::document::XExporter > m_xExporter;
	com::sun::star::uno::Reference< com::sun::star::document::XFilter > m_xFilter;
	com::sun::star::uno::Reference< com::sun::star::lang::XComponent > m_xComponent;
	com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > m_xseqPropFilter;
};

#endif /* HAVE_OPENOFFICE */

#endif /* OPENTELEPRESENCE_DOC_STREAMER_OPENOFFICE_H */
