/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#include "opentelepresence/docstreamer/OTDocStreamerOpenOffice.h"

#if HAVE_OPENOFFICE

#include <cppuhelper/bootstrap.hxx>

#include <osl/file.hxx>
#include <osl/process.h>
#include <rtl/process.h>

#include <com/sun/star/bridge/XUnoUrlResolver.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>

extern "C"
{
	#include <libavformat/avformat.h>
}

#include "tsk_string.h"
#include "tsk_uuid.h"
#include "tsk_thread.h"
#include "tsk_memory.h"
#include "tsk_debug.h"

#include <sys/stat.h> /* stat() */
#include <algorithm> /* replace() */

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::bridge;
using namespace com::sun::star::frame;
using namespace com::sun::star::registry;
using namespace com::sun::star::document;
using namespace com::sun::star::drawing;

static const char* ot_av_err2str(int errnum)
{
	static char __errbuf[AV_ERROR_MAX_STRING_SIZE] = {0};
    return av_make_error_string(__errbuf, AV_ERROR_MAX_STRING_SIZE, errnum);
}

static rtl::OUString _openOfficeGetUrl(std::string path)
{
	OUString sAbsoluteDocUrl, sWorkingDir, sDocPathUrl, sArgDocUrl = OUString::createFromAscii(path.c_str());
    osl_getProcessWorkingDir(&sWorkingDir.pData);
    osl::FileBase::getFileURLFromSystemPath( sArgDocUrl, sDocPathUrl);
    osl::FileBase::getAbsoluteFileURL( sWorkingDir, sDocPathUrl, sAbsoluteDocUrl);
	return sAbsoluteDocUrl;
}

static std::string _openOfficeGetPath(rtl::OUString url)
{
	OUString ustrSystemPath;
	if(osl::FileBase::getSystemPathFromFileURL(url, ustrSystemPath) == osl::FileBase::E_None)
	{
		return std::string(OUStringToOString(ustrSystemPath, RTL_TEXTENCODING_ASCII_US).getStr());
	}
	OT_DEBUG_ERROR_EX(kOTMobuleNameDocStreamerOpenOffice, "_openOfficeGetPath(%s) failed", 
		OUStringToOString(url, RTL_TEXTENCODING_ASCII_US).getStr());
	return std::string(OUStringToOString(url, RTL_TEXTENCODING_ASCII_US).getStr());
}

OTDocStreamerOpenOffice::OTDocStreamerOpenOffice(OTObjectWrapper<OTBridgeInfo*> oBridgeInfo, const OTDocStreamerCallback* pcCallback /*= NULL*/)
: OTDocStreamer(OTDocStreamerType_OpenOffice, oBridgeInfo, pcCallback)
, m_nPageIndex(-1)
, m_nPagesCount(0)
, m_bValid(false)

, m_xDrawPagesSupplier(NULL)
, m_xDrawPages(NULL)
, m_xDrawPage(NULL)
, m_xExporter(NULL)
, m_xFilter(NULL)
, m_xComponent(NULL)

, m_pYuvBufferPtr(NULL)
, m_nYuvBufferSize(0)
, m_nYuvBufferWidth(oBridgeInfo->getVideoWidth())
, m_nYuvBufferHeight(oBridgeInfo->getVideoHeight())
, m_bYuvBufferGotPict(false)
{
	memset(m_ppThreads, 0, sizeof(m_ppThreads));
	m_oMutex = new OTMutex();

	tsk_uuidstring_t uuid;
	tsk_uuidgenerate(&uuid);
	m_strJpegFileUrl = _openOfficeGetUrl(
		oBridgeInfo->getPresentationSharingBaseFolder()
		+ std::string("/") + oBridgeInfo->getId()
		+ std::string("/") + std::string("$") + std::string(uuid) + std::string(".jpeg")
		);
	m_strJpegFilePath = _openOfficeGetPath(m_strJpegFileUrl);

	m_bValid = (m_oMutex);
}

OTDocStreamerOpenOffice::~OTDocStreamerOpenOffice()
{
	close();
}

// @Overrides (OTDocStreamer)
bool OTDocStreamerOpenOffice::isValid()
{
	return m_bValid;
}

// @Overrides (OTDocStreamer)
bool OTDocStreamerOpenOffice::open(std::string strDocPath)
{
	int iRet;

	if(strDocPath.length() == 0)
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameDocStreamerOpenOffice, "Invalid parameter");
		return false;
	}

	if(m_eState != OTDocStreamerState_None && m_eState != OTDocStreamerState_Closed)
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameDocStreamerOpenOffice, "Not valid to request opening the streamer when the current state = %d", m_eState);
		return false;
	}

	struct stat _stat;
	if(stat(strDocPath.c_str(), &_stat) != 0 || _stat.st_size == 0)
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameDocStreamerOpenOffice, "%s doesn't exist or is empty", strDocPath.c_str());
		return false;
	}
	m_strInFileUrl = _openOfficeGetUrl(strDocPath);
	m_strInFilePath = _openOfficeGetPath(m_strInFileUrl);
	
	if(m_ppThreads[OO_THREAD_INDEX_OPEN])
	{
		OT_DEBUG_INFO_EX(kOTMobuleNameDocStreamerOpenOffice, "Join 'open' thread");
		tsk_thread_join(&m_ppThreads[OO_THREAD_INDEX_OPEN]);
	}

	iRet = tsk_thread_create(&m_ppThreads[OO_THREAD_INDEX_OPEN], OTDocStreamerOpenOffice::threadOpen, this);
	if(iRet != 0)
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameDocStreamerOpenOffice, "Failed to create 'open' thread");
		return false;
	}

	return true;
}

// @Overrides (OTDocStreamer)
bool OTDocStreamerOpenOffice::close()
{
	for(size_t i = 0; i < OO_THREAD_INDEX_COUNT; ++i)
	{
		if(m_ppThreads[i])
		{
			tsk_thread_join(&m_ppThreads[i]);
		}
	}

	return _close();
}

// @Overrides (OTDocStreamer)
size_t OTDocStreamerOpenOffice::getPagesCount()
{
	return m_nPagesCount;
}

size_t OTDocStreamerOpenOffice::getPageIndex()
{
	return m_nPageIndex;
}

// @Overrides (OTDocStreamer)
bool OTDocStreamerOpenOffice::exportPage(size_t nPageIndex)
{
	if(!isOpened())
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameDocStreamerOpenOffice, "Not opened yet");
		return false;
	}
	else if(m_eState == OTDocStreamerState_Exporting)
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameDocStreamerOpenOffice, "Already exporting", m_eState);
		return false;
	}

	if(m_nPageIndex == (int32_t)nPageIndex)
	{
		OT_DEBUG_INFO_EX(kOTMobuleNameDocStreamerOpenOffice, "Trying to export same page...nothing to be done");
		return true;
	}

	if(nPageIndex > getPagesCount())
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameDocStreamerOpenOffice, "Invalid page index. %u > %u", nPageIndex, getPagesCount());
		return false;
	}

	if(m_ppThreads[OO_THREAD_INDEX_EXPORT])
	{
		OT_DEBUG_INFO_EX(kOTMobuleNameDocStreamerOpenOffice, "Join 'export' thread");
		tsk_thread_join(&m_ppThreads[OO_THREAD_INDEX_EXPORT]);
	}

	m_nPageIndex = nPageIndex;
	int iRet = tsk_thread_create(&m_ppThreads[OO_THREAD_INDEX_EXPORT], OTDocStreamerOpenOffice::threadExport, this);
	if(iRet != 0)
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameDocStreamerOpenOffice, "Failed to create 'export' thread");
		return false;
	}
	
	return true;
}

// @Overrides (OTDocStreamer)
bool OTDocStreamerOpenOffice::getCurrentFrame(size_t* pnPageIndex, const void** ppcPtr, size_t* pnWidth /*= NULL*/, size_t* pnHeight /*= NULL*/)
{
	if(m_bYuvBufferGotPict && m_pYuvBufferPtr)
	{
		if(pnPageIndex)
		{
			*pnPageIndex = m_nPageIndex;
		}
		if(ppcPtr)
		{
			*ppcPtr = m_pYuvBufferPtr;
		}
		if(pnWidth)
		{
			*pnWidth = m_nYuvBufferWidth;
		}
		if(pnHeight)
		{
			*pnHeight = m_nYuvBufferHeight;
		}
	}
	return m_bYuvBufferGotPict;
}

// close without ending threads (to avoid deadlocks when called from threadOpen() or threadExport())
bool OTDocStreamerOpenOffice::_close()
{
	if(isValid())
	{
		m_oMutex->lock();
#if 0 // Not needed, just dispose the component and let the refs die
		if(m_xDrawPagesSupplier != NULL)
		{
			m_xDrawPagesSupplier->release();
			m_xDrawPagesSupplier = NULL;
		}
		if(m_xDrawPages != NULL)
		{
			m_xDrawPages->release();
			m_xDrawPages = NULL;
		}
		if(m_xDrawPage != NULL)
		{
			m_xDrawPages->release();
			m_xDrawPages = NULL;
		}
		if(m_xExporter != NULL)
		{
			m_xExporter->release();
			m_xExporter = NULL;
		}
		if(m_xFilter != NULL)
		{
			m_xFilter->release();
			m_xFilter = NULL;
		}
#endif
		if(m_xComponent != NULL)
		{
			m_xComponent->dispose();
			m_xComponent = NULL;
		}		

		TSK_FREE(m_pYuvBufferPtr);
		m_nYuvBufferSize = 0;

		// delete the file
		remove(m_strJpegFilePath.c_str());

		m_nPageIndex = m_nPagesCount = 0;

		m_oMutex->unlock();
	}

	setState(OTDocStreamerState_Closed);
	
	return true;
}

bool OTDocStreamerOpenOffice::_decode()
{
	AVFormatContext *pFormatCtx = NULL;
	AVCodecContext *pCodecCtx = NULL;
	AVCodec *pCodec = NULL;
	AVFrame *pFrame = NULL;
	int ret, got_picture_ptr = 0;
	bool bRet = false;
	AVPacket packet = { 0 };	

	ret = avformat_open_input(&pFormatCtx, m_strJpegFilePath.c_str(), NULL, NULL);
	if(!pFormatCtx || !pFormatCtx->streams || !pFormatCtx->streams[0] || !pFormatCtx->streams[0]->codec || ret < 0)
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameDocStreamerOpenOffice, "Could not create context based on file extension: %s", ot_av_err2str(ret));
		goto bail;
	}

	if((ret = av_find_stream_info(pFormatCtx)) < 0)
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameDocStreamerOpenOffice, "No stream info: %s", ot_av_err2str(ret));
		goto bail;
	}

#if 0
	av_dump_format(pFormatCtx, 0, m_strJpegFilePath.c_str(), 0);
#endif

	if(!m_pYuvBufferPtr)
	{
		size_t nSize = (m_nYuvBufferWidth * m_nYuvBufferHeight * 3) >> 1;
		m_pYuvBufferPtr = tsk_calloc(nSize * 2, 1);
		if(!m_pYuvBufferPtr)
		{
			OT_DEBUG_ERROR_EX(kOTMobuleNameDocStreamerOpenOffice, "Failed to allocate buffer with size = %u", nSize);
			goto bail;
		}
		m_nYuvBufferSize = nSize;
	}

	pCodec = avcodec_find_decoder(pFormatCtx->streams[0]->codec->codec_id);
	if(!pCodec)
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameDocStreamerOpenOffice, "Failed to find codec with id: %d", pCodecCtx->codec_id);
		goto bail;
	}

	pCodecCtx = avcodec_alloc_context();
	if(!pCodecCtx)
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameDocStreamerOpenOffice, "Failed to allocate context");
		goto bail;
	}
	avcodec_get_context_defaults(pCodecCtx);
	pCodecCtx->width = m_nYuvBufferWidth;
	pCodecCtx->height = m_nYuvBufferHeight;
	pCodecCtx->pix_fmt = PIX_FMT_YUV420P;
	pCodecCtx->color_range = AVCOL_RANGE_JPEG;
	pCodecCtx->flags2 |= CODEC_FLAG2_FAST;	
	
    if((ret = avcodec_open2(pCodecCtx, pCodec, NULL)) < 0)
    {
        OT_DEBUG_ERROR_EX(kOTMobuleNameDocStreamerOpenOffice, "Failed to open codec: %s", ot_av_err2str(ret));
		goto bail;
    }
	
	av_init_packet(&packet);
    packet.data = NULL;
    packet.size = 0;
    if((ret = av_read_frame(pFormatCtx, &packet)) < 0)
    {
		OT_DEBUG_ERROR_EX(kOTMobuleNameDocStreamerOpenOffice, "Failed to read frame: %s", ot_av_err2str(ret));
		goto bail;
	}

	if(!(pFrame = avcodec_alloc_frame()))
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameDocStreamerOpenOffice, "Failed allocate frame");
		goto bail;
	}
	avcodec_get_frame_defaults(pFrame);

	ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture_ptr, &packet);
    if (ret < 0 || !got_picture_ptr)
    {
        OT_DEBUG_ERROR_EX(kOTMobuleNameDocStreamerOpenOffice, "Failed to decode frame: %s", ot_av_err2str(ret));
		goto bail;
    }
	
	avpicture_layout((AVPicture *)pFrame, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height,
						(unsigned char *)m_pYuvBufferPtr, m_nYuvBufferSize);
#if 0
	{
		FILE *f = fopen("./yuv420.raw", "wb+");
		if(f)
		{
			fwrite(m_pYuvBufferPtr, 1, m_nYuvBufferSize, f);
			fclose(f);
		}
	}
#endif
	
	bRet = true;	

bail:
	m_bYuvBufferGotPict = bRet;
	if(pCodecCtx)
	{
		avcodec_close(pCodecCtx);
		av_free(pCodecCtx);
	}
	if(pFormatCtx)
	{
		avformat_close_input(&pFormatCtx);
	}
	if(pFrame)
	{
		av_free(pFrame);
	}
	av_free_packet(&packet);
	return bRet;
}

std::string OTDocStreamerOpenOffice::buildCommandArgs(unsigned short nLocalPort)
{
	tsk_istr_t istr;
	tsk_itoa(nLocalPort, &istr);
	return std::string("-norestore -headless -nofirststartwizard -invisible \"-accept=socket,host=localhost,port=") 
		+ std::string(istr)
		+ std::string(";urp;StarOffice.ServiceManager\"");
}

std::string OTDocStreamerOpenOffice::buildConnectionString(unsigned short nLocalPort)
{
	tsk_istr_t istr;
	tsk_itoa(nLocalPort, &istr);
	return std::string("uno:socket,host=localhost,port=") 
		+ std::string(istr) 
		+ std::string(";urp;StarOffice.ServiceManager");
}

void* TSK_STDCALL OTDocStreamerOpenOffice::threadOpen(void *pArg)
{
	OT_DEBUG_INFO_EX(kOTMobuleNameDocStreamerOpenOffice, "threadOpen - ENTER");

	static const bool __IsErrorFatal = true; // any error on the open process is fatal

	OTDocStreamerOpenOffice* pThis = dynamic_cast<OTDocStreamerOpenOffice*>((OTDocStreamerOpenOffice*)pArg);

	pThis->m_oMutex->lock();

	if(pThis->getState() != OTDocStreamerState_None && pThis->getState() != OTDocStreamerState_Closed)
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameDocStreamerOpenOffice, "Invalid state(%d)", pThis->getState());
		goto bail;
	}

	pThis->setState(OTDocStreamerState_Opening);

	try
	{
		OUString sConnectionString = OUString::createFromAscii(buildConnectionString(pThis->m_oBridgeInfo->getPresentationSharingLocalPort()).c_str());

		Reference< XComponentContext > xComponentContext(::cppu::defaultBootstrap_InitialComponentContext());

		/* Gets the service manager instance to be used (or null). This method has
		   been added for convenience, because the service manager is a often used
		   object.
		*/
		Reference< XMultiComponentFactory > xMultiComponentFactoryClient(
			xComponentContext->getServiceManager() );

		/* Creates an instance of a component which supports the services specified
		   by the factory.
		*/
		Reference< XInterface > xInterface =
			xMultiComponentFactoryClient->createInstanceWithContext( 
				OUString::createFromAscii( "com.sun.star.bridge.UnoUrlResolver" ),
				xComponentContext );

		Reference< XUnoUrlResolver > resolver( xInterface, UNO_QUERY );

		// Resolves the component context from the office, on the uno URL given by argv[1].
		try
		{    
			xInterface = Reference< XInterface >( 
				resolver->resolve( sConnectionString ), UNO_QUERY );
		}
		catch ( Exception& e )
		{
			OT_DEBUG_ERROR_EX(kOTMobuleNameDocStreamerOpenOffice, "Error: cannot establish a connection using '%s':\n       %s",
				   OUStringToOString(sConnectionString, RTL_TEXTENCODING_ASCII_US).getStr(),
				   OUStringToOString(e.Message, RTL_TEXTENCODING_ASCII_US).getStr());
			goto bail;    
		}

		// gets the server component context as property of the office component factory
		Reference< XPropertySet > xPropSet( xInterface, UNO_QUERY );
		xPropSet->getPropertyValue( OUString::createFromAscii("DefaultContext") ) >>= xComponentContext;

		// gets the service manager from the office
		Reference< XMultiComponentFactory > xMultiComponentFactoryServer(
			xComponentContext->getServiceManager() );

		/* Creates an instance of a component which supports the services specified
		   by the factory. Important: using the office component context.
		*/
		Reference < XComponentLoader > xComponentLoader(
			xMultiComponentFactoryServer->createInstanceWithContext( 
				OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop" ) ),
				xComponentContext ), UNO_QUERY );
		
		/* Loads a component specified by an URL into the specified new or existing
		   frame.
		*/	
		Sequence< PropertyValue > seqPropComponent(1);
		seqPropComponent[0].Name = OUString::createFromAscii("Hidden");
		seqPropComponent[0].Value <<= sal_True;
		
		try
		{
			pThis->m_xComponent = xComponentLoader->loadComponentFromURL(
				pThis->m_strInFileUrl, OUString( RTL_CONSTASCII_USTRINGPARAM("_blank") ), 0,
				seqPropComponent );
		}
		catch ( Exception& e )
		{
			OT_DEBUG_ERROR_EX(kOTMobuleNameDocStreamerOpenOffice, "[Url=%s], %s", 
					OUStringToOString(pThis->m_strInFileUrl, RTL_TEXTENCODING_ASCII_US).getStr(),
					OUStringToOString(e.Message, RTL_TEXTENCODING_ASCII_US).getStr());
			goto bail;
		}

		// Creates Graphic exporter
		try
		{
			xInterface =
				xMultiComponentFactoryClient->createInstanceWithContext( 
					OUString::createFromAscii( "com.sun.star.drawing.GraphicExportFilter" ),
					xComponentContext );
			pThis->m_xExporter = Reference< XExporter > (xInterface, UNO_QUERY);
			pThis->m_xFilter = Reference< XFilter > (xInterface, UNO_QUERY);
		}
		catch ( Exception& e )
		{
			OT_DEBUG_ERROR_EX(kOTMobuleNameDocStreamerOpenOffice, "%s", 
				OUStringToOString(e.Message, RTL_TEXTENCODING_ASCII_US).getStr());
			goto bail;
		}

		// Gets pages supplier and pages
		pThis->m_xDrawPagesSupplier = Reference< XDrawPagesSupplier > (pThis->m_xComponent, UNO_QUERY);
		if(pThis->m_xDrawPagesSupplier == NULL)
		{
			OT_DEBUG_ERROR_EX(kOTMobuleNameDocStreamerOpenOffice, "Failed to get pages supplier for %s", 
				OUStringToOString(pThis->m_strInFileUrl, RTL_TEXTENCODING_ASCII_US).getStr());
			goto bail;
		}
		pThis->m_xDrawPages = pThis->m_xDrawPagesSupplier->getDrawPages();
		pThis->m_nPagesCount = pThis->m_xDrawPages->getCount();

		if(!pThis->m_xseqPropFilter.hasElements( ))
		{
			pThis->m_xseqPropFilter.realloc( 3 );
		}
		
		Sequence< PropertyValue > seqPropFilterData(4);
		
		pThis->m_xseqPropFilter[0].Name = OUString::createFromAscii("MediaType");
		pThis->m_xseqPropFilter[0].Value <<= OUString::createFromAscii("image/jpeg");
		
		seqPropFilterData[0].Name = OUString::createFromAscii("PixelWidth");
		seqPropFilterData[0].Value <<= (sal_Int32)pThis->m_oBridgeInfo->getVideoWidth();
		seqPropFilterData[1].Name = OUString::createFromAscii("PixelHeight");
		seqPropFilterData[1].Value <<= (sal_Int32)pThis->m_oBridgeInfo->getVideoHeight();
		seqPropFilterData[2].Name = OUString::createFromAscii("LogicalWidth");
		seqPropFilterData[2].Value <<= (sal_Int32)pThis->m_oBridgeInfo->getVideoWidth();
		seqPropFilterData[3].Name = OUString::createFromAscii("LogicalHeight");
		seqPropFilterData[3].Value <<= (sal_Int32)pThis->m_oBridgeInfo->getVideoHeight();
		
		pThis->m_xseqPropFilter[1].Name = OUString::createFromAscii("FilterData");
		pThis->m_xseqPropFilter[1].Value <<= seqPropFilterData;
	}
	catch ( Exception& e )
    {
		OT_DEBUG_ERROR_EX(kOTMobuleNameDocStreamerOpenOffice, "%s", 
			OUStringToOString(e.Message, RTL_TEXTENCODING_ASCII_US).getStr());
		goto bail;
    }

	pThis->setState(OTDocStreamerState_Opened);

bail:
	
	if(pThis->getState() != OTDocStreamerState_Opened)
	{
		if(pThis->m_pcCallback)
		{
			pThis->m_pcCallback->onError(__IsErrorFatal, pThis);
		}
		pThis->_close();
	}
	
	pThis->m_oMutex->unlock();

	OT_DEBUG_INFO_EX(kOTMobuleNameDocStreamerOpenOffice, "threadOpen - EXIT");
	return tsk_null;
}

void* TSK_STDCALL OTDocStreamerOpenOffice::threadExport(void *pArg)
{
	OT_DEBUG_INFO_EX(kOTMobuleNameDocStreamerOpenOffice, "threadExport - ENTER");

	static const bool __IsErrorFatal = false;

	OTDocStreamerOpenOffice* pThis = dynamic_cast<OTDocStreamerOpenOffice*>((OTDocStreamerOpenOffice*)pArg);

	pThis->m_oMutex->lock();

	if(!pThis->isOpened() || pThis->getState() == OTDocStreamerState_Exporting)
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameDocStreamerOpenOffice, "Invalid state(%d)", pThis->getState());
		goto bail;
	}

	pThis->setState(OTDocStreamerState_Exporting);

	pThis->m_xDrawPage = Reference< XDrawPage > (pThis->m_xDrawPages->getByIndex(pThis->m_nPageIndex), UNO_QUERY);
	if(pThis->m_xDrawPage == NULL)
	{
		OT_DEBUG_INFO_EX(kOTMobuleNameDocStreamerOpenOffice, "Failed to get page at index = %d", pThis->m_nPageIndex);
		goto bail;
	}
	
    pThis->m_xseqPropFilter[2].Name = OUString::createFromAscii("URL");
    pThis->m_xseqPropFilter[2].Value <<= pThis->m_strJpegFileUrl;
	pThis->m_xExporter->setSourceDocument(Reference< XComponent > (pThis->m_xDrawPage, UNO_QUERY));
	if(!pThis->m_xFilter->filter(pThis->m_xseqPropFilter))
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameDocStreamerOpenOffice, "Failed to export page at index = %d at path = %s", 
			pThis->m_nPageIndex, 
			OUStringToOString(pThis->m_strJpegFileUrl, RTL_TEXTENCODING_ASCII_US).getStr());
		goto bail;
	}

	if(!pThis->_decode())
	{
		OT_DEBUG_ERROR_EX(kOTMobuleNameDocStreamerOpenOffice, "Failed to decode (JPEG->YUV420) picture");
		goto bail;
	}

	pThis->setState(OTDocStreamerState_Exported);

bail:
	if(pThis->getState() != OTDocStreamerState_Exported)
	{
		if(pThis->m_pcCallback)
		{
			pThis->m_pcCallback->onError(__IsErrorFatal, pThis);
		}
	}
	pThis->m_oMutex->unlock();

	OT_DEBUG_INFO_EX(kOTMobuleNameDocStreamerOpenOffice, "threadExport - EXIT");
	return tsk_null;
}

#endif  /* HAVE_OPENOFFICE */
