/**
 * @file main.cpp 
 * Mystaff Service application entry point.
 * @author pcfist	@date 2016.03.02
 */
#include "MystaffSvc.h"

#include "debug.h"


int main(int argc, char *argv[])
{
	qInstallMessageHandler(messageHandler);

// 	while (!IsDebuggerPresent())
// 		::Sleep(500);

	MystaffSvc svc(argc, argv);
	return svc.exec();
}
