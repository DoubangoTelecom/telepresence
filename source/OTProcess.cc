/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#include "opentelepresence/OTProcess.h"

#if OPENTELEPRESENCE_UNDER_WINDOWS
#	include <Windows.h>
#endif /* OPENTELEPRESENCE_UNDER_WINDOWS */
#if HAVE_SPAWN_H 
#	include <spawn.h>
#endif /* HAVE_SPAWN_H */
#if HAVE_SYS_WAIT_H
#	include <sys/wait.h>
#endif /* HAVE_SYS_WAIT_H */
#if HAVE_UNISTD_H
#	include <unistd.h>
#endif /* HAVE_UNISTD_H */
#if HAVE_ERRNO_H
#	include <errno.h>
#endif /* HAVE_ERRNO_H */
#if HAVE_POPEN && HAVE_PCLOSE
#	include <stdio.h> /* popen(), pclose(), ... */
#	include "tsk_thread.h"
#endif

//
//	OTProcess
//

OTProcess::OTProcess(std::string strProgram, std::string strArgs)
{

}

OTProcess::~OTProcess()
{

}

//
//	OTProcessLinux
//
#if HAVE_POSIX_SPAWN || (HAVE_EXECL && (HAVE_FORK || HAVE_VFORK))
class OTProcessLinux : public OTProcess
{
public:
	OTProcessLinux(std::string strProgram, std::string strArgs)
		: OTProcess(strProgram, strArgs)
		, m_bStarted(false)
		, m_bValid(false)
		, m_pid(0)
		, m_strProgram(strProgram)
		, m_strArgs(strArgs)
	{
		if((m_bValid = !strProgram.empty())){ }
	}

	virtual ~OTProcessLinux()
	{
		stop();
	}

	virtual OT_INLINE const char* getObjectId() 
	{ 
		return "OTProcessLinux"; 
	}

	virtual bool isValid()
	{
		return m_bValid;
	}

	virtual bool start()
	{
		if(m_bStarted)
		{
			return true;
		}
		pid_t pid;
#if HAVE_POSIX_SPAWN
		OT_DEBUG_INFO("posix_spawn(%s, %s).",m_strProgram.c_str(), m_strArgs.c_str());
		posix_spawnattr_t	attr;
		if(posix_spawnattr_init(&attr))
		{
			OT_DEBUG_ERROR("posix_spawnattr_init() with error code = %d", errno);
			return false;
		}
		if(posix_spawnattr_setflags(&attr, POSIX_SPAWN_USEVFORK))
		{
			OT_DEBUG_ERROR("posix_spawnattr_setflags(POSIX_SPAWN_USEVFORK) with error code = %d", errno);
			return false;
		}
		const char* command[3] =
		{
			m_strProgram.c_str(),
			m_strArgs.c_str(),
			(char*)0
		};
		if (posix_spawn(&pid, command[0], 0, &attr, (char* const*)command, 0))
		{
			OT_DEBUG_ERROR("posix_spawn(%s, %s) with error code = %d",m_strProgram.c_str(), m_strArgs.c_str(), errno);
			abort();
		}
#elif HAVE_VFORK
		OT_DEBUG_INFO("vfork(%s, %s).",m_strProgram.c_str(), m_strArgs.c_str());
		pid = vfork();
#else
		OT_DEBUG_INFO("fork(%s, %s).",m_strProgram.c_str(), m_strArgs.c_str());
		pid = fork();
#endif

		switch(pid)
		{
		case -1:
			{
				OT_DEBUG_ERROR("Forking failed with error code = %d", errno);
				abort();
			}
		case 0:
			{
				/* This is the child process */
				if(setpgid(0/*current*/, getppid()) != 0)
				{
					OT_DEBUG_ERROR("setpgid() failed. Error code = %d", errno);
					// not fatal error
				}
#if HAVE_POSIX_SPAWN
				break;
#else /* fork() and vfork require calling execl() */
				execl(m_strProgram.c_str(), m_strProgram.c_str(), m_strArgs.c_str(), (char *) 0); // exit only if fail
				OT_DEBUG_ERROR("execl(%s,%s) failed. Error code = %d", m_strProgram.c_str(), m_strArgs.c_str(), errno);
				return false;
#endif
			}
		default:
			{
				/* This is the parent process */
				m_pid = pid;
#if HAVE_SETPGID
				if(m_pid != 0 && setpgid(m_pid/*child*/, getpid()) != 0) // group to be sure child will exit if parent crash
				{
					OT_DEBUG_ERROR("setpgid() failed. Error code = %d", errno);
					// not fatal error
				}
#endif /* HAVE_SETPGID */
#if HAVE_WAITPID && 0
				if(m_pid == 0)
				{
					int status;
					OT_DEBUG_INFO("waitpid(%s,%s)", m_strProgram.c_str(), m_strArgs.c_str());
					if(waitpid(m_pid, &status, 0) != -1)
					{
						OT_DEBUG_INFO("Newly created Child exited with code = %d\n", status);
					}
					else
					{
						OT_DEBUG_ERROR("waitpid(%i, %s,%s) failed. Error code = %d", m_pid, m_strProgram.c_str(), m_strArgs.c_str(), errno);
						return false;
					}
				}
#endif /* HAVE_WAITPID */

				break;
			}
		}//switch

		m_bStarted = true;
		return true;
	}

	virtual bool isStarted()
	{
		return m_bStarted;
	}

	virtual bool stop()
	{
		bool bRet = true;
		if(m_pid == 0)
		{
			// this is child process 
			OT_DEBUG_INFO("exit(0) child process");
			_exit(0);
		}
		else if(m_bStarted)
		{
			OT_DEBUG_INFO("kill(%d) child process", m_pid);
			kill(m_pid, SIGTERM);
			m_pid = 0;
		}
		m_bStarted = false;
		return bRet;
	}

private:
	bool m_bValid;
	bool m_bStarted;
	pid_t m_pid;
	std::string m_strProgram, m_strArgs;
};
#endif /* HAVE_POSIX_SPAWN || (HAVE_EXECL && (HAVE_FORK || HAVE_VFORK))*/


//
//	OTProcessPopen
//
#if HAVE_POPEN && HAVE_PCLOSE
class OTProcessPopen : public OTProcess
{
public:
	OTProcessPopen(std::string strProgram, std::string strArgs)
		: OTProcess(strProgram, strArgs)
		, m_bStarted(false)
		, m_bValid(false)
		, m_pFile(NULL)
	{
		if((m_bValid = !strProgram.empty()))
		{
			m_strCommand = strProgram;
			if(!strArgs.empty())
			{
				m_strCommand += std::string(" ") + strArgs;
			}
		}
	}

	virtual ~OTProcessPopen()
	{
		stop();
	}

	virtual OT_INLINE const char* getObjectId() 
	{ 
		return "OTProcessPopen"; 
	}

	virtual bool isValid()
	{
		return m_bValid;
	}

	virtual bool start()
	{
		if(m_bStarted)
		{
			return true;
		}

		OT_DEBUG_INFO("popen(%s)", m_strCommand.c_str());
		m_pFile = popen(m_strCommand.c_str(), "r");
		if(!m_pFile)
		{
			OT_DEBUG_ERROR("Failed to popen(%s) with error code = %d", m_strCommand.c_str(), errno);
			return false;
		}	

		m_bStarted = true;
		return true;
	}

	virtual bool isStarted()
	{
		return m_bStarted;
	}

	virtual bool stop()
	{
		bool bRet = true;
		if(m_pFile)
		{
			pclose(m_pFile);
			m_pFile = NULL;
		}
		
		m_bStarted = false;
		return bRet;
	}

private:
	bool m_bValid;
	bool m_bStarted;
	FILE* m_pFile;
	std::string m_strCommand;
};
#endif /* HAVE_POPEN && HAVE_PCLOSE */


//
//	OTProcessWin32
//
#if OPENTELEPRESENCE_UNDER_WINDOWS
class OTProcessWin32 : public OTProcess
{
public:
	OTProcessWin32(std::string strProgram, std::string strArgs)
		: OTProcess(strProgram, strArgs)
		, m_bStarted(false)
		, m_bValid(false)
		, m_hProcess(NULL)
	{
		if((m_bValid = !strProgram.empty()))
		{
			m_strCommand = strProgram;
			if(!strArgs.empty())
			{
				m_strCommand += std::string(" ") + strArgs;
			}
		}
	}

	virtual ~OTProcessWin32()
	{
		stop();
	}

	virtual OT_INLINE const char* getObjectId() 
	{ 
		return "OTProcessWin32"; 
	}

	virtual bool isValid()
	{
		return m_bValid;
	}

	virtual bool start()
	{
		if(m_bStarted)
		{
			return true;
		}

		if(!g_hJob)
		{
			g_hJob = CreateJobObject( NULL, NULL);
			if(!g_hJob)
			{
				OT_DEBUG_ERROR("Failed to create job. Error code = %08x", GetLastError());
				return false;
			}
			JOBOBJECT_EXTENDED_LIMIT_INFORMATION ji = {0};
			ji.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE; // close all processes when the main App crash
			if(!SetInformationJobObject(g_hJob, JobObjectExtendedLimitInformation, &ji, sizeof(ji)))
			{
				OT_DEBUG_WARN("SetInformationJobObject() failed with error code = %08x", GetLastError());
				// Not fatal error
			}
		}

		STARTUPINFOA si = {0};
		PROCESS_INFORMATION pi = {0};
		char cmdLine[1200] = { 0 };
		memcpy(cmdLine, m_strCommand.c_str(), TSK_MIN(sizeof(cmdLine), m_strCommand.length()));
		if(!CreateProcessA(NULL, cmdLine, NULL, NULL, NULL, NULL, NULL, NULL, &si, &pi))
		{
			OT_DEBUG_ERROR("Failed to create process with command = %s. Error code = %08x", cmdLine, GetLastError());
			return false;
		}

		if(!AssignProcessToJobObject(g_hJob, pi.hProcess))
		{
			OT_DEBUG_WARN("AssignProcessToJobObject() failed with error code = %08x", GetLastError());
			// Not fatal error
		}

		m_hProcess = pi.hProcess;
		m_bStarted = true;
		return true;
	}

	virtual bool isStarted()
	{
		return m_bStarted;
	}

	virtual bool stop()
	{
		bool bRet = true;
		if(m_hProcess)
		{
			bRet = (TerminateProcess(m_hProcess, 0) == TRUE);
			m_hProcess = NULL;
		}
		m_bStarted = false;
		return bRet;
	}

private:
	bool m_bValid;
	bool m_bStarted;
	std::string m_strCommand;
	HANDLE m_hProcess;
	static HANDLE g_hJob;
};
HANDLE OTProcessWin32::g_hJob = NULL;
#endif /* OPENTELEPRESENCE_UNDER_WINDOWS */


OTObjectWrapper<OTProcess*> OTProcess::New(std::string strProgram, std::string strArgs)
{
	OTObjectWrapper<OTProcess*> oProcess;
#if OPENTELEPRESENCE_UNDER_WINDOWS
	oProcess = new OTProcessWin32(strProgram, strArgs);
#elif HAVE_POPEN && HAVE_PCLOSE /* FIXME: popen() is used instead of vfork() or posix_spawn() because soffice fails to bind to the port in the arguments. Looks like arguments are ignored.  */
	oProcess = new OTProcessPopen(strProgram, strArgs);
#elif HAVE_POSIX_SPAWN || (HAVE_EXECL && (HAVE_FORK || HAVE_VFORK))
	oProcess = new OTProcessLinux(strProgram, strArgs);
#endif

	if(oProcess && !oProcess->isValid())
	{
		oProcess = NULL;
	}
	return oProcess;
}