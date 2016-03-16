# MystaffSvc
System service for Mystaff application.

Service application is built with Qt Framework and depends on [QtService library](https://github.com/qtproject/qt-solutions/tree/master/qtservice) (with some modifications that allow logon event tracking). Currently works in Windows only.


##Main functionality:

1. Start the main application ([Mystaff desktop client](http://app.mystaff.com/)) when the user logs in and keep it running.
2. Pass current user information to the main application.

##TODO:
* Add QtService as a submodule (?)
* Add support for platforms other than Windows.
