/**
 * @file MystaffSvc.h 
 * Mystaff system service class based on QtService.
 * @author pcfist	@date 2016.03.01
 */
#pragma once

#include <QCoreApplication>
#include <QtService>
#include <QSettings>
#include <QProcess>
#include <QTimer>


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

private:
	QString mainAppPath_;
	QProcess* mainAppProcess_;

	bool running_ = false;


	void launchMainApp_(intptr_t sessionId);
};

