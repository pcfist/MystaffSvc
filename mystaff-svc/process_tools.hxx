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


__declspec(selectany)
HANDLE _thisProcess = ::GetCurrentProcess();

__declspec(selectany)
HMODULE _thisModule = ::GetModuleHandle(nullptr);

inline
HANDLE currentProcess() { return _thisProcess; }

inline
HANDLE currentModule() { return _thisModule; }


/**
 * Returns handle to specified module in context of remote process.
 * @param[in]	hProcess	- Remote process handle.
 * @param[in]	dllPath	- Path to target module.
 * @return	[HMODULE]	- Handle to target module in context of remote process.
 */
inline
HMODULE getRemoteDllModuleHandle(HANDLE hProcess, const wchar_t* dllPath)
{
	DWORD bytesNeeded = 0;

#ifdef _WIN64
	const DWORD flags = LIST_MODULES_ALL;
#else
	const DWORD flags = LIST_MODULES_DEFAULT;
#endif // _WIN64

	// Get module list size.
	EnumProcessModulesEx(hProcess, nullptr, 0, &bytesNeeded, flags);

	int moduleCount = bytesNeeded / sizeof(HMODULE);
	HMODULE* moduleArray = new HMODULE[moduleCount];
	auto memGuard = make_scope_guard([moduleArray]{ delete[] moduleArray; });

	// Get module list & find target module.
	if (EnumProcessModulesEx(hProcess, moduleArray, bytesNeeded, &bytesNeeded, LIST_MODULES_ALL))
	{
		for (int i = 0; i < moduleCount; ++i)
		{
			wchar_t strBuffer[MAX_PATH];
			GetModuleFileNameExW(hProcess, moduleArray[i], strBuffer, ARRAYSIZE(strBuffer));
			if (wcsstr(strBuffer, dllPath))
			{
				MODULEINFO mi;
				::GetModuleInformation(hProcess, moduleArray[i], &mi, sizeof mi);
				return (HMODULE)mi.lpBaseOfDll;
			}
		}
	}

	return 0;
}

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
 * Returns full path name to specified file.
 * @param[in]	fileName	- File name.
 * @return	[std::wstring]	- Full path to the file.
 */
inline
std::wstring getFullPathName(const wchar_t* fileName)
{
	std::wstring fullPath;
	wchar_t* name;
	DWORD pathLen = ::GetFullPathName(fileName, (DWORD)fullPath.length(), const_cast<wchar_t*>(fullPath.c_str()), &name);
	if (pathLen > 0)
	{
		fullPath.resize(pathLen);
		::GetFullPathName(fileName, (DWORD)fullPath.length(), const_cast<wchar_t*>(fullPath.c_str()), &name);
	}

	return fullPath;
}

/**
 * Returns short path name to specified file.
 * @param[in]	fileName	- File name.
 * @return	[std::wstring]	- Short path to the file.
 */
inline
std::wstring getShortPathName(const wchar_t* fileName)
{
	std::wstring shortPath;
	DWORD pathLen = ::GetShortPathName(fileName, const_cast<wchar_t*>(shortPath.c_str()), 0);
	if (pathLen > 0)
	{
		shortPath.resize(pathLen);
		::GetShortPathName(fileName, const_cast<wchar_t*>(shortPath.c_str()), pathLen);
		if (shortPath[pathLen - 1] == L'\0')
			shortPath.pop_back();
	}
	else
	{
		shortPath = fileName;
	}

	return shortPath;
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

