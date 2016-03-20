/**
 * @file MystaffSvc.h 
 * MystaffSvc system service class implementation.
 * @author pcfist	@date 2016.03.01
 */
#include "MystaffSvc.h"

#include <QFileInfo>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

#include <windows.h>
#include <psapi.h>

#include "UserSession.h"

#include "process_tools.hxx"


/*static*/
const char MystaffSvc::myServiceName[] = "MyStaff Service";


MystaffSvc::MystaffSvc(int argc, char *argv[]) : QtService(argc, argv, myServiceName), mylog_("MystaffSvc"), running_(false)
{
	QCoreApplication::setOrganizationName("TimeDoctor LLC");
	QCoreApplication::setOrganizationDomain("mystaff.com");
	QCoreApplication::setApplicationName("MystaffSvc");

	// Set service description displayed in the UI.
	setServiceDescription("Mystaff Service");

	// Get service settings, use system-wide storage.
	QSettings settings(QSettings::SystemScope, "TimeDoctor LLC", "MystaffSvc");
	mainAppPath_ = settings.value("MainAppPath").toString();

	watchdogTimer_.setInterval(watchdogInterval);
	QObject::connect(&watchdogTimer_, SIGNAL(timeout()), SLOT(onWatchdogTimeout_()));
}


void MystaffSvc::start()
{
	qDebug() << "Main app path is" << mainAppPath_;

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

	switch (eventType) {
	case QtServiceBase::Logon: {
		pid_t pid = launchMainApp_(sessionId);
		reportAppLaunchResult_(sessionId, pid, "LOGGED ON");
		} break;

	case QtServiceBase::Logoff:
		qDebug() << "session" << sessionId << "LOGGED OFF";
		break;

	case QtServiceBase::Lock:
		qDebug() << "session" << sessionId << "LOCKED";
		break;

	case QtServiceBase::Unlock: {
		pid_t pid = launchMainApp_(sessionId);
		reportAppLaunchResult_(sessionId, pid, "UNLOCKED");
		} break;

	case QtServiceBase::ConnectConsole: {
		pid_t pid = launchMainApp_(sessionId);
		reportAppLaunchResult_(sessionId, pid, "CONSOLE CONNECTED");
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
	// Use scope_guard to make sure we always release the mutex when exiting this function.
	auto mutexGuard = make_scope_guard([this]() {
		watchdogMutex_.unlock();
	});

	if (HANDLE process = s.getProcessByExecutableName(mainAppPath_)) {
		::CloseHandle(process);
		return AlreadyRunning;
	} else {
		QString cmdLine = makeMainAppCmdLine_(s);
		qDebug() << "launching" << mainAppPath_ << cmdLine << "...";
		pid_t pid = s.startProcess(mainAppPath_, cmdLine);

		if (pid) {
			QString args[] = { QString::number(sessionId), s.userName(), QString::number(pid) };
			mylog_.logMessage(EVENTLOG_SUCCESS, MYSTAFF_MSG_MAIN_APP_STARTED, args);
		} else {
			QString args[] = { QString::number(sessionId), s.userName(), QString::number(GetLastError()) };
			mylog_.logMessage(EVENTLOG_WARNING_TYPE, MYSTAFF_MSG_MAIN_APP_START_FAILED, args);
		}
		return pid;
	}
}

QString MystaffSvc::makeMainAppCmdLine_(const UserSession &session)
{
	QString args = "--silent";

	QJsonObject json;
	json.insert("user", session.userName());
	if (!session.userSid().isEmpty()) {
		qDebug() << "make cmd line: user SID =" << session.userSid();
		json.insert("userSid", session.userSid());
	}

	args += ' ';
	QJsonDocument jsonDoc;
	jsonDoc.setObject(json);
	args += "--loginInfo " + jsonDoc.toJson(QJsonDocument::Compact);

	return args;
}

void MystaffSvc::reportAppLaunchResult_(sid_t sid, pid_t result, const char* message)
{
	if (result == LaunchFailed) {
		qWarning() << "session" << sid << message << "-> failed to launch main app";
	} else if (result != AlreadyRunning) {
		qDebug() << "session" << sid << message << "-> launched main app, pid =" << result;
	} else {
		qDebug() << "session" << sid << message;
	}
}

void MystaffSvc::onWatchdogTimeout_()
{
	QVector<sid_t> sessions = UserSession::getSessionIDs();

	for (int i = 0; i < sessions.size(); ++i) {
		pid_t result = launchMainApp_(sessions[i]);

		if (result == LaunchFailed) {
			qWarning() << "Could not start main application in session" << sessions[i];
		} else if (result != AlreadyRunning) {
			qDebug() << "Main application not found in session" << sessions[i] << "-> launched successfully, pid =" << result;
		}
	}
}
