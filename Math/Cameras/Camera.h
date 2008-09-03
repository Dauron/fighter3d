#ifndef __incl_Math_Cameras_Camera_h
#define __incl_Math_Cameras_Camera_h

#include <string>
#include "FieldOfView.h"
#include "../Tracking/ObjectTracker.h"

namespace Math { namespace Cameras {
    using namespace Math::Cameras;
    using namespace Math::Tracking;

    struct CameraTrackingData {
        xPoint3 *P_current;
        Camera  *camera;

        CameraTrackingData ()
            : P_current(NULL), camera(NULL)
        {}
        CameraTrackingData (xPoint3 &p_current, Camera  &cam)
            : P_current(&p_current), camera(&cam)
        {}
    };

    class Camera
    {
    private:
        xMatrix       MX_WorldToView;
        xMatrix       MX_ViewToWorld;

        CameraTrackingData eyeTrackingData;
        CameraTrackingData centerTrackingData;

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
        //static const char *SCRIPT_EyeSeeAll_Center;
        //static const char *SCRIPT_EyeSeeAll_CenterTop;
        //static const char *SCRIPT_EyeSeeAll_Radius;
        static const TrackingScript SCRIPT_EyeSeeAll_Center;
        static const TrackingScript SCRIPT_EyeSeeAll_CenterTop;
        static const TrackingScript SCRIPT_EyeSeeAll_Radius;

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
            eyeTrackingData    = CameraTrackingData(P_eye, *this);
            centerTrackingData = CameraTrackingData(P_center, *this);
            EyeTracker.ScriptData    = (xBYTE*)&eyeTrackingData;
            CenterTracker.ScriptData = (xBYTE*)&centerTrackingData;

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
    };

} } // namespace Math::Cameras

#endif

