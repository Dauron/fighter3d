#ifndef __incl_Debug_h
#define __incl_Debug_h

#include "../Config.h"

void  DEB__log(bool withtime, const char *fmt, ...);
char *DEB__log_read();
char *DEB__log_tail();
void  DEB__log_clear();

#ifdef WIN32
#define LOG(fmt, ...) \
        { \
            DEB__log(true, "%s\t%d:", __FILE__, __LINE__); \
            DEB__log(false, fmt, __VA_ARGS__); \
        }
#else
#define LOG(fmt, a...) \
        { \
            DEB__log(true, "%s\t%d:", __FILE__, __LINE__); \
            DEB__log(false, fmt , ## a ); \
        }
#endif

#define LOG_READ() \
            DEB__log_read()
#define LOG_TAIL() \
            DEB__log_tail()

#ifdef WIN32
#define CheckForGLError(errorFmt, ...) \
        ( DEB_CheckForGLError(__FILE__, __LINE__) ? DEB__log(false, errorFmt, __VA_ARGS__), true : false )
#else
#define CheckForGLError(errorFmt, a...) \
        ( DEB_CheckForGLError(__FILE__, __LINE__) ? DEB__log(false, errorFmt , ## a ), true : false )
#endif

bool DEB_CheckForGLError(char *file, int line);

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
            DEB__log ("Time: %d\n", GetTickCount()-DEB__time);

#define PRINT_VALUE_I(variable)                             \
            DEB__log (#variable ": %d\n", variable)
#define PRINT_VALUE_D(variable)                             \
            DEB__log (#variable ": %3.4f\n", variable)

#define DEBLOG(fmt, ...) \
            LOG(fmt, __VA_ARGS__)

#else

#define PROFILER_START(opt)     int DEB__ver = opt; {
#define PROFILER_OPTION(opt)    } if (DEB__ver == opt) {
#define PROFILER_END            }

#define PRINT_VALUE_I(variable)
#define PRINT_VALUE_D(variable)

#define DEBLOG(message)

#endif

#endif
