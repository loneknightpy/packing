#include "log.h"

FILE *LogFile = stderr;
LogLevel logLevel = LogInfo;

const char *LogText[] = 
{
    "Off: ",
    "Info: ",
    "Warning: ",
    "Error: ",
};

void Log(LogLevel level, const char *format, ...)
{
    va_list         args;
    va_start(args, format);
    if (level >= logLevel)
    {
        fprintf(LogFile, LogText[logLevel]);
        vfprintf(LogFile, format, args);
    }
    va_end(args);
}
