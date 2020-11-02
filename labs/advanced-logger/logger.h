// Logger

int initLogger(char *logType);
void textcolor(int attr, int fg, int bg);
int printWithFormat(char *type, int color, const char *format, va_list arg);
int infof(const char *format, ...);
int warnf(const char *format, ...);
int errorf(const char *format, ...);
int panicf(const char *format, ...);
