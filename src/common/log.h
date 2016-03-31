#ifndef __LOG_H_

#define __LOG_H_

#include <stdio.h>
#include <stdarg.h>

enum LogLevel { LogOff, LogInfo, LogWarning, LogError };

void Log(LogLevel level, const char *format, ...);

#endif
