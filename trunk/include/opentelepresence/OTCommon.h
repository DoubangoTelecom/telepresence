#ifndef OPENTELEPRESENCE_COMMON_H
#define OPENTELEPRESENCE_COMMON_H

#include "OpenTelepresenceConfig.h"

#define OT_CAT_(A, B) A ## B
#define OT_CAT(A, B) OT_CAT_(A, B)
#define OT_STRING_(A) #A
#define OT_STRING(A) OT_STRING_(A)

#if !defined(OT_SAFE_DELETE_CPP)
#	define OT_SAFE_DELETE_CPP(cpp_obj) if(cpp_obj) delete (cpp_obj), (cpp_obj) = NULL;
#endif

#if defined(NDEBUG)
#       define OT_ASSERT(x) (void)(x)
#else
#       define OT_ASSERT(x) assert(x)
#endif

typedef int32_t OTNetFd;
#define OTNetFd_IsValid(self)	((self) > 0)
#define kOTNetFdInvalid			-1

#define kOTMobuleNameCfg "CFG"
#define kOTMobuleNameWebM "WebM"
#define kOTMobuleNameOpenAL "OpenAL Soft"
#define kOTMobuleNameFFmpegRecorder "FFmpegRecorder"
#define kOTMobuleNameFFmpegOverlay "FFmpegOverlay"
#define kOTMobuleNameFFmpegFilter "FFmpegOverlay"
#define kOTMobuleNameFFmpegResampler "FFmpegResampler"
#define kOTMobuleNameDocStreamer "DocStreamer"
#define kOTMobuleNameDocStreamerOpenOffice "DocStreamerOpenOffice"
#define kOTMobuleNameExperimental "Experimental"
#define kOTMobuleNameNetTransport "NetTransport"
#define kOTMobuleNameHttpTransport "HttpTransport"

#define kJsonContentType "application/json"
#define kFileContentType "application/file"

#if !defined(kOTSipHeaderServer)
#	define	kOTSipHeaderServer	"Doubango Telepresence " OT_VERSION_STRING
#endif

typedef enum OTSessionState_e
{
	OTSessionState_None,
	OTSessionState_Connecting,
	OTSessionState_Connected,
	OTSessionState_Terminated
}
OTSessionState_t;

typedef enum OTMediaType_e
{
	OTMediaType_None = 0x00,
	OTMediaType_Audio = (0x01<<0),
	OTMediaType_Video = (0x01<<1),
	OTMediaType_AudioVideo = (OTMediaType_Audio | OTMediaType_Video),

	OTMediaType_All = (OTMediaType_AudioVideo),
}
OTMediaType_t;

typedef enum OTDimension_e
{
	OTDimension_None = 0x00,
	OTDimension_2D = 0x01<<1,
	OTDimension_3D = 0x01<<2
}
OTDimension_t;

typedef enum OTPatternType_e
{
	OTPatternType_None = 0x0000,
	
	OTPatternType_Hangout,
	OTPatternType_WebEx,
}
OTPatternType_t;

typedef struct OT3f_s
{
	float x;
	float y;
	float z;
}
OT3f_t;

typedef struct OTRect_s
{
    size_t    nLeft;
    size_t    nTop;
    size_t    nRight;
    size_t    nBottom;
} 
OTRect_t;
#define OTRectWidth(r) ((r).nRight - (r).nLeft)
#define OTRectHeight(r) ((r).nBottom - (r).nTop)

typedef struct OTRatio_s
{
    size_t nNumerator;
    size_t nDenominator;
}
OTRatio_t;

typedef enum OTDocStreamerState_e
{
	OTDocStreamerState_None = 0x00,
	OTDocStreamerState_Opening = (0x01 << 1),
	OTDocStreamerState_Opened = (0x01 << 2),
	OTDocStreamerState_Exporting = (0x01 << 3) | OTDocStreamerState_Opened,
	OTDocStreamerState_Exported = (0x01 << 4) | OTDocStreamerState_Opened,
	OTDocStreamerState_Closed = (0x01 << 5)
}
OTDocStreamerState_t;

typedef enum OTDocStreamerType_e
{
	OTDocStreamerType_None,
	OTDocStreamerType_OpenOffice
}
OTDocStreamerType_t;

typedef enum OTNetTransporType_e
{
	OTNetTransporType_None,
	OTNetTransporType_TCP,
	OTNetTransporType_TLS
}
OTNetTransporType_t;

static bool OTNetTransporType_isStream(OTNetTransporType_t eType)
{
	switch(eType)
	{
	case OTNetTransporType_TCP:
	case OTNetTransporType_TLS:
		return true;
	default:
		return false;
	}
}

typedef enum OTHttpActionType_e
{
	OTHttpActionType_None,
	OTHttpActionType_UploadPresensation
}
OTHttpActionType_t;

#endif /* OPENTELEPRESENCE_COMMON_H */
