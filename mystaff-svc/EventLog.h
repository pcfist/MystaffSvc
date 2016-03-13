/**
 * @file EventLog.h 
 * Wrapper for Windows Event Log Event Source.
 * @author pcfist	@date 2016.03.12
 */
#pragma once

#include <windows.h>
#include <QString>
#include <QDebug>

#include "svc_eventlog.h"
#include "process_tools.hxx"


class EventLogSource
{
public:
	EventLogSource(const QString& srcName) : myname_(srcName)
	{
		QString regPath = "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\" + myname_;

		HKEY hk = 0;
		::RegCreateKeyEx(HKEY_LOCAL_MACHINE, regPath.toStdWString().c_str(), 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, nullptr, &hk, 0);
		if (hk) {
			auto mypath = getProcessImagePath(::GetCurrentProcess()).toStdWString();
			::RegSetValueEx(hk, L"EventMessageFile", 0, REG_SZ, (byte*)mypath.c_str(), (mypath.length() + 1)*sizeof(wchar_t));

			DWORD typesSupported = EVENTLOG_SUCCESS | EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
			::RegSetValueEx(hk, L"TypesSupported", 0, REG_DWORD, (byte*)&typesSupported, sizeof typesSupported);

			::RegCloseKey(hk);
		}

		myhandle_ = ::RegisterEventSource(nullptr, myname_.toStdWString().c_str());
	}

	~EventLogSource() {
		if (myhandle_)
			::DeregisterEventSource(myhandle_);
	}

	template <class... Args>
	void logMessage(DWORD type, DWORD eventID, Args&&... args) {
		std::wstring strs[sizeof...(args)] = { (QString(args).toStdWString())... };
		const wchar_t* strings[sizeof...(args)];
		for (int i = 0; i < sizeof...(args); ++i)
			strings[i] = strs[i].c_str();
			
		::ReportEvent(myhandle_, type, 0, eventID, nullptr, sizeof...(args), 0, strings, nullptr);
	}

	template <>
	void logMessage(DWORD type, DWORD eventID) {
		::ReportEvent(myhandle_, type, 0, eventID, nullptr, 0, 0, nullptr, nullptr);
	}


public:
	QString myname_;
	HANDLE myhandle_ = 0;
};
