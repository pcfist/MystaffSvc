/**
 * @file main.cpp 
 * Mystaff Service application entry point.
 * @author pcfist	@date 2016.03.02
 */
#include "MystaffSvc.h"


int main(int argc, char *argv[])
{
	MystaffSvc svc(argc, argv);
	return svc.exec();
}
