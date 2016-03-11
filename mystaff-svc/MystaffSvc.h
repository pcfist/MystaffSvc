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


class MystaffSvc : public QObject, public QtService<QCoreApplication>
{
	Q_OBJECT
public:
	MystaffSvc(int argc, char* argv[]);

	void start() override;

	void onSessionChange(LogonEvent eventType, intptr_t sessionId) override;

private:
	static
	const char myServiceName[];

	static const int watchdogInterval = 3000;	// [ms]

	enum AppLaunchResult {
		LaunchFailed = 0,
		AlreadyRunning = (pid_t)(-1),
	};

private:
	QString mainAppPath_;
	QTimer watchdogTimer_;
	// Mutex held when launching main application to avoid race conditions.
	QMutex watchdogMutex_;

	bool running_ = false;


	pid_t launchMainApp_(intptr_t sessionId);

private slots:
	/**
	 * Watchdog timer callback.
	 * Keeps the main application running.
	 */
	void onWatchdogTimeout_();
};

