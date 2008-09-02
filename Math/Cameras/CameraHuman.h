#ifndef __incl_Math_Cameras_CameraHuman_h
#define __incl_Math_Cameras_CameraHuman_h

#include "Camera.h"

namespace Math { namespace Cameras {
    using namespace Math::Cameras;

    #define STEP_DEPTH           0.1f;
    #define FRAMES_PER_HALF_STEP 20;

    class CameraHuman : public Camera
    {
    protected:
        xVector3 N_front;

        xVector3 OrthoPointUp (const xVector3 &source, const xVector3 &oldUp);
        void RotatePoint      (xFLOAT &pX, xFLOAT &pY, xFLOAT angle);
        void RotatePointPitch (const xVector3 front, xFLOAT &pX, xFLOAT &pY, xFLOAT &pZ, xFLOAT angle);

        virtual void SetCamera(xFLOAT eyex, xFLOAT eyey, xFLOAT eyez,
                       xFLOAT centerx, xFLOAT centery, xFLOAT centerz,
                       xFLOAT upx, xFLOAT upy, xFLOAT upz);

    public:
        xFLOAT step;
        xFLOAT stepv;

        virtual void Move   (xFLOAT frwd, xFLOAT side, xFLOAT vert);
        virtual void Rotate (xFLOAT heading, xFLOAT pitch, xFLOAT roll);
        virtual void Orbit  (xFLOAT horz, xFLOAT vert);

        virtual void Update(xFLOAT T_delta)
        {
            Camera::Update(T_delta);
            N_front.init(P_center.x-P_eye.x, P_center.y-P_eye.y, 0).normalize();
        }

        void MakeStep(xFLOAT numFrames)
        {
            step += numFrames * PI/FRAMES_PER_HALF_STEP;
            if (step > 2*PI) step = fmodf(step, 2*PI);
            P_eye.z    -= stepv;
            P_center.z -= stepv;
            stepv = sinf(step)*STEP_DEPTH;
            P_eye.z    += stepv;
            P_center.z += stepv;
        }
    };

} } // namespace Math::Cameras

#endif

