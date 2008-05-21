#ifndef __incl_Math_xMath_h
#define __incl_Math_xMath_h

#include <cstring>
#include <cassert>
#include <cmath>

const float  PI   = 3.1415926535f;
const double PI_d = 3.1415926535897932384626433832795;
static inline float DegToRad(float a) { return a*0.01745329252f;}
static inline float RadToDeg(float a) { return a*57.29577951f;}

#ifndef WIN32
//static inline float abs(float f)            { return f < 0.f ? -f : f; }
static inline float max(float f1, float f2) { return f1 < f2 ? f2 : f1; }
static inline float min(float f1, float f2) { return f1 > f2 ? f2 : f1; }
#endif
#define EPSILON 0.0000000001
static inline bool IsZero(float f)   { return fabs(f) <= EPSILON; }

static inline float Sign(float f)
{
    if (f > 0.f) return 1.f;
    if (f < 0.f) return -1.f;
    return 0.f;
}

typedef unsigned char  xBYTE;
typedef unsigned short xWORD;
typedef unsigned long  xDWORD;
typedef short  xSHORT;
typedef long   xLONG;
typedef float  xFLOAT;
typedef xWORD  xWORD3    [3];
typedef xFLOAT xFLOAT2   [2];
typedef xFLOAT xFLOAT3   [3];
typedef xFLOAT xFLOAT4   [4];
typedef xFLOAT xFLOAT4x4 [4][4];

const xDWORD xDWORD_MAX = (xDWORD)-1;
const xWORD  xWORD_MAX  = (xWORD)-1;

#include "xVector3.h"
#include "xVector4.h"
#include "xQuaternion.h"
#include "xMatrix.h"

typedef xVector4 xPlane;

struct xBox
{
    xVector3 min;
    xVector3 max;

    bool Contains(const xVector3 &vert) const
    {
        return vert >= min && vert <= max;
    }
};

bool operator == (const xMatrix &a, const xMatrix &b);
bool operator != (const xMatrix &a, const xMatrix &b);

xMatrix  xMatrixFromQuaternion(xVector4 q);
xVector4 xMatrixToQuaternion(const xMatrix &m);
xMatrix  xMatrixRotateRad(xFLOAT radX, xFLOAT radY, xFLOAT radZ);
xMatrix  xMatrixTranslate(xFLOAT x, xFLOAT y, xFLOAT z);
xMatrix  xMatrixTranslate(const xFLOAT3 xyz);
xMatrix  xMatrixTranslateT(xFLOAT x, xFLOAT y, xFLOAT z);
xMatrix  xMatrixTranslateT(const xFLOAT3 xyz);

#endif
