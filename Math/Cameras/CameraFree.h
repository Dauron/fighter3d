#ifndef __incl_CameraFree_h
#define __incl_GLCamereFree_h

#include "Camera.h"

class CameraFree : public Camera
{
    public:
        CameraFree() : Camera() {}
        CameraFree(xFLOAT eyex, xFLOAT eyey, xFLOAT eyez,
               xFLOAT centerx, xFLOAT centery, xFLOAT centerz,
               xFLOAT upx, xFLOAT upy, xFLOAT upz)
               : Camera (eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz)
        {}
        virtual ~CameraFree() {}

        virtual void Move   (xFLOAT frwd, xFLOAT side, xFLOAT vert);
        virtual void Rotate (xFLOAT heading, xFLOAT pitch, xFLOAT roll);
        virtual void Orbit  (xFLOAT horz, xFLOAT vert);
};

#endif

