/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_CONFIG_H
#define OPENTELEPRESENCE_CONFIG_H

#define OT_VERSION_MAJOR 2
#define OT_VERSION_MINOR 1
#define OT_VERSION_MICRO 0
#if !defined(OT_VERSION_STRING)
#	define OT_VERSION_STRING OT_STRING(OT_CAT(OT_VERSION_MAJOR, .)) OT_STRING(OT_CAT(OT_VERSION_MINOR, .)) OT_STRING(OT_VERSION_MICRO)
#endif

#if defined(WIN32)|| defined(_WIN32)
#	define OPENTELEPRESENCE_UNDER_WINDOWS	1
#elif defined(__APPLE__) // TARGET_OS_MAC
#	define OPENTELEPRESENCE_UNDER_OSX		1
#endif

#if OPENTELEPRESENCE_UNDER_WINDOWS && defined(OPENTELEPRESENCE_EXPORTS)
# 	define OPENTELEPRESENCE_API		__declspec(dllexport)
# 	define OPENTELEPRESENCE_GEXTERN __declspec(dllexport)
#elif OPENTELEPRESENCE_UNDER_WINDOWS /*&& defined(OPENTELEPRESENCE_IMPORTS)*/
# 	define OPENTELEPRESENCE_API __declspec(dllimport)
# 	define OPENTELEPRESENCE_GEXTERN __declspec(dllimport)
#else
#	define OPENTELEPRESENCE_API
#	define OPENTELEPRESENCE_GEXTERN	extern
#endif

// Disable some well-known warnings
#ifdef _MSC_VER
#	define _CRT_SECURE_NO_WARNINGS
#	define OT_INLINE	_inline
#else
#	define OT_INLINE	inline
#endif

#define OPENTELEPRESENCE_RECORD_ENABLED							false
#define OPENTELEPRESENCE_RECORD_FILE_EXT						"avi"
#define OPENTELEPRESENCE_ACCEPT_INCOMING_SIPREG					false


// Default audio values
#define OPENTELEPRESENCE_AUDIO_LOOPBACK_DEFAULT					false // must be true for testing only
#define OPENTELEPRESENCE_AUDIO_CHANNELS_DEFAULT					1
#define OPENTELEPRESENCE_AUDIO_BITS_PER_SAMPLE_DEFAULT			16
#define OPENTELEPRESENCE_AUDIO_RATE_DEFAULT						8000
#define OPENTELEPRESENCE_AUDIO_PTIME_DEFAULT					20

#define OPENTELEPRESENCE_AUDIO_MIXER_VOL						0.8f
#define OPENTELEPRESENCE_AUDIO_MIXER_DIM						OTDimension_2D

#define OPENTELEPRESENCE_AUDIO_MAX_LATENCY						200 //ms
#define OPENTELEPRESENCE_AUDIO_RTP_TIMEOUT						2500 //ms
#define OPENTELEPRESENCE_AUDIO_TIME_BEFORE_COMPUTING_AVG_VOL	5000 //ms

// Default video values
#define OPENTELEPRESENCE_VIDEO_WIDTH_DEFAULT					640 // VGA
#define OPENTELEPRESENCE_VIDEO_HEIGHT_DEFAULT					480 // VGA
#define OPENTELEPRESENCE_VIDEO_FPS_DEFAULT						15
#define OPENTELEPRESENCE_VIDEO_GOP_SIZE_DEFAULT					30 // seconds
#define OPENTELEPRESENCE_VIDEO_MOTION_RANK_DEFAULT				2 // 1(low), 2(medium) or 3(high)
#define OPENTELEPRESENCE_VIDEO_RTP_TIMEOUT						2500 //ms
#define OPENTELEPRESENCE_VIDEO_MIXER_DIM						OTDimension_2D

#define OPENTELEPRESENCE_VIDEO_PAR_SPEAKER_NUM					0
#define OPENTELEPRESENCE_VIDEO_PAR_SPEAKER_DEN					0
#define OPENTELEPRESENCE_VIDEO_PAR_LISTENER_NUM					1
#define OPENTELEPRESENCE_VIDEO_PAR_LISTENER_DEN					1

#define OPENTELEPRESENCE_PRESENTATION_SHARING_ENABLED				true
#define OPENTELEPRESENCE_PRESENTATION_SHARING_PROCESS_LOCAL_PORT	2083
#define OPENTELEPRESENCE_PRESENTATION_SHARING_BASE_FOLDER			"."
#define OPENTELEPRESENCE_PRESENTATION_SHARING_APP_PATH				"soffice"

#define OPENTELEPRESENCE_VIDEO_OVERLAY_TEXT_COPYRIGTH				"" // do not display copyright text
#define OPENTELEPRESENCE_VIDEO_OVERLAY_FONTSIZE_COPYRIGTH			12
#define OPENTELEPRESENCE_VIDEO_OVERLAY_FONTSIZE_NAME_LISTENER		12
#define OPENTELEPRESENCE_VIDEO_OVERLAY_FONTSIZE_NAME_SPEAKER		16
#define OPENTELEPRESENCE_VIDEO_OVERLAY_FONTSIZE_NAME ((OPENTELEPRESENCE_VIDEO_OVERLAY_FONTSIZE_NAME_SPEAKER + OPENTELEPRESENCE_VIDEO_OVERLAY_FONTSIZE_NAME_LISTENER) >> 1)
#define OPENTELEPRESENCE_VIDEO_OVERLAY_DISPLAY_SPEAKER_NAME			true
#define OPENTELEPRESENCE_VIDEO_OVERLAY_DISPLAY_SPEAKER_JOBTITLE		true
#define OPENTELEPRESENCE_VIDEO_OVERLAY_PATH_WATERMARK				"" // do not dispaly watermark image
#define OPENTELEPRESENCE_VIDEO_OVERLAY_PATH_FONTFOLDER				"./fonts/truetype/freefont" // base folder
#define OPENTELEPRESENCE_VIDEO_OVERLAY_FILENAME_FONT_COPYRIGHT		"FreeSerif.ttf"
#define OPENTELEPRESENCE_VIDEO_OVERLAY_FILENAME_FONT_SPEAKER_NAME	"FreeSerif.ttf"
#define OPENTELEPRESENCE_VIDEO_OVERLAY_FILENAME_FONT_SPEAKER		"FreeMonoBold.ttf"
#define OPENTELEPRESENCE_VIDEO_OVERLAY_FILENAME_FONT_JOBTITLE		"FreeMonoBold.ttf"


#define OPENTELEPRESENCE_INVALID_ID	0

#if OPENTELEPRESENCE_UNDER_WINDOWS
#	define _WINSOCKAPI_
#	include <windows.h>
#elif OPENTELEPRESENCE_UNDER_LINUX
#elif OPENTELEPRESENCE_UNDER_MACOS
#endif

#include <stdint.h>

#if !defined(INT64_C) /* should be in <stdint> */
#	define INT64_C(c) (c ## LL)
#endif
#if !defined(UINT64_C) /* should be in <stdint> */
#	define UINT64_C(c) (c ## ULL)
#endif

#ifdef __GNUC__
#	define ot_atomic_inc(_ptr_) __sync_fetch_and_add((_ptr_), 1)
#	define ot_atomic_dec(_ptr_) __sync_fetch_and_sub((_ptr_), 1)
#elif defined (_MSC_VER)
#	define ot_atomic_inc(_ptr_) InterlockedIncrement((_ptr_))
#	define ot_atomic_dec(_ptr_) InterlockedDecrement((_ptr_))
#else
#	define ot_atomic_inc(_ptr_) ++(*(_ptr_))
#	define ot_atomic_dec(_ptr_) --(*(_ptr_))
#endif

#if _MSC_VER >= 1400
#	pragma warning( disable : 4290 4800 4251 )
#endif


#define OT_DEBUG_INFO(FMT, ...) TSK_DEBUG_INFO("[TELEPRESENCE] " FMT, ##__VA_ARGS__)
#define OT_DEBUG_WARN(FMT, ...) TSK_DEBUG_WARN("[TELEPRESENCE] " FMT, ##__VA_ARGS__)
#define OT_DEBUG_ERROR(FMT, ...) TSK_DEBUG_ERROR("[TELEPRESENCE] " FMT, ##__VA_ARGS__)
#define OT_DEBUG_FATAL(FMT, ...) TSK_DEBUG_FATAL("[TELEPRESENCE] " FMT, ##__VA_ARGS__)

#define OT_DEBUG_INFO_EX(MODULE, FMT, ...) OT_DEBUG_INFO("[" MODULE "] " FMT, ##__VA_ARGS__)
#define OT_DEBUG_WARN_EX(MODULE, FMT, ...) OT_DEBUG_WARN("[" MODULE "] " FMT, ##__VA_ARGS__)
#define OT_DEBUG_ERROR_EX(MODULE, FMT, ...) OT_DEBUG_ERROR("[" MODULE "] " FMT, ##__VA_ARGS__)
#define OT_DEBUG_FATAL_EX(MODULE, FMT, ...) OT_DEBUG_FATAL("[" MODULE "] " FMT, ##__VA_ARGS__)

#include <stdlib.h>

#if HAVE_CONFIG_H
	#include <config.h>
#endif

#endif /* OPENTELEPRESENCE_CONFIG_H */

