# MystaffSvc
System service for Mystaff application.

Service application is built with Qt Framework and depends on [QtService library](https://github.com/qtproject/qt-solutions/tree/master/qtservice) (with some modifications that allow logon event tracking). Currently works in Windows only.


## Main functionality:

1. Start the main application ([Mystaff desktop client](http://app.mystaff.com/)) when the user logs in and keep it running.
2. Pass current user information to the main application. 

## Usage

### Build:
The application can be built with Qt Creator (qmake) or using Visual Studio project files. Supports Visual C++ 2010 and higher.

### Installation:
#### Automatic installation
The service can be installed using dedicated MSI installer (it also installs the main application). The installer's source code and required binary files are located in the `mystaff-client-setup` subdirectory. MSI package can be built using [WiX Toolset](http://wixtoolset.org/releases/) or using WiX plugin Visual Studio.

#### Manual installation
To install the service manually, run the following command in the elevated command prompt:
```
MystaffSvc -i
```

This will install the service on your system. After the service is installed, you can start it using `MystaffSvc -e` or control it from Windows' built-in Task Manager or `services.msc` MMC Snap-in.

### Uninstallation:
To remove the service from your system, run in the elevated command prompt:
```
MystaffSvc -u
```

### Configuration
The service configuration values are stored in the Windows registry:
```
HKEY_LOCAL_MACHINE\SOFTWARE\[Wow6432Node\]TimeDoctor LLC\MystaffSvc\
```
(The `Wow6432Node` part should be used when running 32-bit service on 64-bit operating system.)

There is only one parameter — `MainAppPath`, which points to location of the main process (the one to be kept alive). To set it, just create a registry value of type REG_SZ with full path to the main process executable.

### Interaction with Main Application
The service interacts with the main application (Mystaff Desktop Client) mainly by passing command line parameters when launching it in each user session. The parameters are passed in the following form:
```
mystaff.exe --silent --loginInfo <user-info-json>
```
where `<user-info-json>` is a user session information block in JSON format. It can contain the following fields:
* `user`: name of the user logged into the session;
* `userSid`: a user's SID string (Windows-specific).

### Logging
The service writes events to the system log on startup and shutdown and when it launches the main process. You can view these messages using Event Viewer (`eventvwr.msc`).
To enable detailed logging, build the service in `debug` configuration. The debug build prints more detailed messages to `service-debuglog.txt` located in the system drive root directory.

## TODO:
* Add QtService as a submodule (?)
* Add support for platforms other than Windows.
