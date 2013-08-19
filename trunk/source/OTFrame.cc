/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#include "opentelepresence/OTFrame.h"
#include "opentelepresence/OTFrameAudio.h"
#include "opentelepresence/OTFrameVideo.h"

#include "tsk_memory.h"
#include "tsk_debug.h"

#include <string.h>/* memcpy */
#include <assert.h>

OTFrame::OTFrame(OTMediaType_t eMediaType, bool bOwnBuffer, const void *pBufferPtr, uint32_t nBufferSize)
: OTObject()
, m_pBufferPtr(NULL)
, m_nBufferSize(0)
, m_ePatternType(OTPatternType_None)
, m_nValidDataSize(0)
, m_eMediaType(eMediaType)
, m_eDimension(OTDimension_2D)
, m_bOwnBuffer(bOwnBuffer)
, m_phMutex(NULL)
{
	if(nBufferSize)
	{
		if(m_bOwnBuffer)
		{
			if((m_pBufferPtr = tsk_calloc(nBufferSize, sizeof(uint8_t))))
			{
				m_nBufferSize = nBufferSize;
				if(pBufferPtr)
				{
					memcpy(m_pBufferPtr, pBufferPtr, m_nBufferSize);
					m_nValidDataSize = m_nBufferSize;
				}
			}
			else
			{
				m_pBufferPtr = NULL;
				m_nBufferSize = 0;
			}
		}
		else
		{
			m_pBufferPtr = (void*)pBufferPtr;
			m_nBufferSize = m_pBufferPtr ? nBufferSize : 0;
			m_nValidDataSize = m_nBufferSize;
		}
	}
}

OTFrame::~OTFrame()
{
	if(m_bOwnBuffer)
	{
		TSK_FREE(m_pBufferPtr);
	}
	else
	{
		m_pBufferPtr = NULL;
	}
	m_nBufferSize = 0;

	if(m_phMutex)
	{
		tsk_mutex_destroy(&m_phMutex);
	}
}

bool OTFrame::copyBuffer(const void *pSrcBufferPtr, uint32_t nSrcBufferSize)
{
	m_nValidDataSize = 0;
	if(m_bOwnBuffer && m_nBufferSize < nSrcBufferSize)
	{
		if((m_pBufferPtr = tsk_realloc(m_pBufferPtr, nSrcBufferSize)))
		{
			m_nBufferSize = nSrcBufferSize;
		}
		else
		{
			m_pBufferPtr = NULL;
			m_nBufferSize = 0;
		}
	}

	if(m_pBufferPtr)
	{
		m_nValidDataSize = TSK_MIN(m_nBufferSize, nSrcBufferSize);
		memcpy(m_pBufferPtr, pSrcBufferPtr, m_nValidDataSize);
		return true;
	}
	return false;
}

bool OTFrame::resizeBuffer(uint32_t nNewBufferSize)
{
	if(m_nBufferSize == nNewBufferSize)
	{
		return true;
	}

	if(!nNewBufferSize)
	{
		OT_DEBUG_ERROR("%u not valid for buffer size", nNewBufferSize);
		return false;
	}

	if(!m_bOwnBuffer)
	{
		OT_DEBUG_ERROR("You cannot resize the buffer because you are not the owner");
		return false;
	}
	
	if((m_pBufferPtr = tsk_realloc(m_pBufferPtr, nNewBufferSize)))
	{
		m_nBufferSize = nNewBufferSize;
	}
	else
	{
		m_pBufferPtr = NULL;
		m_nBufferSize = 0;
		m_nValidDataSize = 0;
	}
	return (m_nBufferSize == nNewBufferSize);
}

bool OTFrame::prepareLock()
{
	// global lock against lock()/unlock()
	static tsk_mutex_handle_t *__phMutex = tsk_mutex_create_2(tsk_false);

	// not delay-sensitive function
	tsk_mutex_lock(__phMutex);
	if(!m_phMutex)
	{
		m_phMutex = tsk_mutex_create();
	}
	tsk_mutex_unlock(__phMutex);

	return (m_phMutex != NULL);
}

bool OTFrame::lock()
{
	if(!m_phMutex)
	{
		prepareLock();
	}
	return (tsk_mutex_lock(m_phMutex) == 0);
}

bool OTFrame::unlock()
{
	// OT_ASSERT(m_phMutex);
	return (tsk_mutex_unlock(m_phMutex) == 0);
}

OTObjectWrapper<OTFrame*> OTFrame::New(OTMediaType_t eMediaType, bool bOwnBuffer, const void *pBufferPtr, uint32_t nBufferSize)
{
	switch(eMediaType)
	{
		case OTMediaType_Audio:
			{
				OTObjectWrapper<OTFrameAudio*> pOTFrameAudio = OTFrameAudio::New(bOwnBuffer, pBufferPtr, nBufferSize);
				if(pOTFrameAudio)
				{
					if(!pOTFrameAudio->isValid())
					{
						OTObjectSafeRelease(pOTFrameAudio);
					}
					else
					{
						return dynamic_cast<OTFrame*>(*pOTFrameAudio);
					}
				}
				break;
			}

		case OTMediaType_Video:
		{
			OTObjectWrapper<OTFrameVideo*> pOTFrameVideo = OTFrameVideo::New(bOwnBuffer, pBufferPtr, nBufferSize);
			if(pOTFrameVideo)
			{
				if(!pOTFrameVideo->isValid())
				{
					OTObjectSafeRelease(pOTFrameVideo);
				}
				else
				{
					return dynamic_cast<OTFrame*>(*pOTFrameVideo);
				}
			}
			break;
		}
	}

	return NULL;
}
