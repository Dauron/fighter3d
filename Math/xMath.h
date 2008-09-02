#ifndef __incl_Math_xMath_h
#define __incl_Math_xMath_h

#include <cstring>
#include <cassert>
#include <cmath>

const float  PI     = 3.1415926535f;
const float  PI_inv = 1 / PI;
const double PI_d   = 3.1415926535897932384626433832795;
static inline float DegToRad(float a) { return a*0.01745329252f;}
static inline float RadToDeg(float a) { return a*57.29577951f;}
#define             DEGTORAD(a)       ((a)*0.01745329252f)
#define             RADTODEG(a)       ((a)*57.29577951f)

//#ifndef max
//#define max(a,b)            (((a) > (b)) ? (a) : (b))
//#endif

//#ifndef min
//#define min(a,b)            (((a) < (b)) ? (a) : (b))
//#endif

#ifndef WIN32
static inline float fabs(float f)           { return f < 0.f ? -f : f; }
#endif
static inline float max(float f1, float f2) { return f1 < f2 ? f2 : f1; }
static inline float min(float f1, float f2) { return f1 > f2 ? f2 : f1; }
static inline int   max(int   i1, int   i2) { return i1 < i2 ? i2 : i1; }
static inline int   min(int   i1, int   i2) { return i1 > i2 ? i2 : i1; }

#define xFLOAT_HUGE_POSITIVE    1000000000.f
#define xFLOAT_HUGE_NEGATIVE   -1000000000.f
#define EPSILON                 0.0000000001f
#define EPSILON3                0.000001f
#define EPSILON2                0.001f
static inline bool IsZero(float f)   { return fabs(f) <= EPSILON; }

static inline float Sign(float f)
{
    if (f > EPSILON) return 1.f;
    if (f < -EPSILON) return -1.f;
    return 0.f;
}

typedef unsigned char  xBYTE;
typedef unsigned short xWORD;
typedef unsigned long  xDWORD;
typedef signed   char  xCHAR;
typedef signed   short xSHORT;
typedef signed   long  xLONG;
typedef float  xFLOAT;
typedef xWORD  xWORD2    [2];
typedef xWORD  xWORD3    [3];
typedef xWORD  xWORD4    [4];
typedef xFLOAT xFLOAT2   [2];
typedef xFLOAT xFLOAT3   [3];
typedef xFLOAT xFLOAT4   [4];
typedef xFLOAT xFLOAT4x4 [4][4];

const xDWORD xDWORD_MAX = (xDWORD)-1;
const xWORD  xWORD_MAX  = (xWORD)-1;
const xBYTE  xBYTE_MAX  = (xBYTE)-1;

#include "xVector3.h"
typedef xVector3 xPoint3;

#include "xVector4.h"
typedef xVector4 xPoint4;
#include "xPlane.h"
#include "xQuaternion.h"
typedef xVector4 xColor;

#include "xMatrix.h"

inline void GetMinMax3(xFLOAT v1, xFLOAT v2, xFLOAT v3, xFLOAT &minV, xFLOAT &maxV)
{
    if (v1 >= v2)
    {
        if (v1 >= v3)
        { maxV = v1; minV = min(v2, v3); }
        else
        { maxV = v3; minV = v2; }
    }
    else
    {
        if (v2 >= v3)
        { maxV = v2; minV = min(v1, v3); }
        else
        { maxV = v3; minV = v1; }
    }
}


struct xRectangle
{
    xFLOAT X, Y, W, H;

    bool Contains(xFLOAT x, xFLOAT y)
    { return x >= X && x <= X + W && y >= Y && y <= Y + H; }

    bool Contains(int x, int y)
    { return x >= X && x <= X + W && y >= Y && y <= Y + H; }

    bool Contains(xDWORD x, xDWORD y)
    { return x >= X && x <= X + W && y >= Y && y <= Y + H; }

    xRectangle()
        : X(0.f), Y(0.f), W(0.f), H(0.f)
    {}
    xRectangle(xFLOAT x, xFLOAT y, xFLOAT w, xFLOAT h)
        : X(x), Y(y), W(w), H(h)
    {}
};

#endif
