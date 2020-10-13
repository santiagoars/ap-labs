#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include "logger.h"

//Finishing
                
int loggerType;

int myVPrintf(const char *format, va_list arg){
    int r = 0;
    if(loggerType){
        vsyslog(LOG_INFO, format, arg);
    }else{
        r = vprintf(format, arg);
    }
	printf("\033[0m"); 
	va_end(arg);
	return r;
}

int initLogger(char *logType) {
    if(!logType || strcmp(logType, "stdout") == 0 || !logType[0]){
        if(loggerType == 1){
            printf("Initializing Logger on: stdout\n");
            loggerType = 0;
            closelog();
        } else{
            printf("Already initialized on: stdout\n");
        }
    }
    else if(strcmp(logType, "syslog") == 0){
        if(loggerType == 0){
            printf("Initializing Logger on: syslog\n");
            openlog(NULL, LOG_NDELAY, LOG_USER);
            loggerType = 1;
        } else{
            printf("Already initialized on: syslog\n");
        }     
    }
    else{
        errorf("Logger option '%s' is unknown\n", logType);
        return -1;
    }
    return 0;
}

int infof(const char *format, ...){
	printf("\033[0m"); 
	va_list arg;
	va_start(arg, format);
	return myVPrintf(format, arg);
}

int warnf(const char *format, ...){
	printf("\033[0;33m");
	va_list arg;
	va_start(arg, format);
	return myVPrintf(format, arg);
}

int errorf(const char *format, ...){
	printf("\033[0;31m"); 
	va_list arg;
	va_start(arg, format);
	return myVPrintf(format, arg);
}

int panicf(const char *format, ...){
	printf("\033[1;35m"); 
	va_list arg;
	va_start(arg, format);
	myVPrintf(format, arg);
	abort();
	return -1;
}