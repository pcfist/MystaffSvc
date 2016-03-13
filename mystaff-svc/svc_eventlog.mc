;#ifndef _MYSTAFF_EVENT_LOG_MESSAGE_FILE_H_
;#define _MYSTAFF_EVENT_LOG_MESSAGE_FILE_H_

MessageIdTypeDef=DWORD


SeverityNames=(Success=0x0:STATUS_SEVERITY_SUCCESS
               Informational=0x1:STATUS_SEVERITY_INFORMATIONAL
               Warning=0x2:STATUS_SEVERITY_WARNING
               Error=0x3:STATUS_SEVERITY_ERROR
               )

LanguageNames=(EnglishUS=0x401:MSG00401
               Neutral=0x0000:MSG00000
               )

MessageId=0x0   SymbolicName=MYSTAFF_MSG_STARTUP
Severity=Informational
Facility=Application
Language=Neutral
Mystaff service started.
.

MessageId=0x1   SymbolicName=MYSTAFF_MSG_SHUTDOWN
Severity=Informational
Facility=Application
Language=Neutral
Mystaff service stopped.
.

MessageId=0x2   SymbolicName=MYSTAFF_MSG_MAIN_APP_STARTED
Severity=Success
Facility=Application
Language=Neutral
Main application successfully launched in session SID %1 (user %2), PID = %3
.

MessageId=0x3   SymbolicName=MYSTAFF_MSG_MAIN_APP_START_FAILED
Severity=Warning
Facility=Application
Language=Neutral
Main application failed to launch in session SID %1 (user %2), error code %3
.


;#endif
