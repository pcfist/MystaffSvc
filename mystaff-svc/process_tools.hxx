/**
 * @file process_tools.hxx 
 * Helper functions for process manipulation.
 * @author pcfist	@date 2015.12.10
 */
#pragma once

#include <windows.h>
#include <psapi.h>

#include <scope_guard.hxx>
#include <QString>


// Process ID value type.
typedef DWORD	pid_t;
// Session ID value type.
typedef DWORD	sid_t;


/**
 * Creates child process with specified command line.
 * @param[in]	executablePath	- Path to executable to be launched.
 * @param[in]	cmdLine	- Command line string.
 * @param[in]	invisible	- If true, child process won't have a window.
 * @return	[HANDLE]	- Handle to newly created process or 0 in failure.
 */
inline
HANDLE createProcess(const wchar_t* executablePath, wchar_t* cmdLine, bool invisible = false)
{
	STARTUPINFO si = { sizeof si };
	si.dwFlags = STARTF_USESTDHANDLES;
	PROCESS_INFORMATION pi = {};
	
	DWORD flags = 0;
	if (invisible)
		flags |= CREATE_NO_WINDOW;
	
	if (::CreateProcess(executablePath, cmdLine, nullptr, nullptr, false, flags, nullptr, nullptr, &si, &pi))
	{
		::CloseHandle(pi.hThread);
		return pi.hProcess;
	}

	return 0;
}


/**
 * Returns full process image path.
 * @param[in]	hProcess	- Handle to process.
 * @return	[QString]	- Process image path.
 */
inline
QString getProcessImagePath(HANDLE hProcess)
{
	std::wstring path;
	DWORD length = MAX_PATH;
	path.resize(length);

	::QueryFullProcessImageName(hProcess, 0, const_cast<wchar_t*>(path.c_str()), &length);

	if (length > path.length())
	{
		path.resize(length);
		::QueryFullProcessImageName(hProcess, 0, const_cast<wchar_t*>(path.c_str()), &length);
	}

	if (length < path.length())
		path.resize(length);

	return QString::fromStdWString(path);
}

