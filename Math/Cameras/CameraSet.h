#ifndef __incl_Math_Cameras_CameraSet_h
#define __incl_Math_Cameras_CameraSet_h

#include "Camera.h"

namespace Math { namespace Cameras {
    using namespace Math::Cameras;

    class CameraSet
    {
    public:
        typedef std::vector<Camera*> Vec_Camera;

        Vec_Camera L_cameras;

        virtual void Update(xFLOAT T_delta)
        {
            Vec_Camera::iterator CM_curr = L_cameras.begin(),
                                 CM_last = L_cameras.end();
            for (; CM_curr != CM_last; ++CM_curr )
                (**CM_curr).Update(T_delta);
        }

        virtual void Init()
        {
            L_cameras.clear();
        }

        virtual void Free()
        {
            Vec_Camera::iterator CM_curr = L_cameras.begin(),
                                 CM_last = L_cameras.end();
            for (; CM_curr != CM_last; ++CM_curr )
                delete *CM_curr;
            L_cameras.clear();
        }

        void Load(const char *fileName);
    };

} } // namespace Math::Cameras

#endif
