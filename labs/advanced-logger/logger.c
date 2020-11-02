#include <stdio.h>
#include <stdarg.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <syslog.h>

#define RESET 0
#define BRIGHT 1
#define DIM 2
#define UNDERLINE 3
#define BLINK 4
#define REVERSE 7
#define HIDDEN 8
#define RED 1
#define GREEN 2
#define YELLOW 3
#define BLUE 4
#define MAGENTA 5
#define CYAN 6
#define WHITE 7
#define STDOUT 0
#define SYSLOG 1

int logDest = 0;

int initLogger(char *logType)
{
    if (strcmp(logType, "syslog") == 0)
    {
        logDest = SYSLOG;
        printf("Initializing Logger on: %s\n", logType);
    }
    else if (strcmp(logType, "stdout") == 0)
    {
        logDest = STDOUT;
        printf("Initializing Logger on: %s\n", logType);
    }
    else
    {
        printf("Invalid log destination");
    }
    return 0;
}

void textcolor(int attr, int fg, int bg)
{
    char command[13];
    sprintf(command, "%c[%d;%d;%dm", 0x1B, attr, fg + 30, bg + 40);
    printf("%s", command);
}

int printWithFormat(char *type, int color, const char *format, va_list arg)
{
    int done;
    textcolor(BRIGHT, color, HIDDEN);
    printf("%s", type);
    done = vfprintf(stdout, format, arg);
    textcolor(RESET, WHITE, HIDDEN);
    return done;
}

int infof(const char *format, ...)
{
    
    va_list arg;
    va_start(arg, format);
    if(logDest == STDOUT) {
        int done;
        done = printWithFormat("INFO: ", WHITE, format, arg);
        va_end(arg);
        return done;
    } else {
        openlog("syslog", LOG_NDELAY, LOG_DAEMON);
        syslog(LOG_INFO, format, arg);
        closelog();
        va_end(arg);
        return 0;
    }
}

int errorf(const char *format, ...)
{
    va_list arg;
    va_start(arg, format);
    if(logDest == STDOUT) {
        int done;
        done = printWithFormat("ERROR: ", RED, format, arg);
        va_end(arg);
        return done;
    } else {
        openlog("syslog", LOG_NDELAY, LOG_DAEMON);
        syslog(LOG_PERROR, format, arg);
        closelog();
        va_end(arg);
        return 0;
    }
}

int panicf(const char *format, ...)
{
    
    va_list arg;
    va_start(arg, format);
    if(logDest == STDOUT) {
        int done;
        done = printWithFormat("PANIC: ", GREEN, format, arg);
        va_end(arg);
        return done;
    } else {
        openlog("syslog", LOG_NDELAY, LOG_DAEMON);
        syslog(LOG_INFO, format, arg);
        closelog();
        va_end(arg);
        return 0;
    }
}

int warnf(const char *format, ...)
{
    
    va_list arg;
    va_start(arg, format);
    if(logDest == STDOUT) {
        int done;
        done = printWithFormat("WARNING: ", YELLOW, format, arg);
        va_end(arg);
        return done;
    } else {
        openlog("syslog", LOG_NDELAY, LOG_DAEMON);
        syslog(LOG_WARNING, format, arg);
        closelog();
        va_end(arg);
        return 0;
    }
}