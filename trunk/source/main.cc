/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#include <OpenTelepresenceAPI.h>

#include "tsk_string.h" /*tsk_str*()*/

#include <assert.h>

#define ot_list_count(list)					tsk_list_count((list), tsk_null, tsk_null)
#define ot_str_is(str, val)					tsk_striequals((const char*)(str), val)
#define ot_str_is_star(str)					ot_str_is((str), "*")
#define ot_str_is_yes(str)					ot_str_is((str), "yes")
#define ot_str_is_null_or_empty(str)		tsk_strnullORempty((str))
#define ot_strn_update(ppstr, newstr, n)	if(ppstr){ if(*ppstr) free(*ppstr); *ppstr = tsk_strndup(newstr, n); }

static char* sConfigFilePath = NULL;
static const char* kConfigFilePath = "./telepresence.cfg";

static int printUsage()
{
	fprintf(stdout, 
		"Usage: telepresence [OPTION]\n\n"
		"--config=PATH     override the default path to the telepresence.cfg file\n"
		"--help            display this help and exit\n"
		"--version         output version information and exit\n"
		);

	return 0;
}

static int parseArgument(const char* arg, const char** name, tsk_size_t* name_size, const char** value, tsk_size_t* value_size)
{
	int32_t index, arg_size;
	if(tsk_strnullORempty(arg) || !name || !name_size || !value || !value_size)
	{
		OT_DEBUG_ERROR("Invalid parameter");
		return -1;
	}
	*name = *value = tsk_null;
	*name_size = *value_size = 0;
	arg_size = tsk_strlen(arg);

	*name = arg;
	index = tsk_strindexOf(arg, arg_size, "=");
	if(index <= 0)
	{
		*name_size = arg_size;
		return 0;
	}
	*name_size = index;
	*value = &arg[index + 1];
	*value_size = (arg_size - index - 1);
	return 0;
}

static int parseArguments(int argc, char** argv)
{
	int i, ret;
	const char *name, *value;
	tsk_size_t name_size, value_size;

	if(argc <= 1 || !argv)
	{
		return 0;
	}

	for(i = 1; i < argc; ++i)
	{
		if((ret = parseArgument(argv[i], &name, &name_size, &value, &value_size)))
		{
			printUsage();
			return ret;
		}
		if(tsk_strniequals("--config", name, name_size))
		{
			if(!value || !value_size)
			{
				fprintf(stderr, "--config requires valid PATH\n");
				printUsage();
				exit(-1);
			}
			ot_strn_update(&sConfigFilePath, value, value_size);
		}
		else if(tsk_strniequals("--help", name, name_size))
		{
			printUsage();
			exit(-1);
		}
		else if(tsk_strniequals("--version", name, name_size))
		{
			fprintf(stdout, "%d.%d.%d\n", OT_VERSION_MAJOR, OT_VERSION_MINOR, OT_VERSION_MICRO);
			exit(-1);
		}
		else
		{
			fprintf(stderr, "'%.*s' not valid as command argument\n", name_size, name);
			exit(-1);
		}
	}
	return 0;
}

int main (int argc, char** argv) 
{
	bool bRet;
	int iRet, i = 0;
	char quit[4];

	printf("*******************************************************************\n"
		"Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>\n"
		"PRODUCT: telepresence - the open source TelePresence System\n"
		"HOME PAGE: http://conf-call.org\n"
		"CODE SOURCE: https://code.google.com/p/telepresence/\n"
		"LICENCE: GPLv3 or commercial(contact us)\n"
		"VERSION: %s\n"
		"'quit' to quit the application.\n"
		"*******************************************************************\n\n"
		, OT_VERSION_STRING);

	// parse command arguments
	if((iRet = parseArguments(argc, argv)) != 0)
	{
		return iRet;
	}
	// create engine
	OTObjectWrapper<OTEngine*> oEngine = OTEngine::New();
	// set a configuration file
	if(!(bRet = oEngine->setConfFile(ot_str_is_null_or_empty(sConfigFilePath) ? kConfigFilePath : sConfigFilePath)))
	{
		return iRet;
	}
	if(!(bRet = oEngine->start()))
	{
		exit (-1);
	}
	
	while(true)
	{
		if((quit[i & 3] = getchar()) == 't')
		{
			if(quit[(i + 1) & 3] == 'q' && quit[(i + 2) & 3] == 'u' && quit[(i + 3) & 3] == 'i')
			{
				break;
			}
		}
		// https://code.google.com/p/webrtc2sip/issues/detail?id=96
		tsk_thread_sleep(1);
		++i;
	}

	oEngine->stop();

	return 0;
}