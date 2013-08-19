/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_CODEC_H
#define OPENTELEPRESENCE_CODEC_H

#include "OpenTelepresenceConfig.h"
#include "opentelepresence/OTObject.h"
#include "opentelepresence/OTCommon.h"

#if HAVE_HARTALLO_H
#	include <hartallo.h>
#endif

#include "tinymedia/tmedia_codec.h"
#include "tinymedia/tmedia_common.h"
#include "tinydav/tdav.h"

#if !defined(OT_VIDEO_MAX_CHUNKS_PER_FRAME)
#	define OT_VIDEO_MAX_CHUNKS_PER_FRAME 0xFF /* maximum number of video chuncks per frame (depends on the MTU) */
#endif /* OT_VIDEO_MAX_CHUNKS_PER_FRAME */


// all OT codec types must match TDAV types (because of casts elsewhere)
typedef enum OTCodecType_e
{
	OTCodecType_None = tmedia_codec_id_none,

	OTCodecType_VP8 = tmedia_codec_id_vp8,
	OTCodecType_H264Base = tmedia_codec_id_h264_bp,
	OTCodecType_H264Main = tmedia_codec_id_h264_mp,
#if HAVE_HARTALLO_H
	OTCodecType_H264SVC = tdav_codec_id_h264_svc,
#endif
	OTCodecType_H263 = tmedia_codec_id_h263,
	OTCodecType_H263P = tmedia_codec_id_h263p,
	OTCodecType_H263PP = tmedia_codec_id_h263pp
}
OTCodec_Type_t;

typedef enum OTCodecAction_e
{
	OTCodecAction_None,
	OTCodecAction_ForceIntra,
	OTCodecAction_FlushEncodedData
}
OTCodecAction_t;


//
//	OTCodecEncodingResult
//

class OTCodecEncodingResult : public OTObject
{
public:
	OTCodecEncodingResult(const void* pDataPtr, size_t nDataSize, uint32_t nDuration, bool bLastResultInFrame);
	virtual ~OTCodecEncodingResult();
	virtual OT_INLINE const char* getObjectId() { return "OTCodecEncodingResult"; }
	virtual OT_INLINE const void* getDataPtr(){ return m_pDataPtr; }
	virtual OT_INLINE size_t getDataEncodedSize(){ return m_nDataEncodedSize; }
	virtual OT_INLINE uint32_t getDuration(){ return m_nDuration; }
	virtual OT_INLINE bool isLastResultInFrame(){ return m_bLastResultInFrame; }

	virtual bool update(const void* pDataPtr, size_t nDataSize, uint32_t nDuration, bool bLastResultInFrame);

private:
	void* m_pDataPtr;
	size_t m_nDataSize, m_nDataEncodedSize;
	uint32_t m_nDuration;
	bool m_bLastResultInFrame;
};

//
//	Codec base type
//

class OTCodec : public OTObject
{
protected:
	OTCodec(OTCodec_Type_t eType);
public:
	virtual ~OTCodec();
	virtual OT_INLINE const char* getObjectId() { return "OTCodec"; }

	virtual OT_INLINE OTCodec_Type_t getType() { return m_eType; }
	virtual OT_INLINE OTMediaType_t getMediaType() { 
		switch(m_eType){
			case OTCodecType_VP8:
			case OTCodecType_H264Base:
			case OTCodecType_H264Main:
#if HAVE_HARTALLO_H
			case OTCodecType_H264SVC
#endif
				return OTMediaType_Video;
			default:
				return OTMediaType_None;
		}
	}
	virtual OT_INLINE bool isOpened() { return m_bOpened; }

	virtual bool isValid() { return (m_pWrappedCodec != NULL); };
	virtual bool open();
	virtual bool close();
	virtual bool encode(const void* pcInData, size_t nInSize, void** ppOutData = NULL, size_t* pOutMaxSize = NULL) = 0;
	virtual bool executeAction(OTCodecAction_t eAction) = 0;

protected:
	OTCodec_Type_t m_eType;
	bool m_bOpened;
	tmedia_codec_t *m_pWrappedCodec;
};

//
// Video Codec
//
class OTCodecVideo : public OTCodec
{
public:
	OTCodecVideo(OTCodec_Type_t eType, size_t nWidth, size_t nHeight);
	virtual ~OTCodecVideo();
	static OTObjectWrapper<OTCodecVideo*> createBestCodec(OTCodec_Type_t eListOfCodecsToSearchInto, size_t nWidth, size_t nHeight);
	virtual OT_INLINE bool flushEncodedData() { m_nResultsCount = 0; return true; } // @Override
	virtual bool encode(const void* pcInData, size_t nInSize, void** ppOutData = NULL, size_t* pOutMaxSize = NULL); //  @Override
	virtual bool executeAction(OTCodecAction_t eAction); //  @Override
	virtual OT_INLINE bool hasResults() { return m_nResultsCount > 0; } //  @Override
	virtual OT_INLINE OTObjectWrapper<OTCodecEncodingResult*> getEncodingResultAt(size_t nChunkIndex)
	{
		if(nChunkIndex < m_nResultsCount)
		{
			return m_aResults[nChunkIndex];
		}
		return NULL;
	}

	static OTObjectWrapper<OTCodec*> OTNew(OTCodec_Type_t eType, size_t nWidth, size_t nHeight);

private:
	static int encoderResultCb(const tmedia_video_encode_result_xt* pcResult);
	OTObjectWrapper<OTCodecEncodingResult*> m_aResults[OT_VIDEO_MAX_CHUNKS_PER_FRAME];
	size_t m_nResultsCount;
};




//
//	H.264SVC codec
//
#if HAVE_TOULDE_H
#endif /* HAVE_TOULDE_H */


#endif /* OPENTELEPRESENCE_CODEC_H */
