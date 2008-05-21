#ifndef __incl_CameraHuman_h
#define __incl_CameraHuman_h

#include "Camera.h"

#define STEP_DEPTH           0.1f;
#define FRAMES_PER_HALF_STEP 20;

class CameraHuman : public Camera
{
    protected:
        xVector3 front;

        xVector3 OrthoPointUp (const xVector3 &source, const xVector3 &oldUp);
        void RotatePoint      (xFLOAT &pX, xFLOAT &pY, xFLOAT angle);
        void RotatePointPitch (const xVector3 front, xFLOAT &pX, xFLOAT &pY, xFLOAT &pZ, xFLOAT angle);

    public:
        xFLOAT step;
        xFLOAT stepv;

        CameraHuman() : Camera()
        {
            SetCamera(0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
        }
        CameraHuman(xFLOAT eyex, xFLOAT eyey, xFLOAT eyez,
               xFLOAT centerx, xFLOAT centery, xFLOAT centerz,
               xFLOAT upx, xFLOAT upy, xFLOAT upz)
               : Camera (eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz)
        {
            SetCamera(eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz);
        }
        virtual ~CameraHuman() {}

        virtual void SetCamera(xFLOAT eyex, xFLOAT eyey, xFLOAT eyez,
                       xFLOAT centerx, xFLOAT centery, xFLOAT centerz,
                       xFLOAT upx, xFLOAT upy, xFLOAT upz);

        virtual void Move   (xFLOAT frwd, xFLOAT side, xFLOAT vert);
        virtual void Rotate (xFLOAT heading, xFLOAT pitch, xFLOAT roll);
        virtual void Orbit  (xFLOAT horz, xFLOAT vert);

        void MakeStep(xFLOAT numFrames)
        {
            step += numFrames * PI/FRAMES_PER_HALF_STEP;
            if (step > 2*PI) step = fmodf(step, 2*PI);
            eye.z -= stepv;
            center.z -= stepv;
            stepv = sinf(step)*STEP_DEPTH;
            eye.z += stepv;
            center.z += stepv;
        }
};

#endif
