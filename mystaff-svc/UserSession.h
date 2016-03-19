/**
 * @file UserSession.h 
 * User session wrapper class.
 * @author pcfist	@date 2016.03.08
 */
#ifndef _usersession_h_
#define _usersession_h_


#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QDebug>

#include <windows.h>
#include <WtsApi32.h>
#include <userenv.h>

#include "scope_guard.hxx"
#include "process_tools.hxx"


class UserSession
{
public:
	static UserSession getActiveSession()
	{
		return UserSession(getActiveSessionId());
	}

	static sid_t getActiveSessionId()
	{
		sid_t sid = ::WTSGetActiveConsoleSessionId();
		return sid == UINT_MAX ? 0 : sid;
	}

	/**
	 * Returns list of user session IDs.
	 * @return	[QVector<intptr_t>]	- List of user session IDs.
	 */
	static QVector<sid_t> getSessionIDs()
	{
		WTS_SESSION_INFO *si = nullptr;
		DWORD sessionCount = 0;
		::WTSEnumerateSessions(WTS_CURRENT_SERVER_HANDLE, 0, 1, &si, &sessionCount);

		QVector<sid_t> sessions;
		sessions.reserve(sessionCount);
		for (DWORD i = 0; i < sessionCount; ++i)
			sessions.push_back(si[i].SessionId);

		return sessions;
	}

public:
	UserSession(sid_t sid) : mysid_(sid), myhandle_(0)
	{
		::WTSQueryUserToken(mysid_, &myhandle_);

		// Get session's user name.
		DWORD level = 1;
		WTS_SESSION_INFO_1 *si = nullptr;
		DWORD count = 0;
		if (::WTSEnumerateSessionsEx(WTS_CURRENT_SERVER_HANDLE, &level, 0, &si, &count)) {
			for (int i = 0; i < count; ++i) {
				if (si[i].SessionId != mysid_)
					continue;

				myuserName_ = QString::fromWCharArray(si[i].pUserName);
				break;
			}
		}
	}

	UserSession(UserSession &&other) : mysid_(other.mysid_), myhandle_(other.myhandle_) {
		other.myhandle_ = 0;
	}

	~UserSession() {
		if (myhandle_)
			::CloseHandle(myhandle_);
	}

	bool valid() const {
		return myhandle_ != 0;
	}

	/**
	 * Starts the process in user session.
	 * @param[in]	targetPath	- Path to the executable file.
	 * @return	[pid_t]	- New process ID or 0 if the process failed to start.
	 */
	pid_t startProcess(const QString &targetPath) {
		STARTUPINFO si = {};
		si.cb = sizeof si;
        si.lpDesktop = const_cast<wchar_t *>(L"");

		QFileInfo fi(targetPath);

		// Get working directory for target executable.
		QString wd = QDir::toNativeSeparators(fi.path());

		// Get environment block for the user session.
		void *env = nullptr;
		if (!::CreateEnvironmentBlock(&env, myhandle_, false)) {
			qDebug() << "create ENV for session" << mysid_ << "failed w/error" << GetLastError();
		}

		PROCESS_INFORMATION pi = {};
		bool result = ::CreateProcessAsUser(myhandle_, targetPath.toStdWString().c_str(), nullptr, nullptr, nullptr, false, CREATE_UNICODE_ENVIRONMENT, env,
			wd.toStdWString().c_str(), &si, &pi) != FALSE;

		if (result) {
			qDebug() << "CreateProcessAsUser() succeeded, pid =" << pi.dwProcessId;
		} else {
			qDebug() << "CreateProcess() -> FAIL, error code " << GetLastError();
		}

		if (env)
			::DestroyEnvironmentBlock(env);

		return pi.dwProcessId;
	}

	/**
	 * Finds first instance of process with given image path in the session.
	 * @param[in]	path	- Executable image path.
	 * @return	[HANDLE]	- Process handle or 0 if not found. The handle must be closed with ::CloseHandle().
	 */
	HANDLE getProcessByExecutableName(const QString &path)
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

			HANDLE proc = ::OpenProcess(PROCESS_QUERY_INFORMATION, false, pidArray[i]);
			if (!proc)
				continue;

			QString imagePath = getProcessImagePath(proc);
			if (imagePath == path)
				return proc;

			::CloseHandle(proc);
		}

		return 0;
	}

	/**
	 * Returns name of user logged into this session (if any).
	 * @return	[const QString&]	- User name, or empty string if no user is associated with the session.
	 */
	const QString &userName() const {
		return myuserName_;
	}

	/**
	 * Returns true if a user is logged on to this session.
	 * @return	[bool]	- true if session is associated with user.
	 */
	bool isDesktopSession() const {
		return !myuserName_.isEmpty();
	}

protected:
	sid_t mysid_;
	HANDLE myhandle_;
	QString myuserName_;

private:
	// DELETED
	UserSession(const UserSession &);
};


#endif // _usersession_h_
