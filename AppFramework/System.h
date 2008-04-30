#ifndef __incl_System_h
#define __incl_System_h

#ifdef WIN32

#include <windows.h>

#else

typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef long LONG;

#endif

#include <ctime>

inline float GetTick()
{
   return clock() / (float)CLOCKS_PER_SEC * 1000.f;
}

#endif

