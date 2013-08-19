//
// Copyright (C) 2013 Mamadou DIOP
// Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
// License: GPLv3
// This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
//

//
// Main ragel file for Doubango Telepresence system configuration (a.k.a 'cfg')
//
#include "opentelepresence/cfg/OTCfgParser.h"

#include "tsk_debug.h"
#include "tsk_memory.h"
#include "tsk_string.h"
#include "tsk_ragel_state.h"

#include <string.h>

#if !defined (OT_DEBUG_PARSER)
#	define OT_DEBUG_PARSER	0
#endif /* OT_DEBUG_PARSER */

%%{

	machine telepresence_machine_parser_cfg;

	action tag {
		tag_start = p;
#if OT_DEBUG_PARSER
		OT_DEBUG_INFO("tag=%s", tag_start);
#endif
	}

	action is_comment{
#if OT_DEBUG_PARSER
		TSK_PARSER_SET_STRING(temp);
		OT_DEBUG_INFO("is_comment '%s'", temp);
#endif
	}
	

	action next_not_newline{
		(p && *p != '\n' && *p != '\r')
	}

	action next_not_special{
		(p && *p != '[' && *p != '#')
	}

	action set_section_value{
		TSK_PARSER_SET_STRING(temp);
#if OT_DEBUG_PARSER
		OT_DEBUG_INFO("set_section_value '%s'", temp);
#endif
		if(fnOnNewCfg && !tsk_strnullORempty(temp))
		{
			tsk_strtrim(&temp);
			if(!tsk_strnullORempty(temp))
			{
				OTObjectWrapper<OTCfgSection*> oCfg = new OTCfgSection(temp);
				if(!(bRet = fnOnNewCfg(dynamic_cast<OTCfg*>(*oCfg), pcCallbackData)))
				{
					goto bail;
				}
			}
		}
	}

	action set_param_value{
#if OT_DEBUG_PARSER
		TSK_PARSER_SET_STRING(temp);
		OT_DEBUG_INFO("set_param_value '%s'", temp);
#endif
		if(fnOnNewCfg)
		{
			int len = (int)(p  - tag_start);
			if(len > 0)
			{
				TSK_OBJECT_SAFE_FREE(param);
				if((param = tsk_params_parse_param(tag_start, len)))
				{
					tsk_strtrim(&param->name);
					tsk_strtrim(&param->value);
					if(!tsk_strnullORempty(param->name))
					{
						OTObjectWrapper<OTCfgParam*> oCfg = new OTCfgParam(param->name, param->value);
						if(!(bRet = fnOnNewCfg(dynamic_cast<OTCfg*>(*oCfg), pcCallbackData)))
						{
							goto bail;
						}
					}
				}
			}
		}
	}

	SP = " ";
	LF = "\n";
	CR = "\r";
	CRLF = CR LF;
	HTAB = "\t";
	WSP = SP | HTAB;
	endline = (CRLF | LF | CR);

	main := |*
				(WSP* "[" (any when next_not_newline)+ >tag %set_section_value :>"]" :>endline?) >100 { };
				(WSP* "#" (any when next_not_newline)* >tag %is_comment  :>endline?) >99 { };
				(WSP* (any when next_not_special)* >tag :> "="? (any when next_not_special)* %set_param_value :>endline?) > 98 {  };
				
				any >0 { };
			*|;

}%%


bool OTCfgParser::parse(const char* pcFullFilePath, const void* pcCallbackData, OTCfgParserOnNewCfg_f fnOnNewCfg)
{
	if(tsk_strnullORempty(pcFullFilePath))
	{
		OT_DEBUG_ERROR("Invalid parameter");
		return false;
	}

	OT_DEBUG_INFO("Parsing cfg file with path = %s", pcFullFilePath);

	FILE* fd;
	char* pBuffPtr = tsk_null;
	size_t nBuffSize = 0;
	int cs = 0;
	const char *p;
	const char *pe;
	const char *eof;

	if((fd = fopen(pcFullFilePath, "r"))){
		fseek(fd, 0L, SEEK_END);
		nBuffSize = ftell(fd);
		fseek(fd, 0L, SEEK_SET);
		if(!(pBuffPtr = (char*)tsk_calloc(nBuffSize + 1, 1)))
		{
			OT_DEBUG_ERROR("Failed to allocate buffer with size = %ld", (nBuffSize + 1));
			fclose(fd);
			return false;
		}
		fread(pBuffPtr, 1, nBuffSize, fd);
		p = &pBuffPtr[0];
		pe = p + nBuffSize + 1/* hack */;
		eof = pe;
		fclose(fd);
		
		pBuffPtr[nBuffSize] = '\n'; /* hack to have perfect lines */
	}
	else
	{
		OT_DEBUG_ERROR("Failed to open cfg file with path = %s. Did you forget to run 'make samples'?", pcFullFilePath);
		return false;
	}
	
	tsk_param_t* param = tsk_null;
	char* temp = tsk_null;
	bool bRet = true;

	const char *ts = tsk_null, *te = tsk_null;
	int act = 0;
	
	const char *tag_start = tsk_null;
	
	%%write data;
	%%write init;
	%%write exec;

	if( cs < %%{ write first_final; }%% ){
		OT_DEBUG_ERROR("Failed to parse '%s' cfg:", p);
		bRet = false;
	}

	// End of File
	if(bRet && fnOnNewCfg)
	{
		if(!(bRet = fnOnNewCfg(new OTCfgEoF(), pcCallbackData)))
		{
			goto bail;
		}
	}

bail:
	TSK_OBJECT_SAFE_FREE(param);
	TSK_FREE(temp);
	TSK_FREE(pBuffPtr);

	return bRet;
}