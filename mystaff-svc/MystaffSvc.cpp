#include "MystaffSvc.h"

#include <QFileInfo>
#include <QDebug>

#include <windows.h>
#include <psapi.h>

#include "UserSession.h"

#include "process_tools.hxx"


/*static*/
const char MystaffSvc::myServiceName[] = "MyStaff Service";

HANDLE getProcessByExecutableName(const QString& path)
{
	DWORD pidArray[1024];
	DWORD bytesReturned;
	if (!EnumProcesses(pidArray, sizeof pidArray, &bytesReturned))
		return 0;

	int processCount = bytesReturned/sizeof *pidArray;
	for (int i = 0; i < processCount; ++i) {
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

MystaffSvc::MystaffSvc(int argc, char* argv[]) : QtService(argc, argv, myServiceName)
{
	QCoreApplication::setOrganizationName("TimeDoctorLLC");
	QCoreApplication::setOrganizationDomain("mystaff.com");
	QCoreApplication::setApplicationName("MystaffSvc");

	setServiceDescription("Mystaff Service");

	// Add debug privilege to access the process information.
	getDebugPrivilege();

	QSettings settings(QSettings::SystemScope, "TimeDoctorLLC");
	settings.setValue("testKey", "TestValue");
	mainAppPath_ = settings.value("MainAppPath").toString();

	qDebug() << "Main app path is" << mainAppPath_;

	watchdogTimer_.setInterval(watchdogInterval);
	QObject::connect(&watchdogTimer_, SIGNAL(timeout()), SLOT(onWatchdogTimeout_()));
}


void MystaffSvc::start()
{
	running_ = true;

	launchMainApp_(UserSession::getActiveSessionId());
	watchdogTimer_.start();
}

void MystaffSvc::onSessionChange(LogonEvent eventType, intptr_t sessionId)
{
	if (!running_)
		return;

	switch (eventType)
	{
	case QtServiceBase::Logon: {
		auto pid = launchMainApp_(sessionId);
		if (pid == LaunchFailed) {
			qWarning() << "session" << sessionId << "LOGGED ON -> failed to launch main app";
		} else if (pid != AlreadyRunning) {
			qDebug() << "session" << sessionId << "LOGGED ON -> launched main app, pid =" << pid;
		} else {
			qDebug() << "session" << sessionId << "LOGGED ON";
		}
		} break;

	case QtServiceBase::Logoff:
		qDebug() << "session" << sessionId << "LOGGED OFF";
		break;

	case QtServiceBase::Lock:
		qDebug() << "session" << sessionId << "LOCKED";
		break;

	case QtServiceBase::Unlock: {
		auto pid = launchMainApp_(sessionId);
		if (pid == LaunchFailed) {
			qWarning() << "session" << sessionId << "UNLOCKED -> failed to launch main app";
		} else if (pid != AlreadyRunning) {
			qDebug() << "session" << sessionId << "UNLOCKED -> launched main app, pid =" << pid;
		} else {
			qDebug() << "session" << sessionId << "UNLOCKED";
		}
		} break;

	case QtServiceBase::ConnectConsole: {
		auto pid = launchMainApp_(sessionId);
		if (pid == LaunchFailed) {
			qWarning() << "session" << sessionId << "CONSOLE CONNECTED -> failed to launch main app";
		} else if (pid != AlreadyRunning) {
			qDebug() << "session" << sessionId << "CONSOLE CONNECTED -> launched main app, pid =" << pid;
		} else {
			qDebug() << "session" << sessionId << "CONSOLE CONNECTED";
		}
	} break;

	case QtServiceBase::DisconnectConsole:
		qDebug() << "session" << sessionId << "CONSOLE DISCONNECTED";
		break;
	}
}


pid_t MystaffSvc::launchMainApp_(intptr_t sessionId)
{
	UserSession s(sessionId);
	/*
	 * Check if this session has a user logged on. It is possible to have a session
	 * that is not associated with any user (for example, such a session is created
	 * when 'Switch User' screen is active). We don't need to launch our app in such
	 * sessions because it will be run under System account.
	 */
	if (!s.isDesktopSession()) {
		return AlreadyRunning;
	}
	
	watchdogMutex_.lock();
	auto mutexGuard = make_scope_guard([this]{ watchdogMutex_.unlock(); });

	if (HANDLE process = s.getProcessByExecutableName(mainAppPath_))
	{
		::CloseHandle(process);
		return AlreadyRunning;
	}
	else
	{
		return s.startProcess(mainAppPath_);
	}
}

void MystaffSvc::onWatchdogTimeout_()
{
	auto sessions = UserSession::getSessionIDs();

	for (auto sid : sessions) {
		auto result = launchMainApp_(sid);

		if (result == LaunchFailed) {
			qWarning() << "Could not start main application in session" << sid;
		} else if (result != AlreadyRunning) {
			qDebug() << "Main application not found in session" << sid << "-> launched successfully, pid =" << result;
		}
	}
}
