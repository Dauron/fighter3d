#ifndef __incl_Math_Cameras_CameraFree_h
#define __incl_Math_Cameras_CameraFree_h

#include "Camera.h"

namespace Math { namespace Cameras {
    using namespace Math::Cameras;

    class CameraFree : public Camera
    {
    public:
        virtual void Move   (xFLOAT frwd, xFLOAT side, xFLOAT vert);
        virtual void Rotate (xFLOAT heading, xFLOAT pitch, xFLOAT roll);
        virtual void Orbit  (xFLOAT horz, xFLOAT vert);
    };

} } // namespace Math::Cameras

#endif
