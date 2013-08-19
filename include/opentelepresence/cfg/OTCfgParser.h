/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_CFG_PARSER_H
#define OPENTELEPRESENCE_CFG_PARSER_H

#include "OpenTelepresenceConfig.h"
#include "OTCfg.h"

typedef bool (*OTCfgParserOnNewCfg_f)(OTObjectWrapper<OTCfg*> oCfg, const void* pcCallbackData);

class OTCfgParser
{
public:
	static bool parse(const char* pcFullFilePath, const void* pcCallbackData, OTCfgParserOnNewCfg_f fnOnNewCfg);
};

#endif /* OPENTELEPRESENCE_CFG_PARSER_H */
