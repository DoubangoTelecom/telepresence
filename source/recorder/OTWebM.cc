/* Copyright (C) WebM Project */
/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#include "opentelepresence/recorder/OTWebM.h"
#include "opentelepresence/OTCommon.h"

#include <assert.h>
#include <stdio.h>
#include <limits.h> /* SHRT_MAX..... */
#include <stdio.h>
#include <wchar.h>
#include <string.h>

#if !defined(VPX_CODEC_VERSION_STR)
#define VPX_CODEC_VERSION_STR "v1.1.0-222-g7e9a519"
#endif

/* Need special handling of these functions on Windows */
#if defined(_MSC_VER)
/* MSVS doesn't define off_t, and uses _f{seek,tell}i64 */
//--typedef __int64 off_t;
#define fseeko _fseeki64
#define ftello _ftelli64
#elif defined(_WIN32)
/* MinGW defines off_t as long
   and uses f{seek,tell}o64/off64_t for large files */
#define fseeko fseeko64
#define ftello ftello64
#define off_t off64_t
#endif

#define LITERALU64(hi,lo) ((((uint64_t)hi)<<32)|lo)

/* We should use 32-bit file operations in WebM file format
 * when building ARM executable file (.axf) with RVCT */
#if !CONFIG_OS_SUPPORT && 0
typedef long off_t;
#define fseeko fseek
#define ftello ftell
#endif

typedef off_t EbmlLoc;


enum mkv {
  EBML = 0x1A45DFA3,
  EBMLVersion = 0x4286,
  EBMLReadVersion = 0x42F7,
  EBMLMaxIDLength = 0x42F2,
  EBMLMaxSizeLength = 0x42F3,
  DocType = 0x4282,
  DocTypeVersion = 0x4287,
  DocTypeReadVersion = 0x4285,
/* CRC_32 = 0xBF, */
  Void = 0xEC,
  SignatureSlot = 0x1B538667,
  SignatureAlgo = 0x7E8A,
  SignatureHash = 0x7E9A,
  SignaturePublicKey = 0x7EA5,
  Signature = 0x7EB5,
  SignatureElements = 0x7E5B,
  SignatureElementList = 0x7E7B,
  SignedElement = 0x6532,
  /* segment */
  Segment = 0x18538067,
  /* Meta Seek Information */
  SeekHead = 0x114D9B74,
  Seek = 0x4DBB,
  SeekID = 0x53AB,
  SeekPosition = 0x53AC,
  /* Segment Information */
  Info = 0x1549A966,
/* SegmentUID = 0x73A4, */
/* SegmentFilename = 0x7384, */
/* PrevUID = 0x3CB923, */
/* PrevFilename = 0x3C83AB, */
/* NextUID = 0x3EB923, */
/* NextFilename = 0x3E83BB, */
/* SegmentFamily = 0x4444, */
/* ChapterTranslate = 0x6924, */
/* ChapterTranslateEditionUID = 0x69FC, */
/* ChapterTranslateCodec = 0x69BF, */
/* ChapterTranslateID = 0x69A5, */
  TimecodeScale = 0x2AD7B1,
  Segment_Duration = 0x4489,
  DateUTC = 0x4461,
/* Title = 0x7BA9, */
  MuxingApp = 0x4D80,
  WritingApp = 0x5741,
  /* Cluster */
  Cluster = 0x1F43B675,
  Timecode = 0xE7,
/* SilentTracks = 0x5854, */
/* SilentTrackNumber = 0x58D7, */
/* Position = 0xA7, */
  PrevSize = 0xAB,
  BlockGroup = 0xA0,
  Block = 0xA1,
/* BlockVirtual = 0xA2, */
  BlockAdditions = 0x75A1,
  BlockMore = 0xA6,
  BlockAddID = 0xEE,
  BlockAdditional = 0xA5,
  BlockDuration = 0x9B,
/* ReferencePriority = 0xFA, */
  ReferenceBlock = 0xFB,
/* ReferenceVirtual = 0xFD, */
/* CodecState = 0xA4, */
/* Slices = 0x8E, */
/* TimeSlice = 0xE8, */
  LaceNumber = 0xCC,
/* FrameNumber = 0xCD, */
/* BlockAdditionID = 0xCB, */
/* MkvDelay = 0xCE, */
/* Cluster_Duration = 0xCF, */
  SimpleBlock = 0xA3,
/* EncryptedBlock = 0xAF, */
  /* Track */
  Tracks = 0x1654AE6B,
  TrackEntry = 0xAE,
  TrackNumber = 0xD7,
  TrackUID = 0x73C5,
  TrackType = 0x83,
  FlagEnabled = 0xB9,
  FlagDefault = 0x88,
  FlagForced = 0x55AA,
  FlagLacing = 0x9C,
/* MinCache = 0x6DE7, */
/* MaxCache = 0x6DF8, */
  DefaultDuration = 0x23E383,
/* TrackTimecodeScale = 0x23314F, */
/* TrackOffset = 0x537F, */
  MaxBlockAdditionID = 0x55EE,
  Name = 0x536E,
  Language = 0x22B59C,
  CodecID = 0x86,
  CodecPrivate = 0x63A2,
  CodecName = 0x258688,
/* AttachmentLink = 0x7446, */
/* CodecSettings = 0x3A9697, */
/* CodecInfoURL = 0x3B4040, */
/* CodecDownloadURL = 0x26B240, */
/* CodecDecodeAll = 0xAA, */
/* TrackOverlay = 0x6FAB, */
/* TrackTranslate = 0x6624, */
/* TrackTranslateEditionUID = 0x66FC, */
/* TrackTranslateCodec = 0x66BF, */
/* TrackTranslateTrackID = 0x66A5, */
  /* video */
  Video = 0xE0,
  FlagInterlaced = 0x9A,
  StereoMode = 0x53B8,
  AlphaMode = 0x53C0,
  PixelWidth = 0xB0,
  PixelHeight = 0xBA,
  PixelCropBottom = 0x54AA,
  PixelCropTop = 0x54BB,
  PixelCropLeft = 0x54CC,
  PixelCropRight = 0x54DD,
  DisplayWidth = 0x54B0,
  DisplayHeight = 0x54BA,
  DisplayUnit = 0x54B2,
  AspectRatioType = 0x54B3,
/* ColourSpace = 0x2EB524, */
/* GammaValue = 0x2FB523, */
  FrameRate = 0x2383E3,
  /* end video */
  /* audio */
  Audio = 0xE1,
  SamplingFrequency = 0xB5,
  OutputSamplingFrequency = 0x78B5,
  Channels = 0x9F,
/* ChannelPositions = 0x7D7B, */
  BitDepth = 0x6264,
  /* end audio */
  /* content encoding */
/* ContentEncodings = 0x6d80, */
/* ContentEncoding = 0x6240, */
/* ContentEncodingOrder = 0x5031, */
/* ContentEncodingScope = 0x5032, */
/* ContentEncodingType = 0x5033, */
/* ContentCompression = 0x5034, */
/* ContentCompAlgo = 0x4254, */
/* ContentCompSettings = 0x4255, */
/* ContentEncryption = 0x5035, */
/* ContentEncAlgo = 0x47e1, */
/* ContentEncKeyID = 0x47e2, */
/* ContentSignature = 0x47e3, */
/* ContentSigKeyID = 0x47e4, */
/* ContentSigAlgo = 0x47e5, */
/* ContentSigHashAlgo = 0x47e6, */
  /* end content encoding */
  /* Cueing Data */
  Cues = 0x1C53BB6B,
  CuePoint = 0xBB,
  CueTime = 0xB3,
  CueTrackPositions = 0xB7,
  CueTrack = 0xF7,
  CueClusterPosition = 0xF1,
  CueBlockNumber = 0x5378
/* CueCodecState = 0xEA, */
/* CueReference = 0xDB, */
/* CueRefTime = 0x96, */
/* CueRefCluster = 0x97, */
/* CueRefNumber = 0x535F, */
/* CueRefCodecState = 0xEB, */
  /* Attachment */
/* Attachments = 0x1941A469, */
/* AttachedFile = 0x61A7, */
/* FileDescription = 0x467E, */
/* FileName = 0x466E, */
/* FileMimeType = 0x4660, */
/* FileData = 0x465C, */
/* FileUID = 0x46AE, */
/* FileReferral = 0x4675, */
  /* Chapters */
/* Chapters = 0x1043A770, */
/* EditionEntry = 0x45B9, */
/* EditionUID = 0x45BC, */
/* EditionFlagHidden = 0x45BD, */
/* EditionFlagDefault = 0x45DB, */
/* EditionFlagOrdered = 0x45DD, */
/* ChapterAtom = 0xB6, */
/* ChapterUID = 0x73C4, */
/* ChapterTimeStart = 0x91, */
/* ChapterTimeEnd = 0x92, */
/* ChapterFlagHidden = 0x98, */
/* ChapterFlagEnabled = 0x4598, */
/* ChapterSegmentUID = 0x6E67, */
/* ChapterSegmentEditionUID = 0x6EBC, */
/* ChapterPhysicalEquiv = 0x63C3, */
/* ChapterTrack = 0x8F, */
/* ChapterTrackNumber = 0x89, */
/* ChapterDisplay = 0x80, */
/* ChapString = 0x85, */
/* ChapLanguage = 0x437C, */
/* ChapCountry = 0x437E, */
/* ChapProcess = 0x6944, */
/* ChapProcessCodecID = 0x6955, */
/* ChapProcessPrivate = 0x450D, */
/* ChapProcessCommand = 0x6911, */
/* ChapProcessTime = 0x6922, */
/* ChapProcessData = 0x6933, */
  /* Tagging */
/* Tags = 0x1254C367, */
/* Tag = 0x7373, */
/* Targets = 0x63C0, */
/* TargetTypeValue = 0x68CA, */
/* TargetType = 0x63CA, */
/* Tagging_TrackUID = 0x63C5, */
/* Tagging_EditionUID = 0x63C9, */
/* Tagging_ChapterUID = 0x63C4, */
/* AttachmentUID = 0x63C6, */
/* SimpleTag = 0x67C8, */
/* TagName = 0x45A3, */
/* TagLanguage = 0x447A, */
/* TagDefault = 0x4484, */
/* TagString = 0x4487, */
/* TagBinary = 0x4485, */
};

struct cue_entry
{
    unsigned int time;
    uint64_t     loc;
};

struct EbmlGlobal
{
    int debug;

    FILE    *stream; // not owning
    int64_t last_pts_ms;
    EbmlRational  framerate;

    /* These pointers are to the start of an element */
    off_t    position_reference;
    off_t    seek_info_pos;
    off_t    segment_info_pos;
    off_t    track_pos;
    off_t    cue_pos;
    off_t    cluster_pos;

    /* This pointer is to a specific element to be serialized */
    off_t    track_id_pos;

    /* These pointers are to the size field of the element */
    EbmlLoc  startSegment;
    EbmlLoc  startCluster;

    uint32_t cluster_timecode;
    int      cluster_open;

    struct cue_entry *cue_list;
    unsigned int      cues;

};


static void Ebml_Serialize(EbmlGlobal *glob, const void *buffer_in, int buffer_size, unsigned long len);
static void Ebml_Write(EbmlGlobal *glob, const void *buffer_in, unsigned long len);

void Ebml_Init(struct EbmlGlobal** ppGlob, FILE* pFile)
{
	OT_ASSERT(ppGlob && !*ppGlob);
	OT_ASSERT((*ppGlob = (struct EbmlGlobal*)calloc(sizeof(struct EbmlGlobal), 1)));
	(*ppGlob)->stream = pFile;
}

void Ebml_DeInit(struct EbmlGlobal** ppGlob)
{
	if(ppGlob && *ppGlob)
	{
		if((*ppGlob)->cue_list)
		{
			free((*ppGlob)->cue_list);
			(*ppGlob)->cue_list = NULL;
		}
		free((*ppGlob));
		(*ppGlob) = NULL;
	}
}

static void Ebml_WriteLen(EbmlGlobal *glob, int64_t val)
{
    /* TODO check and make sure we are not > than 0x0100000000000000LLU */
    unsigned char size = 8; /* size in bytes to output */

    /* mask to compare for byte size */
    int64_t minVal = 0xff;

    for (size = 1; size < 8; size ++)
    {
        if (val < minVal)
            break;

        minVal = (minVal << 7);
    }

    val |= (((uint64_t)0x80) << ((size - 1) * 7));

    Ebml_Serialize(glob, (void *) &val, sizeof(val), size);
}

static void Ebml_WriteString(EbmlGlobal *glob, const char *str)
{
    const size_t size_ = strlen(str);
    const uint64_t  size = size_;
    Ebml_WriteLen(glob, size);
    /* TODO: it's not clear from the spec whether the nul terminator
     * should be serialized too.  For now we omit the null terminator.
     */
    Ebml_Write(glob, str, (unsigned long)size);
}

static void Ebml_WriteUTF8(EbmlGlobal *glob, const wchar_t *wstr)
{
    const size_t strlen = wcslen(wstr);

    /* TODO: it's not clear from the spec whether the nul terminator
     * should be serialized too.  For now we include it.
     */
    const uint64_t  size = strlen;

    Ebml_WriteLen(glob, size);
    Ebml_Write(glob, wstr, (unsigned long)size);
}

static void Ebml_WriteID(EbmlGlobal *glob, unsigned long class_id)
{
    int len;

    if (class_id >= 0x01000000)
        len = 4;
    else if (class_id >= 0x00010000)
        len = 3;
    else if (class_id >= 0x00000100)
        len = 2;
    else
        len = 1;

    Ebml_Serialize(glob, (void *)&class_id, sizeof(class_id), len);
}

static void Ebml_SerializeUnsigned64(EbmlGlobal *glob, unsigned long class_id, uint64_t ui)
{
    unsigned char sizeSerialized = 8 | 0x80;
    Ebml_WriteID(glob, class_id);
    Ebml_Serialize(glob, &sizeSerialized, sizeof(sizeSerialized), 1);
    Ebml_Serialize(glob, &ui, sizeof(ui), 8);
}

static void Ebml_SerializeUnsigned(EbmlGlobal *glob, unsigned long class_id, unsigned long ui)
{
    unsigned char size = 8; /* size in bytes to output */
    unsigned char sizeSerialized = 0;
    unsigned long minVal;

    Ebml_WriteID(glob, class_id);
    minVal = 0x7fLU; /* mask to compare for byte size */

    for (size = 1; size < 4; size ++)
    {
        if (ui < minVal)
        {
            break;
        }

        minVal <<= 7;
    }

    sizeSerialized = 0x80 | size;
    Ebml_Serialize(glob, &sizeSerialized, sizeof(sizeSerialized), 1);
    Ebml_Serialize(glob, &ui, sizeof(ui), size);
}
/* TODO: perhaps this is a poor name for this id serializer helper function */
static void Ebml_SerializeBinary(EbmlGlobal *glob, unsigned long class_id, unsigned long bin)
{
    int size;
    for (size=4; size > 1; size--)
    {
        if (bin & 0x000000ff << ((size-1) * 8))
            break;
    }
    Ebml_WriteID(glob, class_id);
    Ebml_WriteLen(glob, size);
    Ebml_WriteID(glob, bin);
}

static void Ebml_SerializeFloat(EbmlGlobal *glob, unsigned long class_id, double d)
{
    unsigned char len = 0x88;

    Ebml_WriteID(glob, class_id);
    Ebml_Serialize(glob, &len, sizeof(len), 1);
    Ebml_Serialize(glob,  &d, sizeof(d), 8);
}

static void Ebml_WriteSigned16(EbmlGlobal *glob, short val)
{
    signed long out = ((val & 0x003FFFFF) | 0x00200000) << 8;
    Ebml_Serialize(glob, &out, sizeof(out), 3);
}

static void Ebml_SerializeString(EbmlGlobal *glob, unsigned long class_id, const char *s)
{
    Ebml_WriteID(glob, class_id);
    Ebml_WriteString(glob, s);
}

static void Ebml_SerializeUTF8(EbmlGlobal *glob, unsigned long class_id, wchar_t *s)
{
    Ebml_WriteID(glob,  class_id);
    Ebml_WriteUTF8(glob,  s);
}

static void Ebml_SerializeData(EbmlGlobal *glob, unsigned long class_id, unsigned char *data, unsigned long data_length)
{
    Ebml_WriteID(glob, class_id);
    Ebml_WriteLen(glob, data_length);
    Ebml_Write(glob,  data, data_length);
}

static void Ebml_WriteVoid(EbmlGlobal *glob, unsigned long vSize)
{
    unsigned char tmp = 0;
    unsigned long i = 0;

    Ebml_WriteID(glob, 0xEC);
    Ebml_WriteLen(glob, vSize);

    for (i = 0; i < vSize; i++)
    {
        Ebml_Write(glob, &tmp, 1);
    }
}


static void Ebml_Write(EbmlGlobal *glob, const void *buffer_in, unsigned long len)
{
    (void) fwrite(buffer_in, 1, len, glob->stream);
}

#define WRITE_BUFFER(s) \
for(i = len-1; i>=0; i--)\
{ \
    x = (char)(*(const s *)buffer_in >> (i * CHAR_BIT)); \
    Ebml_Write(glob, &x, 1); \
}
static void Ebml_Serialize(EbmlGlobal *glob, const void *buffer_in, int buffer_size, unsigned long len)
{
    char x;
    int i;

    /* buffer_size:
     * 1 - int8_t;
     * 2 - int16_t;
     * 3 - int32_t;
     * 4 - int64_t;
     */
    switch (buffer_size)
    {
        case 1:
            WRITE_BUFFER(int8_t)
            break;
        case 2:
            WRITE_BUFFER(int16_t)
            break;
        case 4:
            WRITE_BUFFER(int32_t)
            break;
        case 8:
            WRITE_BUFFER(int64_t)
            break;
        default:
            break;
    }
}
#undef WRITE_BUFFER

/* Need a fixed size serializer for the track ID. libmkv provides a 64 bit
 * one, but not a 32 bit one.
 */
static void Ebml_SerializeUnsigned32(EbmlGlobal *glob, unsigned long class_id, uint64_t ui)
{
    unsigned char sizeSerialized = 4 | 0x80;
    Ebml_WriteID(glob, class_id);
    Ebml_Serialize(glob, &sizeSerialized, sizeof(sizeSerialized), 1);
    Ebml_Serialize(glob, &ui, sizeof(ui), 4);
}


static void
Ebml_StartSubElement(EbmlGlobal *glob, EbmlLoc *ebmlLoc,
                          unsigned long class_id)
{
    /* todo this is always taking 8 bytes, this may need later optimization */
    /* this is a key that says length unknown */
    uint64_t unknownLen = LITERALU64(0x01FFFFFF, 0xFFFFFFFF);

    Ebml_WriteID(glob, class_id);
    *ebmlLoc = (EbmlLoc)ftello(glob->stream);
    Ebml_Serialize(glob, &unknownLen, sizeof(unknownLen), 8);
}

static void
Ebml_EndSubElement(EbmlGlobal *glob, EbmlLoc *ebmlLoc)
{
    off_t pos;
    uint64_t size;

    /* Save the current stream pointer */
    pos = (EbmlLoc)ftello(glob->stream);

    /* Calculate the size of this element */
    size = pos - *ebmlLoc - 8;
    size |= LITERALU64(0x01000000,0x00000000);

    /* Seek back to the beginning of the element and write the new size */
    fseeko(glob->stream, *ebmlLoc, SEEK_SET);
    Ebml_Serialize(glob, &size, sizeof(size), 8);

    /* Reset the stream pointer */
    fseeko(glob->stream, pos, SEEK_SET);
}


static void
write_webm_seek_element(EbmlGlobal *ebml, unsigned long id, off_t pos)
{
    uint64_t offset = pos - ebml->position_reference;
    EbmlLoc start;
    Ebml_StartSubElement(ebml, &start, Seek);
    Ebml_SerializeBinary(ebml, SeekID, id);
    Ebml_SerializeUnsigned64(ebml, SeekPosition, offset);
    Ebml_EndSubElement(ebml, &start);
}


static void
write_webm_seek_info(EbmlGlobal *ebml)
{

    off_t pos;

    /* Save the current stream pointer */
    pos = (EbmlLoc)ftello(ebml->stream);

    if(ebml->seek_info_pos)
        fseeko(ebml->stream, ebml->seek_info_pos, SEEK_SET);
    else
        ebml->seek_info_pos = pos;

    {
        EbmlLoc start;

        Ebml_StartSubElement(ebml, &start, SeekHead);
        write_webm_seek_element(ebml, Tracks, ebml->track_pos);
        write_webm_seek_element(ebml, Cues,   ebml->cue_pos);
        write_webm_seek_element(ebml, Info,   ebml->segment_info_pos);
        Ebml_EndSubElement(ebml, &start);
    }
    {
        /* segment info */
        EbmlLoc startInfo;
        uint64_t frame_time;
        char version_string[64];

        /* Assemble version string */
        if(ebml->debug)
            strcpy(version_string, "vpxenc");
        else
        {
            strcpy(version_string, "vpxenc ");
            strncat(version_string,
                    VPX_CODEC_VERSION_STR,
                    sizeof(version_string) - 1 - strlen(version_string));
        }

        frame_time = (uint64_t)1000 * ebml->framerate.den
                     / ebml->framerate.num;
        ebml->segment_info_pos = (off_t)ftello(ebml->stream);
        Ebml_StartSubElement(ebml, &startInfo, Info);
        Ebml_SerializeUnsigned(ebml, TimecodeScale, 1000000);
        Ebml_SerializeFloat(ebml, Segment_Duration,
                            (double)(ebml->last_pts_ms + frame_time));
        Ebml_SerializeString(ebml, 0x4D80, version_string);
        Ebml_SerializeString(ebml, 0x5741, version_string);
        Ebml_EndSubElement(ebml, &startInfo);
    }
}


/*static*/ void
write_webm_file_header(EbmlGlobal                *glob,
                       const struct EbmlRational *fps,
					   unsigned int pixelWidth,
					   unsigned int pixelHeight,
                       stereo_format_t            stereo_fmt)
{
    {
        EbmlLoc start;
        Ebml_StartSubElement(glob, &start, EBML);
        Ebml_SerializeUnsigned(glob, EBMLVersion, 1);
        Ebml_SerializeUnsigned(glob, EBMLReadVersion, 1);
        Ebml_SerializeUnsigned(glob, EBMLMaxIDLength, 4);
        Ebml_SerializeUnsigned(glob, EBMLMaxSizeLength, 8);
        Ebml_SerializeString(glob, DocType, "webm");
        Ebml_SerializeUnsigned(glob, DocTypeVersion, 2);
        Ebml_SerializeUnsigned(glob, DocTypeReadVersion, 2);
        Ebml_EndSubElement(glob, &start);
    }
    {
        Ebml_StartSubElement(glob, &glob->startSegment, Segment);
        glob->position_reference = (off_t)ftello(glob->stream);
        glob->framerate = *fps;
        write_webm_seek_info(glob);

        {
            EbmlLoc trackStart;
            glob->track_pos = (off_t)ftello(glob->stream);
            Ebml_StartSubElement(glob, &trackStart, Tracks);
            {
                unsigned int trackNumber = 1;
                uint64_t     trackID = 0;

                EbmlLoc start;
                Ebml_StartSubElement(glob, &start, TrackEntry);
                Ebml_SerializeUnsigned(glob, TrackNumber, trackNumber);
                glob->track_id_pos = (off_t)ftello(glob->stream);
                Ebml_SerializeUnsigned32(glob, TrackUID, trackID);
                Ebml_SerializeUnsigned(glob, TrackType, 1);
                Ebml_SerializeString(glob, CodecID, "V_VP8");
                {
                    float        frameRate   = (float)fps->num/(float)fps->den;

                    EbmlLoc videoStart;
                    Ebml_StartSubElement(glob, &videoStart, Video);
                    Ebml_SerializeUnsigned(glob, PixelWidth, pixelWidth);
                    Ebml_SerializeUnsigned(glob, PixelHeight, pixelHeight);
                    Ebml_SerializeUnsigned(glob, StereoMode, stereo_fmt);
                    Ebml_SerializeFloat(glob, FrameRate, frameRate);
                    Ebml_EndSubElement(glob, &videoStart);
                }
                Ebml_EndSubElement(glob, &start); /* Track Entry */
            }
            Ebml_EndSubElement(glob, &trackStart);
        }
        /* segment element is open */
    }
}


/*static*/ void
write_webm_block(EbmlGlobal                *glob,
				 bool isInvisible,
				 bool isKeyFrame,
				 unsigned pts,
				 EbmlRational fps,
                 const void* dataPtr, size_t dataSize)
{
    unsigned long  block_length;
    unsigned char  track_number;
    unsigned short block_timecode = 0;
    unsigned char  flags;
    int64_t        pts_ms;
    int            start_cluster = 0;

    /* Calculate the PTS of this frame in milliseconds */
    pts_ms = pts * 1000
             * (uint64_t)fps.num / (uint64_t)fps.den;
    if(pts_ms <= glob->last_pts_ms)
        pts_ms = glob->last_pts_ms + 1;
    glob->last_pts_ms = pts_ms;

    /* Calculate the relative time of this block */
    if(pts_ms - glob->cluster_timecode > SHRT_MAX)
        start_cluster = 1;
    else
        block_timecode = (unsigned short)pts_ms - glob->cluster_timecode;
    
    if(start_cluster || isKeyFrame)
    {
        if(glob->cluster_open)
            Ebml_EndSubElement(glob, &glob->startCluster);

        /* Open the new cluster */
        block_timecode = 0;
        glob->cluster_open = 1;
        glob->cluster_timecode = (uint32_t)pts_ms;
        glob->cluster_pos = (off_t)ftello(glob->stream);
        Ebml_StartSubElement(glob, &glob->startCluster, Cluster); /* cluster */
        Ebml_SerializeUnsigned(glob, Timecode, glob->cluster_timecode);

        /* Save a cue point if this is a keyframe. */
        if(isKeyFrame)
        {
            struct cue_entry *cue, *new_cue_list;

            new_cue_list = (struct cue_entry *)realloc(glob->cue_list,
                                   (glob->cues+1) * sizeof(struct cue_entry));
			OT_ASSERT(new_cue_list);
            glob->cue_list = new_cue_list;

            cue = &glob->cue_list[glob->cues];
            cue->time = glob->cluster_timecode;
            cue->loc = glob->cluster_pos;
            glob->cues++;
        }
    }

    /* Write the Simple Block */
    Ebml_WriteID(glob, SimpleBlock);

    block_length = (unsigned long)dataSize + 4;
    block_length |= 0x10000000;
    Ebml_Serialize(glob, &block_length, sizeof(block_length), 4);

    track_number = 1;
    track_number |= 0x80;
    Ebml_Write(glob, &track_number, 1);

    Ebml_Serialize(glob, &block_timecode, sizeof(block_timecode), 2);

    flags = 0;
    if(isKeyFrame)
        flags |= 0x80;
    if(isInvisible)
        flags |= 0x08;
    Ebml_Write(glob, &flags, 1);

    Ebml_Write(glob, dataPtr, (unsigned long)dataSize);
}


/*static*/ void
write_webm_file_footer(EbmlGlobal *glob, long hash)
{

    if(glob->cluster_open)
        Ebml_EndSubElement(glob, &glob->startCluster);

    {
        EbmlLoc start;
        unsigned int i;

        glob->cue_pos = (off_t)ftello(glob->stream);
        Ebml_StartSubElement(glob, &start, Cues);
        for(i=0; i<glob->cues; i++)
        {
            struct cue_entry *cue = &glob->cue_list[i];
            EbmlLoc start;

            Ebml_StartSubElement(glob, &start, CuePoint);
            {
                EbmlLoc start;

                Ebml_SerializeUnsigned(glob, CueTime, cue->time);

                Ebml_StartSubElement(glob, &start, CueTrackPositions);
                Ebml_SerializeUnsigned(glob, CueTrack, 1);
                Ebml_SerializeUnsigned64(glob, CueClusterPosition,
                                         cue->loc - glob->position_reference);
                Ebml_EndSubElement(glob, &start);
            }
            Ebml_EndSubElement(glob, &start);
        }
        Ebml_EndSubElement(glob, &start);
    }

    Ebml_EndSubElement(glob, &glob->startSegment);

    /* Patch up the seek info block */
    write_webm_seek_info(glob);

    /* Patch up the track id */
    fseeko(glob->stream, glob->track_id_pos, SEEK_SET);
    Ebml_SerializeUnsigned32(glob, TrackUID, glob->debug ? 0xDEADBEEF : hash);

    fseeko(glob->stream, 0, SEEK_END);
}