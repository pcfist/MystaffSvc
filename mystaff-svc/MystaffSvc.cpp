/**
 * @file MystaffSvc.h 
 * MystaffSvc system service class implementation.
 * @author pcfist	@date 2016.03.01
 */
#include "MystaffSvc.h"

#include <QFileInfo>
#include <QDebug>

#include <windows.h>
#include <psapi.h>

#include "UserSession.h"

#include "process_tools.hxx"


/*static*/
const char MystaffSvc::myServiceName[] = "MyStaff Service";


MystaffSvc::MystaffSvc(int argc, char* argv[]) : QtService(argc, argv, myServiceName), mylog_("MystaffSvc")
{
	QCoreApplication::setOrganizationName("TimeDoctorLLC");
	QCoreApplication::setOrganizationDomain("mystaff.com");
	QCoreApplication::setApplicationName("MystaffSvc");

	// Set service description displayed in the UI.
	setServiceDescription("Mystaff Service");

	// Get service settings, use system-wide storage.
	QSettings settings(QSettings::SystemScope, "TimeDoctorLLC");
	settings.setValue("testKey", "TestValue");
	mainAppPath_ = settings.value("MainAppPath").toString();

	qDebug() << "Main app path is" << mainAppPath_;

	watchdogTimer_.setInterval(watchdogInterval);
	QObject::connect(&watchdogTimer_, SIGNAL(timeout()), SLOT(onWatchdogTimeout_()));
}


void MystaffSvc::start()
{
	mylog_.logMessage(EVENTLOG_INFORMATION_TYPE, MYSTAFF_MSG_STARTUP);

	running_ = true;
	launchMainApp_(UserSession::getActiveSessionId());
	watchdogTimer_.start();
}

void MystaffSvc::stop()
{
	watchdogTimer_.stop();
	running_ = false;

	mylog_.logMessage(EVENTLOG_INFORMATION_TYPE, MYSTAFF_MSG_SHUTDOWN);
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
		auto pid = s.startProcess(mainAppPath_);

		if (pid)
			mylog_.logMessage(EVENTLOG_SUCCESS, MYSTAFF_MSG_MAIN_APP_STARTED, QString::number(sessionId), s.userName(), QString::number(pid));
		else
			mylog_.logMessage(EVENTLOG_WARNING_TYPE, MYSTAFF_MSG_MAIN_APP_START_FAILED, QString::number(sessionId), s.userName(), QString::number(GetLastError()));
		return pid;
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
