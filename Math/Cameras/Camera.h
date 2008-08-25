#ifndef __incl_Math_Cameras_Camera_h
#define __incl_Math_Cameras_Camera_h

#include <string>
#include "FieldOfView.h"
#include "../Tracking/ObjectTracker.h"

namespace Math { namespace Cameras {
    using namespace Math::Cameras;
    using namespace Math::Tracking;

    class Camera
    {
    private:
        xMatrix       MX_WorldToView;
        xMatrix       MX_ViewToWorld;

    public:
        std::string   SN_name;

        ObjectTracker EyeTracker;
        ObjectTracker CenterTracker;
        xFLOAT        W_TrackingSpeed;

        FieldOfView   FOV;

        xPoint3       P_eye;
        xPoint3       P_center;
        xVector3      NW_up;

    public:
        static const char *SCRIPT_EyeSeeAll_Center;
        static const char *SCRIPT_EyeSeeAll_CenterTop;
        static const char *SCRIPT_EyeSeeAll_Radius;

        virtual void Init()
        { Init (0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f); }
        virtual void Init (xFLOAT eyex, xFLOAT eyey, xFLOAT eyez,
                xFLOAT centerx, xFLOAT centery, xFLOAT centerz,
                xFLOAT upx, xFLOAT upy, xFLOAT upz)
        {
            SN_name.clear();
            SetCamera(eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz);
            EyeTracker.Init();
            CenterTracker.Init();
            W_TrackingSpeed = 1.f;
            FOV.InitCamera(this);
        }

        virtual void Update(xFLOAT T_delta);

        const xMatrix &MX_WorldToView_Get() const { return MX_WorldToView; }
        const xMatrix &MX_ViewToWorld_Get() const { return MX_ViewToWorld; }

        virtual void Move   (xFLOAT frwd, xFLOAT side, xFLOAT vert) = 0;
        virtual void Rotate (xFLOAT heading, xFLOAT pitch, xFLOAT roll) = 0;
        virtual void Orbit  (xFLOAT horz, xFLOAT vert) = 0;

    protected:
        virtual void SetCamera (xFLOAT eyex, xFLOAT eyey, xFLOAT eyez,
                       xFLOAT centerx, xFLOAT centery, xFLOAT centerz,
                       xFLOAT upx, xFLOAT upy, xFLOAT upz);

    private:
        void LookAtMatrix(xMatrix &MX_WorldToView);

        void Script_EyeSeeAll_Center(ObjectTracker &tracker, const xPoint3 &P_current);
        void Script_EyeSeeAll_CenterTop(ObjectTracker &tracker, const xPoint3 &P_current);
        void Script_EyeSeeAll_Radius(ObjectTracker &tracker, const xPoint3 &P_current);
    };

} } // namespace Math::Cameras

#endif

