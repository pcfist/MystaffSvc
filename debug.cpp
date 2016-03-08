#include "debug.h"

#include <QFile>
#include <QTextStream>


void messageHandler(QtMsgType ty, const QMessageLogContext& ctx, const QString& msg)
{
	QFile outFile("C:\\mystaff-svc.log");
	outFile.open(QIODevice::WriteOnly);
	QTextStream ts(&outFile);
	ts << msg << endl << flush;
}
