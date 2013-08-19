/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/

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


/* #line 110 "cfg.rl" */



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
	
	
/* #line 80 "../source/cfg/OTCfgParser.cc" */
static const char _telepresence_machine_parser_cfg_actions[] = {
	0, 1, 0, 1, 1, 1, 2, 1, 
	3, 1, 4, 1, 5, 1, 6, 1, 
	7, 1, 8, 1, 10, 1, 11, 1, 
	12, 2, 0, 1, 2, 0, 3, 2, 
	1, 7, 2, 1, 11, 2, 3, 8, 
	2, 3, 12, 3, 0, 1, 7, 3, 
	0, 1, 11, 3, 0, 3, 8, 3, 
	0, 3, 9
};

static const char _telepresence_machine_parser_cfg_cond_offsets[] = {
	0, 0, 0, 1, 2, 3, 3, 3, 
	3, 4, 4, 5
};

static const char _telepresence_machine_parser_cfg_cond_lengths[] = {
	0, 0, 1, 1, 1, 0, 0, 0, 
	1, 0, 1, 1
};

static const short _telepresence_machine_parser_cfg_cond_keys[] = {
	-128, 127, -128, 127, -128, 127, -128, 127, 
	-128, 127, -128, 127, 0
};

static const char _telepresence_machine_parser_cfg_cond_spaces[] = {
	0, 0, 1, 0, 0, 1, 0
};

static const char _telepresence_machine_parser_cfg_key_offsets[] = {
	0, 0, 3, 5, 9, 28, 30, 31, 
	32, 38, 39, 45
};

static const short _telepresence_machine_parser_cfg_trans_keys[] = {
	9, 32, 91, 384, 639, 349, 605, 384, 
	639, 777, 778, 781, 800, 803, 829, 859, 
	1033, 1034, 1037, 1056, 1059, 1115, -128, 127, 
	640, 895, 896, 1151, 10, 13, 10, 10, 
	266, 269, 522, 525, 384, 639, 10, 266, 
	269, 522, 525, 384, 639, 778, 781, 1034, 
	1037, 896, 1151, 0
};

static const char _telepresence_machine_parser_cfg_single_lengths[] = {
	0, 3, 0, 2, 13, 2, 1, 1, 
	4, 1, 4, 4
};

static const char _telepresence_machine_parser_cfg_range_lengths[] = {
	0, 0, 1, 1, 3, 0, 0, 0, 
	1, 0, 1, 1
};

static const char _telepresence_machine_parser_cfg_index_offsets[] = {
	0, 0, 4, 6, 10, 27, 30, 32, 
	34, 40, 42, 48
};

static const char _telepresence_machine_parser_cfg_trans_targs[] = {
	1, 1, 2, 0, 3, 0, 5, 5, 
	3, 0, 1, 4, 7, 1, 8, 11, 
	2, 1, 4, 7, 1, 8, 2, 0, 
	4, 11, 0, 4, 6, 4, 4, 4, 
	4, 4, 4, 9, 4, 9, 10, 4, 
	4, 4, 4, 9, 4, 9, 10, 4, 
	4, 7, 4, 7, 11, 4, 4, 4, 
	4, 4, 4, 4, 4, 0
};

static const char _telepresence_machine_parser_cfg_trans_actions[] = {
	0, 0, 0, 0, 1, 0, 5, 5, 
	0, 0, 0, 51, 28, 0, 28, 1, 
	28, 0, 51, 28, 0, 0, 0, 0, 
	55, 1, 28, 13, 0, 19, 13, 19, 
	17, 23, 43, 25, 43, 25, 1, 47, 
	15, 21, 31, 3, 31, 3, 0, 34, 
	37, 7, 37, 7, 0, 40, 19, 19, 
	23, 47, 21, 34, 40, 0
};

static const char _telepresence_machine_parser_cfg_to_state_actions[] = {
	0, 0, 0, 0, 9, 0, 0, 0, 
	0, 0, 0, 0
};

static const char _telepresence_machine_parser_cfg_from_state_actions[] = {
	0, 0, 0, 0, 11, 0, 0, 0, 
	0, 0, 0, 0
};

static const char _telepresence_machine_parser_cfg_eof_actions[] = {
	0, 0, 0, 0, 28, 0, 0, 0, 
	0, 0, 0, 0
};

static const char _telepresence_machine_parser_cfg_eof_trans[] = {
	0, 0, 0, 0, 0, 56, 56, 57, 
	58, 59, 60, 61
};

static const int telepresence_machine_parser_cfg_start = 4;
static const int telepresence_machine_parser_cfg_first_final = 4;
static const int telepresence_machine_parser_cfg_error = 0;

static const int telepresence_machine_parser_cfg_en_main = 4;


/* #line 165 "cfg.rl" */
	
/* #line 192 "../source/cfg/OTCfgParser.cc" */
	{
	cs = telepresence_machine_parser_cfg_start;
	ts = 0;
	te = 0;
	act = 0;
	}

/* #line 166 "cfg.rl" */
	
/* #line 202 "../source/cfg/OTCfgParser.cc" */
	{
	int _klen;
	unsigned int _trans;
	short _widec;
	const char *_acts;
	unsigned int _nacts;
	const short *_keys;

	if ( p == pe )
		goto _test_eof;
	if ( cs == 0 )
		goto _out;
_resume:
	_acts = _telepresence_machine_parser_cfg_actions + _telepresence_machine_parser_cfg_from_state_actions[cs];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 ) {
		switch ( *_acts++ ) {
	case 5:
/* #line 1 "cfg.rl" */
	{ts = p;}
	break;
/* #line 224 "../source/cfg/OTCfgParser.cc" */
		}
	}

	_widec = (*p);
	_klen = _telepresence_machine_parser_cfg_cond_lengths[cs];
	_keys = _telepresence_machine_parser_cfg_cond_keys + (_telepresence_machine_parser_cfg_cond_offsets[cs]*2);
	if ( _klen > 0 ) {
		const short *_lower = _keys;
		const short *_mid;
		const short *_upper = _keys + (_klen<<1) - 2;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + (((_upper-_lower) >> 1) & ~1);
			if ( _widec < _mid[0] )
				_upper = _mid - 2;
			else if ( _widec > _mid[1] )
				_lower = _mid + 2;
			else {
				switch ( _telepresence_machine_parser_cfg_cond_spaces[_telepresence_machine_parser_cfg_cond_offsets[cs] + ((_mid - _keys)>>1)] ) {
	case 0: {
		_widec = (short)(128 + ((*p) - -128));
		if ( 
/* #line 39 "cfg.rl" */

		(p && *p != '\n' && *p != '\r')
	 ) _widec += 256;
		break;
	}
	case 1: {
		_widec = (short)(640 + ((*p) - -128));
		if ( 
/* #line 43 "cfg.rl" */

		(p && *p != '[' && *p != '#')
	 ) _widec += 256;
		break;
	}
				}
				break;
			}
		}
	}

	_keys = _telepresence_machine_parser_cfg_trans_keys + _telepresence_machine_parser_cfg_key_offsets[cs];
	_trans = _telepresence_machine_parser_cfg_index_offsets[cs];

	_klen = _telepresence_machine_parser_cfg_single_lengths[cs];
	if ( _klen > 0 ) {
		const short *_lower = _keys;
		const short *_mid;
		const short *_upper = _keys + _klen - 1;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + ((_upper-_lower) >> 1);
			if ( _widec < *_mid )
				_upper = _mid - 1;
			else if ( _widec > *_mid )
				_lower = _mid + 1;
			else {
				_trans += (_mid - _keys);
				goto _match;
			}
		}
		_keys += _klen;
		_trans += _klen;
	}

	_klen = _telepresence_machine_parser_cfg_range_lengths[cs];
	if ( _klen > 0 ) {
		const short *_lower = _keys;
		const short *_mid;
		const short *_upper = _keys + (_klen<<1) - 2;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + (((_upper-_lower) >> 1) & ~1);
			if ( _widec < _mid[0] )
				_upper = _mid - 2;
			else if ( _widec > _mid[1] )
				_lower = _mid + 2;
			else {
				_trans += ((_mid - _keys)>>1);
				goto _match;
			}
		}
		_trans += _klen;
	}

_match:
_eof_trans:
	cs = _telepresence_machine_parser_cfg_trans_targs[_trans];

	if ( _telepresence_machine_parser_cfg_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _telepresence_machine_parser_cfg_actions + _telepresence_machine_parser_cfg_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
/* #line 24 "cfg.rl" */
	{
		tag_start = p;
#if OT_DEBUG_PARSER
		OT_DEBUG_INFO("tag=%s", tag_start);
#endif
	}
	break;
	case 1:
/* #line 31 "cfg.rl" */
	{
#if OT_DEBUG_PARSER
		TSK_PARSER_SET_STRING(temp);
		OT_DEBUG_INFO("is_comment '%s'", temp);
#endif
	}
	break;
	case 2:
/* #line 47 "cfg.rl" */
	{
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
	break;
	case 3:
/* #line 66 "cfg.rl" */
	{
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
	break;
	case 6:
/* #line 103 "cfg.rl" */
	{te = p+1;{ }}
	break;
	case 7:
/* #line 104 "cfg.rl" */
	{te = p+1;{ }}
	break;
	case 8:
/* #line 105 "cfg.rl" */
	{te = p+1;{  }}
	break;
	case 9:
/* #line 107 "cfg.rl" */
	{te = p+1;{ }}
	break;
	case 10:
/* #line 103 "cfg.rl" */
	{te = p;p--;{ }}
	break;
	case 11:
/* #line 104 "cfg.rl" */
	{te = p;p--;{ }}
	break;
	case 12:
/* #line 105 "cfg.rl" */
	{te = p;p--;{  }}
	break;
/* #line 428 "../source/cfg/OTCfgParser.cc" */
		}
	}

_again:
	_acts = _telepresence_machine_parser_cfg_actions + _telepresence_machine_parser_cfg_to_state_actions[cs];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 ) {
		switch ( *_acts++ ) {
	case 4:
/* #line 1 "cfg.rl" */
	{ts = 0;}
	break;
/* #line 441 "../source/cfg/OTCfgParser.cc" */
		}
	}

	if ( cs == 0 )
		goto _out;
	if ( ++p != pe )
		goto _resume;
	_test_eof: {}
	if ( p == eof )
	{
	if ( _telepresence_machine_parser_cfg_eof_trans[cs] > 0 ) {
		_trans = _telepresence_machine_parser_cfg_eof_trans[cs] - 1;
		goto _eof_trans;
	}
	const char *__acts = _telepresence_machine_parser_cfg_actions + _telepresence_machine_parser_cfg_eof_actions[cs];
	unsigned int __nacts = (unsigned int) *__acts++;
	while ( __nacts-- > 0 ) {
		switch ( *__acts++ ) {
	case 0:
/* #line 24 "cfg.rl" */
	{
		tag_start = p;
#if OT_DEBUG_PARSER
		OT_DEBUG_INFO("tag=%s", tag_start);
#endif
	}
	break;
	case 3:
/* #line 66 "cfg.rl" */
	{
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
	break;
/* #line 499 "../source/cfg/OTCfgParser.cc" */
		}
	}
	}

	_out: {}
	}

/* #line 167 "cfg.rl" */

	if( cs < 
/* #line 510 "../source/cfg/OTCfgParser.cc" */
4
/* #line 168 "cfg.rl" */
 ){
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