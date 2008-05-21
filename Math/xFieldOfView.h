#ifndef __incl_lib3dx_xFieldOfView_h
#define __incl_lib3dx_xFieldOfView_h

#include "xMath.h"

struct xFieldOfView
{
public:
    xPlane  LeftPlane;
    xPlane  RightPlane;
    xPlane  TopPlane;
    xPlane  BottomPlane;

    xFLOAT  Angle, Aspect;
    xFLOAT  FrontClip;
    xFLOAT  BackClip;

    xMatrix  ViewTransform;

    void init( xFLOAT angle, xFLOAT aspect, xFLOAT frontClip, xFLOAT backClip )
    {
        Angle     = angle;
        Aspect    = aspect;
        FrontClip = frontClip;
        BackClip  = backClip;

        xVector3 p0, p1, p2;

        p0.init(0.f,0.f,0.f);
        p1.z = BackClip;
        p1.y = -p1.z * (xFLOAT)tan( Angle * 0.5f );
        p1.x = p1.y * Aspect;
        p2.init(p1.x, -p1.y, p1.z);
        LeftPlane.planeFromPoints(p0, p1, p2);

        p1.y = -p1.y; p2.x = -p2.x;
        TopPlane.planeFromPoints(p0, p1, p2);

        p1.x = -p1.x; p2.y = -p1.y;
        RightPlane.planeFromPoints(p0, p1, p2);

        p1.y = -p1.y; p2.x = -p1.x;
        BottomPlane.planeFromPoints(p0, p1, p2);
    }

    bool CheckSphere(const xVector3 &sphereCenter, xFLOAT sphereRadius) const
    {
        float dist;

        xVector3 viewPos;
        viewPos.z = -(ViewTransform.z0 * sphereCenter.x
                    + ViewTransform.z1 * sphereCenter.y
                    + ViewTransform.z2 * sphereCenter.z
                    + ViewTransform.z3);
        // In front of the Front Clipping plane
        if ( viewPos.z + sphereRadius < FrontClip )
            return false;
        // Behind the Back Clipping plane
        if ( viewPos.z - sphereRadius > BackClip )
            return false;

        viewPos.x = -(ViewTransform.x0 * sphereCenter.x
                    + ViewTransform.x1 * sphereCenter.y
                    + ViewTransform.x2 * sphereCenter.z
                    + ViewTransform.x3);
        // Test the left plane
        dist = viewPos.x * LeftPlane.x + viewPos.z * LeftPlane.z;
        if ( dist > sphereRadius )
            return false;
        // Test the right plane
        dist = viewPos.x * RightPlane.x + viewPos.z * RightPlane.z;
        if ( dist > sphereRadius )
            return false;

        viewPos.y = -(ViewTransform.y0 * sphereCenter.x
                    + ViewTransform.y1 * sphereCenter.y
                    + ViewTransform.y2 * sphereCenter.z
                    + ViewTransform.y3);
        // Test the top plane
        dist = viewPos.y * TopPlane.y + viewPos.z * TopPlane.z;
        if ( dist > sphereRadius )
            return false;
        // Test the right plane
        dist = viewPos.y * BottomPlane.y + viewPos.z * BottomPlane.z;
        if ( dist > sphereRadius )
            return false;

        // Inside the field of view
        return true;
    }

    bool CheckBox(const xVector3 boundingPoints[8]) const
    {
        xVector3 viewPos[8];
        const xVector3 *iterS;
        xVector3 *iterD;
        bool culled;
        int v;

        iterS = boundingPoints;
        iterD = viewPos;
        for (v = 8; v; --v, ++iterS, ++iterD)
            iterD->z = -(ViewTransform.z0 * iterS->x
                       + ViewTransform.z1 * iterS->y
                       + ViewTransform.z2 * iterS->z
                       + ViewTransform.z3);
        // In front of the Front Clipping plane
        iterD = viewPos; culled = true;
        for (v = 8; v && culled; --v, ++iterD)
            culled = iterD->z < FrontClip;
        if (culled) return false;
        // Behind the Back Clipping plane
        iterD = viewPos; culled = true;
        for (v = 8; v && culled; --v, ++iterD)
            culled = iterD->z > BackClip;
        if (culled) return false;

        iterS = boundingPoints;
        iterD = viewPos;
        for (v = 8; v; --v, ++iterS, ++iterD)
            iterD->x = -(ViewTransform.x0 * iterS->x
                       + ViewTransform.x1 * iterS->y
                       + ViewTransform.x2 * iterS->z
                       + ViewTransform.x3);
        // Test the left plane
        iterD = viewPos; culled = true;
        for (v = 8; v && culled; --v, ++iterD)
            culled = iterD->x * LeftPlane.x + iterD->z * LeftPlane.z > 0;
        if (culled) return false;
        // Test the right plane
        iterD = viewPos; culled = true;
        for (v = 8; v && culled; --v, ++iterD)
            culled = iterD->x * RightPlane.x + iterD->z * RightPlane.z > 0;
        if (culled) return false;

        iterS = boundingPoints;
        iterD = viewPos;
        for (v = 8; v; --v, ++iterS, ++iterD)
            iterD->x = -(ViewTransform.y0 * iterS->x
                       + ViewTransform.y1 * iterS->y
                       + ViewTransform.y2 * iterS->z
                       + ViewTransform.y3);
        // Test the top plane
        iterD = viewPos; culled = true;
        for (v = 8; v && culled; --v, ++iterD)
            culled = iterD->y * TopPlane.x + iterD->z * TopPlane.z > 0;
        if (culled) return false;
        // Test the bottom plane
        iterD = viewPos; culled = true;
        for (v = 8; v && culled; --v, ++iterD)
            culled = iterD->y * BottomPlane.x + iterD->z * BottomPlane.z > 0;
        if (culled) return false;

        // None of the planes could cull this box
        return true;
    }
};

#endif
