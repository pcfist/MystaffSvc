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

protected:
	intptr_t mysid_ = 0;
	HANDLE myhandle_ = 0;
};
