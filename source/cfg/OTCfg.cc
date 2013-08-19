/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#include "opentelepresence/cfg/OTCfg.h"
#include "opentelepresence/OTCommon.h"

#include "tsk_string.h"
#include "tsk_memory.h"
#include "tsk_debug.h"

#include <assert.h>


//
//	OTCfg
//
OTCfg::OTCfg(OTCfgType_t eType)
: m_eType(eType)
{
}

OTCfg::~OTCfg()
{
}


//
//	OTCfgSection
//
OTCfgSection::OTCfgSection(const char* pcName)
: OTCfg(OTCfgType_Section)
{
	OT_ASSERT(!tsk_strnullORempty(pcName));
	m_pName = tsk_strdup(pcName);
}

OTCfgSection::~OTCfgSection()
{
	TSK_FREE(m_pName);
}

bool OTCfgSection::addParam(OTObjectWrapper<OTCfgParam *> oParam)
{
	if(oParam)
	{
		m_oParams.push_back(oParam);
		return true;
	}
	return false;
}

bool OTCfgSection::addParam(const char* pcName, const char* pcValue)
{
	if(tsk_strnullORempty(pcName))
	{
		OT_DEBUG_ERROR("Invalid parameter");
		return false;
	}

	return addParam(new OTCfgParam(pcName, pcValue));
}


//
//	OTCfgParam
//
OTCfgParam::OTCfgParam(const char* pcName, const char* pcValue)
: OTCfg(OTCfgType_Param)
, m_pValue(NULL)
{
	OT_ASSERT(!tsk_strnullORempty(pcName));
	m_pName = tsk_strdup(pcName);
	m_pValue = tsk_strdup(pcValue);
}

OTCfgParam::~OTCfgParam()
{
	TSK_FREE(m_pName);
	TSK_FREE(m_pValue);
}
