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

	qDebug() << "Main app path is " << mainAppPath_;
}


void MystaffSvc::start()
{
	running_ = true;

	launchMainApp_(UserSession::getActiveSessionId());
}

void MystaffSvc::onSessionChange(LogonEvent eventType, intptr_t sessionId)
{
	switch (eventType)
	{
	case QtServiceBase::Logon:
		qDebug() << "session " << sessionId << " LOGGED ON";
		if (running_)
			launchMainApp_(sessionId);
		break;

	case QtServiceBase::Logoff:
		qDebug() << "session " << sessionId << " LOGGED OFF";
		break;

	case QtServiceBase::Lock:
		qDebug() << "session " << sessionId << " LOCKED";
		break;

	case QtServiceBase::Unlock:
		qDebug() << "session " << sessionId << " UNLOCKED";
		if (running_)
			launchMainApp_(sessionId);
		break;

	case QtServiceBase::ConnectConsole:
		qDebug() << "session " << sessionId << " CONSOLE CONNECTED";
		if (running_)
			launchMainApp_(sessionId);
		break;

	case QtServiceBase::DisconnectConsole:
		qDebug() << "session " << sessionId << " CONSOLE DISCONNECTED";
		break;

	default:
		qDebug() << "session " << sessionId << " ???";
		break;
	}
}


void MystaffSvc::launchMainApp_(intptr_t sessionId)
{
	if (HANDLE process = getProcessByExecutableName(mainAppPath_))
	{
		qDebug() << "Main app already running!";
		::CloseHandle(process);
	}
	else
	{
		qDebug() << "Launching the main app...";

//		QProcess::startDetached(mainAppPath_);
		
		if (sessionId) {
			UserSession s(sessionId);
			s.startProcess(mainAppPath_);
		}
	}
}
