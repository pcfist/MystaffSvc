/**
 * @file MystaffSvc.h 
 * Mystaff system service class based on QtService.
 * @author pcfist	@date 2016.03.01
 */
#pragma once

#include <QCoreApplication>
#include <QtService>
#include <QSettings>
#include <QTimer>

#include "process_tools.hxx"
#include "EventLog.h"


class MystaffSvc : public QObject, public QtService<QCoreApplication>
{
	Q_OBJECT
public:
	MystaffSvc(int argc, char *argv[]);

	void start() override;
	void stop() override;

	/**
	 * Logon event handler.
	 * @param[in]	eventType	- Logon event type.
	 * @param[in]	sessionId	- User session ID.
	 */
	void onSessionChange(LogonEvent eventType, intptr_t sessionId) override;

private:
	static
	const char myServiceName[];

	static const int watchdogInterval = 3000;	// Main process check interval [ms]

	enum AppLaunchResult {
		LaunchFailed = 0,	// An error occurred when launching the process.
		AlreadyRunning = (pid_t)(-1),	// Process is already running.
	};

private:
	EventLogSource mylog_;
	QString mainAppPath_;
	QTimer watchdogTimer_;
	// Mutex held when launching main application to avoid race conditions.
	QMutex watchdogMutex_;

	bool running_;


	/**
	 * Runs main application in the specified user session.
	 * @param[in]	sessionId	- User session ID.
	 * @return	[pid_t]	- Process ID if launched successfully or one of AppLaunchResult enum values.
	 * @see AppLaunchResult
	 */
	pid_t launchMainApp_(intptr_t sessionId);

	/**
	 * Writes debug log message about main process launch attempt.
	 * @param[in]	sid	- User session ID.
	 * @param[in]	result	- Process launch result (PID) as returned by UserSession::startProcess().
	 * @param[in]	message	- Event message.
	 */
	void reportAppLaunchResult_(sid_t sid, pid_t result, const char *message);


private slots:
	/**
	 * Watchdog timer callback.
	 * Keeps the main application running.
	 */
	void onWatchdogTimeout_();
};

