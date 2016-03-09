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

#include "process_tools.hxx"


class MystaffSvc : public QtService<QCoreApplication>
{
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


	void launchMainApp_(intptr_t sessionId);
};

