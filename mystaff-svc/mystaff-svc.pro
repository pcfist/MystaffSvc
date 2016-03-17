TEMPLATE = app
TARGET = MystaffSvc
QT += core
CONFIG += console qt
INCLUDEPATH += .
	
HEADERS += ./EventLog.h \
    ./process_tools.hxx \
    ./resource.h \
    ./scope_guard.hxx \
    ./UserSession.h \
    ./MystaffSvc.h
SOURCES += ./main.cpp \
    ./MystaffSvc.cpp

win32|win64 {
    mcTarget.target = svc_eventlog.mc
    mcTarget.depends = FORCE
    mcTarget.commands = mc.exe -A -b -c -h $$PWD -r $$PWD $$PWD/svc_eventlog.mc
    QMAKE_EXTRA_TARGETS += mcTarget
    PRE_TARGETDEPS += svc_eventlog.mc

    buildhook.depends = mcTarget
    CONFIG(debug,debug|release):buildhook.target = Makefile.Debug
    CONFIG(release,debug|release):buildhook.target = Makefile.Release
    QMAKE_EXTRA_TARGETS += buildhook

    RC_FILE += svc_eventlog.rc

    DEPENDPATH += .
}

win32:LIBS += -lpsapi -lwtsapi32 -luserenv -ladvapi32


include(../qtservice/src/qtservice.pri)
