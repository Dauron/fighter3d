#ifndef __incl_Debug_h
#define __incl_Debug_h

#include "../Config.h"
#include <cstdio>

void  logEx(bool withtime, const char *fmt, ...);
void  log(const char *fmt, ...);
char *log_read();
char *log_tail();
void  log_clear();

#ifdef WIN32
#define DEB_LOG(fmt, ...) \
        { \
            logEx(true, "%s\t%d:", __FILE__, __LINE__); \
            logEx(false, fmt, __VA_ARGS__); \
        }
#else
#define DEB_LOG(fmt, a...) \
        { \
            logEx(true, "%s\t%d:", __FILE__, __LINE__); \
            logEx(false, fmt , ## a ); \
        }
#endif

#ifdef WIN32
#define CheckForGLError(errorFmt, ...) \
        ( _CheckForGLError(__FILE__, __LINE__) ? logEx(false, errorFmt, __VA_ARGS__), true : false )
#else
#define CheckForGLError(errorFmt, a...) \
        ( _CheckForGLError(__FILE__, __LINE__) ? logEx(false, errorFmt , ## a ), true : false )
#endif

bool _CheckForGLError(char *file, int line);

#ifndef NDEBUG

#include "../AppFramework/System.h" // GetTickCount
#include <cstdio>

#define PROFILER_START(opt)                                 \
            int DEB__time = GetTickCount();                 \
            int DEB__ver = opt;                             \
            for (int DEB__i=0; DEB__i<360*1000; ++DEB__i)   \
            {{

#define PROFILER_OPTION(opt)                                \
                }                                           \
                if (DEB__ver == opt) {

#define PROFILER_END                                        \
            }}                                              \
            log ("Time: %d\n", GetTickCount()-DEB__time);

#define PRINT_VALUE_I(variable)                             \
            log (#variable ": %d\n", variable)
#define PRINT_VALUE_D(variable)                             \
            log (#variable ": %3.4f\n", variable)

#else

#define PROFILER_START(opt)     int DEB__ver = opt; {
#define PROFILER_OPTION(opt)    } if (DEB__ver == opt) {
#define PROFILER_END            }

#define PRINT_VALUE_I(variable)
#define PRINT_VALUE_D(variable)

#endif

#endif
