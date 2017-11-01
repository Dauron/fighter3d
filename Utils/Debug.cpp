#include "Debug.h"
#include <cstdio>
#include <cassert>
#include <ctime>
#include "../AppFramework/System.h"
#include "Filesystem.h"
#include <GL/gl.h>

#ifndef WIN32
#include <stdarg.h>
#endif

FILE *log_file;

void log(int level, const char *fmt, ...)
{
    if (level > Config::LoggingLevel) return;

    if (!log_file)
        log_file = fopen(Filesystem::GetFullPath("Data/log.txt").c_str(), "a+");
    assert(log_file);
    if (!log_file)
        return;

    va_list ap;                        // Pointer To List Of Arguments
    va_start(ap, fmt);                 // Parses The String For Variables
    vfprintf(log_file, fmt, ap);            // And Converts Symbols To Actual Numbers
    va_end(ap);                        // Results Are Stored In Text

    fprintf(log_file, "\n");

    //fclose(log_file);
    fflush(log_file);
    log_file = NULL;
}

void logEx(int level, bool withtime, const char *fmt, ...)
{
    if (level > Config::LoggingLevel) return;

    if (!log_file)
        log_file = fopen(Filesystem::GetFullPath("Data/log.txt").c_str(), "a+");
    assert(log_file);
    if (!log_file)
        return;

    if (withtime)
    {
        time_t rawtime;
        struct tm *timeinfo;

        time ( &rawtime );
        timeinfo = localtime ( &rawtime );
        fprintf(log_file, "%4d.%2.2d.%2.2d %2d:%2.2d,%2.2d\t", 1900+timeinfo->tm_year, 1+timeinfo->tm_mon, timeinfo->tm_mday,
            timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    }

    va_list ap;                        // Pointer To List Of Arguments
    va_start(ap, fmt);                 // Parses The String For Variables
    vfprintf(log_file, fmt, ap);            // And Converts Symbols To Actual Numbers
    va_end(ap);                        // Results Are Stored In Text

    fprintf(log_file, "\n");

    //fclose(log_file);
    fflush(log_file);
    log_file = NULL;
}

char *log_read()
{
    if (!log_file)
        log_file = fopen(Filesystem::GetFullPath("Data/log.txt").c_str(), "a+");
    assert(log_file);
    if (!log_file)
        return NULL;

    fseek(log_file, 0, SEEK_END);
    long size = ftell(log_file);
    char *buffer = new char[size+1];
    rewind(log_file);

    size = fread(buffer, 1, size, log_file);
    buffer[size] = 0;

    return buffer;
}

char *log_tail()
{
    if (!log_file)
        log_file = fopen(Filesystem::GetFullPath("Data/log.txt").c_str(), "a+");
    assert(log_file);
    if (!log_file)
        return NULL;

    fseek(log_file, 0, SEEK_END);
    long size = ftell(log_file);
    if (size > 1024) size = 1024;
    char *buffer = new char[size+1];
    fseek(log_file, -size, SEEK_END);

    size = fread(buffer, 1, size, log_file);
    buffer[size] = 0;

    return buffer;
}

void log_clear()
{
    if (log_file)
    {
        fclose(log_file);
        log_file = NULL;
    }
    remove(Filesystem::GetFullPath("Data/log.txt").c_str());
}

bool _CheckForGLError(const char *file, int line)
{
    int error;
    bool wasE = false;

    while ((error = glGetError())) {
        if (!wasE)
            logEx(2, true, "%s\t%d:", file, line);
        logEx(2, false, "OpenGL error: %d", error);
        wasE = true;
    }
    return wasE;
}
