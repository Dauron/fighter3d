#ifndef __incl_Math_xMath_h
#define __incl_Math_xMath_h

#include <cstring>
#include <cassert>
#include <cmath>

const float  PI   = 3.1415926535f;
const double PI_d = 3.1415926535897932384626433832795;
static inline float DegToRad(float a) { return a*0.01745329252f;}
static inline float RadToDeg(float a) { return a*57.29577951f;}

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

#define EPSILON  0.0000000001
#define EPSILON2 0.0001
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
#include "xVector4.h"
#include "xQuaternion.h"
#include "xMatrix.h"

typedef xVector4 xPlane;
typedef xVector4 xColor;

struct xBox
{
    xVector3 min;
    xVector3 max;

    xVector3 points[8];

    bool Contains(const xVector3 &vert) const
    {
        return vert >= min && vert <= max;
    }

    const xVector3 *TransformatedPoints(xMatrix transformation)
    {
        points[0] = min;
        points[1] = max;
        points[2].init(min.x, min.y, max.z);
        points[3].init(min.x, max.y, max.z);
        points[4].init(max.x, min.y, max.z);
        points[5].init(max.x, max.y, min.z);
        points[6].init(max.x, min.y, min.z);
        points[7].init(min.x, max.y, min.z);

        points[0] = transformation.preTransformP(points[0]);
        points[1] = transformation.preTransformP(points[1]);
        points[2] = transformation.preTransformP(points[2]);
        points[3] = transformation.preTransformP(points[3]);
        points[4] = transformation.preTransformP(points[4]);
        points[5] = transformation.preTransformP(points[5]);
        points[6] = transformation.preTransformP(points[6]);
        points[7] = transformation.preTransformP(points[7]);

        return points;
    }

    const xVector3 *UnTransformatedPoints()
    {
        points[0] = min;
        points[1] = max;
        points[2].init(min.x, min.y, max.z);
        points[3].init(min.x, max.y, max.z);
        points[4].init(max.x, min.y, max.z);
        points[5].init(max.x, max.y, min.z);
        points[6].init(max.x, min.y, min.z);
        points[7].init(min.x, max.y, min.z);
        return points;
    }

    bool culledBy(const xPlane *planes, int count)
    {
        UnTransformatedPoints();
        // See if there is one plane for which all of the
        // vertices are in the negative half space.
        for (int p = 0; p < count; ++p) {

            bool culled = true;
            int v;
            // Assume this plane culls all points.  See if there is a point
            // not culled by the plane... early out when at least one point
            // is in the positive half space.
            for (v = 0; (v < 8) && culled; ++v)
                culled = xVector3::DotProduct(planes[p].vector3, points[v]) + planes[p].w < 0;
            if (culled) return true;
        }
        // None of the planes could cull this box
        return false;
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
