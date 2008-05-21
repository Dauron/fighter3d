#ifndef __incl_Camera_h
#define __incl_Camera_h

#include "../xMath.h"

class Camera
{
    public:
        xVector3 eye;
        xVector3 center;
        xVector3 up;

    public:
        Camera()
        {
            SetCamera(0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f);
        }
        Camera (xFLOAT eyex, xFLOAT eyey, xFLOAT eyez,
                xFLOAT centerx, xFLOAT centery, xFLOAT centerz,
                xFLOAT upx, xFLOAT upy, xFLOAT upz)
        {
            SetCamera(eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz);
        }
        virtual ~Camera() {}

        virtual void SetCamera (xFLOAT eyex, xFLOAT eyey, xFLOAT eyez,
                       xFLOAT centerx, xFLOAT centery, xFLOAT centerz,
                       xFLOAT upx, xFLOAT upy, xFLOAT upz);

        virtual void Move   (xFLOAT frwd, xFLOAT side, xFLOAT vert) = 0;
        virtual void Rotate (xFLOAT heading, xFLOAT pitch, xFLOAT roll) = 0;
        virtual void Orbit  (xFLOAT horz, xFLOAT vert) = 0;

        void LookAtMatrix(xMatrix &viewTransform);
};

#endif

