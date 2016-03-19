/**
 * @file EventLog.h 
 * Wrapper for Windows Event Log Event Source.
 * @author pcfist	@date 2016.03.12
 */
#ifndef _eventlog_h_
#define _eventlog_h_

#include <QString>
#include <QDebug>

#include <windows.h>
#include <string>

#include "svc_eventlog.h"
#include "process_tools.hxx"


class EventLogSource
{
public:
	EventLogSource(const QString &srcName) : myhandle_(0), myname_(srcName)
	{
		/*
		 * Set up registry values that describe messages for our event source.
		 * It's better to have it done in the installer, but we can still write
		 * these values here just to be sure.
		 */
		QString regPath = "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\" + myname_;

		HKEY hk = 0;
		::RegCreateKeyEx(HKEY_LOCAL_MACHINE, regPath.toStdWString().c_str(), 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, nullptr, &hk, 0);
		if (hk) {
			std::wstring mypath = getProcessImagePath(::GetCurrentProcess()).toStdWString();
			::RegSetValueEx(hk, L"EventMessageFile", 0, REG_SZ, (byte*)mypath.c_str(), (mypath.length() + 1)*sizeof(wchar_t));

			DWORD typesSupported = EVENTLOG_SUCCESS | EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
			::RegSetValueEx(hk, L"TypesSupported", 0, REG_DWORD, (byte*)&typesSupported, sizeof typesSupported);

			::RegCloseKey(hk);
		}

		// Register the event source.
		myhandle_ = ::RegisterEventSource(nullptr, myname_.toStdWString().c_str());
	}

	~EventLogSource() {
		if (myhandle_)
			::DeregisterEventSource(myhandle_);

		/*
		 * Don't clean up the registry values we added at construction so that
		 * user can read our messages properly even when our service is not running.
		 * The clean-up should be done in the uninstaller.
		 */
	}

	template <int Argc>
	void logMessage(DWORD type, DWORD eventID, const QString (&args)[Argc]) {
		std::wstring strs[Argc];
		const wchar_t* strings[Argc];
		for (int i = 0; i < Argc; ++i) {
			strs[i] = args[i].toStdWString();
			strings[i] = strs[i].c_str();
		}
			
		::ReportEvent(myhandle_, type, 0, eventID, nullptr, Argc, 0, strings, nullptr);
	}

	void logMessage(DWORD type, DWORD eventID) {
		::ReportEvent(myhandle_, type, 0, eventID, nullptr, 0, 0, nullptr, nullptr);
	}


public:
	QString myname_;
	HANDLE myhandle_;
};

#endif // _eventlog_h_
