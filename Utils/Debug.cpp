#include "Debug.h"
#include <cstdio>
#include <cassert>
#include <ctime>
#include "../AppFramework/System.h"
#include <GL/gl.h>

#ifdef WIN32
#pragma warning(disable : 4996) // deprecated
#else
#include <stdarg.h>
#endif

FILE *log;

void DEB__log(bool withtime, const char *fmt, ...)
{
    if (!log)
        log = fopen("log.txt", "a+");
    assert(log);
    if (!log)
        return;

    if (withtime)
    {
        time_t rawtime;
        struct tm *timeinfo;

        time ( &rawtime );
        timeinfo = localtime ( &rawtime );
        fprintf(log, "%4d.%2.2d.%2.2d %2d:%2.2d,%2.2d\t", 1900+timeinfo->tm_year, 1+timeinfo->tm_mon, timeinfo->tm_mday,
            timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    }

    va_list ap;                        // Pointer To List Of Arguments
    va_start(ap, fmt);                 // Parses The String For Variables
    vfprintf(log, fmt, ap);            // And Converts Symbols To Actual Numbers
    va_end(ap);                        // Results Are Stored In Text

    fprintf(log, "\n");

    fclose(log);
    log = NULL;
}

char *DEB__log_read()
{
    if (!log)
        log = fopen("log.txt", "a+");
    assert(log);
    if (!log)
        return NULL;

    fseek(log, 0, SEEK_END);
    long size = ftell(log);
    char *buffer = new char[size+1];
    rewind(log);

    size = fread(buffer, 1, size, log);
    buffer[size] = 0;

    return buffer;
}

char *DEB__log_tail()
{
    if (!log)
        log = fopen("log.txt", "a+");
    assert(log);
    if (!log)
        return NULL;

    fseek(log, 0, SEEK_END);
    long size = ftell(log);
    if (size > 1024) size = 1024;
    char *buffer = new char[size+1];
    fseek(log, -size, SEEK_END);

    size = fread(buffer, 1, size, log);
    buffer[size] = 0;

    return buffer;
}

void DEB__log_clear()
{
    if (log)
    {
        fclose(log);
        log = NULL;
    }
    remove("log.txt");
}

bool DEB_CheckForGLError(char *file, int line)
{
    int error;
    bool wasE = false;

    while ((error = glGetError())) {
        if (!wasE)
            DEB__log(true, "%s\t%d:", file, line);
        DEB__log(false, "OpenGL error: %d", error);
        wasE = true;
    }
    return wasE;
}
