/**
 * @file UserSession.h 
 * User session wrapper class.
 * @author pcfist	@date 2016.03.08
 */
#pragma once

#include <QString>
#include <QFileInfo>
#include <QDir>

#include <windows.h>
#include <WtsApi32.h>
#include <userenv.h>

#include <QDebug>

#include "scope_guard.hxx"
#include "process_tools.hxx"


class UserSession
{
public:
	static UserSession getActiveSession() {
		return UserSession(getActiveSessionId());
	}

	static intptr_t getActiveSessionId() {
		intptr_t sid = ::WTSGetActiveConsoleSessionId();
		return sid == UINT_MAX ? 0 : sid;
	}

public:
	UserSession(intptr_t sid) : mysid_(sid) {
		::WTSQueryUserToken(mysid_, &myhandle_);
	}

	UserSession(const UserSession&) = delete;
	UserSession(UserSession&& other) : mysid_(other.mysid_), myhandle_(other.myhandle_) {
		other.myhandle_ = 0;
	}

	~UserSession() {
		if (myhandle_)
			::CloseHandle(myhandle_);
	}

	bool valid() const {
		return myhandle_ != 0;
	}

	void startProcess(const QString& targetPath) {
		STARTUPINFO si = {};
		si.cb = sizeof si;
		si.lpDesktop = L"";

		QFileInfo fi(targetPath);

		// Get working directory for target executable.
		QString wd = QDir::toNativeSeparators(fi.path());

		// Get environment block for the user session.
		void* env = nullptr;
		if (!::CreateEnvironmentBlock(&env, myhandle_, false)) {
			qDebug() << "create ENV for session" << mysid_ << "failed w/error" << GetLastError();
		}

		PROCESS_INFORMATION pi = {};
		bool result = ::CreateProcessAsUser(myhandle_, targetPath.toStdWString().c_str(), nullptr, nullptr, nullptr, false, CREATE_UNICODE_ENVIRONMENT, env,
			wd.toStdWString().c_str(), &si, &pi);

		if (result) {
			qDebug() << "CreateProcessAsUser() succeeded, pid =" << pi.dwProcessId;
		} else {
			qDebug() << "CreateProcess() -> FAIL, error code " << GetLastError();
		}

		if (env)
			::DestroyEnvironmentBlock(env);
	}



	/**
	 * Finds first instance of process with given image path in the session.
	 * @param[in]	path	- Executable image path.
	 * @return	[HANDLE]	- Process handle or 0 if not found. The handle must be closed with ::CloseHandle().
	 */
	HANDLE getProcessByExecutableName(const QString& path)
	{
		DWORD pidArray[1024];
		DWORD bytesReturned;
		if (!EnumProcesses(pidArray, sizeof pidArray, &bytesReturned))
			return 0;

		int processCount = bytesReturned / sizeof *pidArray;
		for (int i = 0; i < processCount; ++i) {
			DWORD processSid = 0;
			::ProcessIdToSessionId(pidArray[i], &processSid);
			if (processSid != mysid_)
				continue;

			HANDLE proc = ::OpenProcess(PROCESS_ALL_ACCESS, false, pidArray[i]);
			if (!proc)
				continue;

			QString imagePath = getProcessImagePath(proc);
			if (imagePath == path)
				return proc;

			::CloseHandle(proc);
		}

		return 0;
	}

protected:
	intptr_t mysid_ = 0;
	HANDLE myhandle_ = 0;
};
