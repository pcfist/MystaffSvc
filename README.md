# MystaffSvc
System service for Mystaff application.

Service application is built with Qt Framework and depends on [QtService library](https://github.com/qtproject/qt-solutions/tree/master/qtservice) (with some modifications that allow logon event tracking). Currently works in Windows only.


##Main functionality:

1. Start the main application ([Mystaff desktop client](http://app.mystaff.com/)) when the user logs in and keep it running.
2. Pass current user information to the main application. 

##Usage

###Build:
The application can be built with Qt Creator (qmake) or using Visual Studio project files. Supports Visual C++ 2010 and higher.

###Installation:
Run the following command in the elevated command prompt:
```
MystaffSvc -i
```

This will install the service on your system. After the service is installed, you can start it using `MystaffSvc -e` or control it from Windows' built-in Task Manager or `services.msc` MMC Snap-in.

###Uninstallation:
To remove the service from your system, run in the elevated command prompt:
```
MystaffSvc -u
```

###Configuration
The service configuration values are stored in the Windows registry:
```
HKEY_LOCAL_MACHINE\SOFTWARE\[Wow6432Node\]TimeDoctorLLC\OrganizationDefaults\
```
(The `Wow6432Node` part should be used when running 32-bit service on 64-bit operating system.)

There is only one parameter — `MainAppPath`, which points to location of the main process (the one to be kept alive). To set it, just create a registry value of type REG_SZ with full path to the main process executable.

###Logging
The service writes events to the system log on startup and shutdown and when it launches the main process. You can view these messages using Event Viewer (`eventvwr.msc`).
To enable detailed logging, build the service in `debug` configuration. The debug build prints more detailed messages to `service-debuglog.txt` located in the system drive root directory.

##TODO:
* Add QtService as a submodule (?)
* Add support for platforms other than Windows.
