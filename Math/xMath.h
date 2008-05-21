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

#include "xVector3.h"
#include "xVector4.h"
#include "xQuaternion.h"
#include "xMatrix.h"

xVector3 operator + (const xVector3 &a, const xVector3 &b);
xVector3 operator - (const xVector3 &a, const xVector3 &b);
xVector3 operator * (float f, const xVector3 &v);
xVector3 operator * (const xVector3 &v, float f);
xVector3 operator * (const xMatrix &a,  const xVector3 &b);
xVector3 operator * (const xVector3 &a, const xMatrix &b);
xVector3 operator / (const xVector3 &v, float f);
xVector3 operator - (const xVector3 &a);
bool     operator ==(const xVector3 &a, const xVector3 &b);
bool     operator <=(const xVector3 &a, const xVector3 &b);
bool     operator >=(const xVector3 &a, const xVector3 &b);

xVector4 operator + (const xVector4 &a, const xVector4 &b);
xMatrix  operator + (const xMatrix &a, const xMatrix &b);
xVector4 operator - (const xVector4 &a, const xVector4 &b);
xVector4 operator * (float f, const xVector4 &v);
xVector4 operator * (const xVector4 &v, float f);
xVector4 operator * (const xMatrix &a,  const xVector4 &b);
xVector4 operator * (const xVector4 &a, const xMatrix &b);
xMatrix  operator * (const xMatrix &a, const xMatrix &b);
xMatrix  operator * (const xMatrix &m, xFLOAT f);
xMatrix  operator * (xFLOAT f, const xMatrix &m);
xVector4 operator / (const xVector4 &v, float f);
xVector4 operator - (const xVector4 &a);

bool operator == (const xMatrix &a, const xMatrix &b);
bool operator != (const xMatrix &a, const xMatrix &b);

xMatrix  xMatrixFromQuaternion(xVector4 q);
xVector4 xMatrixToQuaternion(const xMatrix &m);
xMatrix  xMatrixRotateRad(xFLOAT radX, xFLOAT radY, xFLOAT radZ);
xMatrix  xMatrixTranslate(xFLOAT x, xFLOAT y, xFLOAT z);
xMatrix  xMatrixTranslate(const xFLOAT3 xyz);

#endif
