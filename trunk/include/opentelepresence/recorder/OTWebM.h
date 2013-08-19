/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_RECODER_WEBM_H
#define OPENTELEPRESENCE_RECODER_WEBM_H

#include "OpenTelepresenceConfig.h"

/* Stereo 3D packed frame format */
typedef enum stereo_format
{
    STEREO_FORMAT_MONO       = 0,
    STEREO_FORMAT_LEFT_RIGHT = 1,
    STEREO_FORMAT_BOTTOM_TOP = 2,
    STEREO_FORMAT_TOP_BOTTOM = 3,
    STEREO_FORMAT_RIGHT_LEFT = 11
} stereo_format_t;

typedef struct EbmlRational
{
    int num; /**< fraction numerator */
    int den; /**< fraction denominator */
}
EbmlRational;

#endif /* OPENTELEPRESENCE_RECODER_H */
