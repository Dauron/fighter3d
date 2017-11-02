#ifndef __incl_Math_Cameras_FieldOfView_h
#define __incl_Math_Cameras_FieldOfView_h

#include "../xMath.h"
#include "../Figures/xBoxA.h"
#include "../Figures/xBoxO.h"
#include "../Figures/xSphere.h"

namespace Math { namespace Cameras {
    class Camera;

    struct FieldOfView
    {
    private:
        Camera *camera;

    public:
        bool    Empty;

        enum eProjection {
            PROJECT_UNDEFINED,
            PROJECT_PERSPECTIVE,
            PROJECT_ORTHOGONAL
        } Projection;

        union {
            xFLOAT  PerspAngle;
            xFLOAT  OrthoScale;
        };
        xFLOAT  Aspect;
        xFLOAT  FrontClip;
        xFLOAT  BackClip;
        xMatrix MX_Projection;

        xLONG   ViewportLeft;
        xLONG   ViewportTop;
        xLONG   ViewportWidth;
        xLONG   ViewportHeight;

        xFLOAT  ViewportLeftPercent;
        xFLOAT  ViewportTopPercent;
        xFLOAT  ViewportWidthPercent;
        xFLOAT  ViewportHeightPercent;

        xPlane  LeftPlane;
        xPlane  RightPlane;
        xPlane  TopPlane;
        xPlane  BottomPlane;

        xPoint3 Corners3D[4];
        xPlane  Planes[5];

        FieldOfView() : Empty(true) { Projection = PROJECT_UNDEFINED; Aspect = 1.333f; }

        void InitCamera(Camera *camera) { this->camera = camera; }

        void InitViewportPercent ( xFLOAT leftPcnt,    xFLOAT topPcnt,
                                   xFLOAT widthPcnt,   xFLOAT heightPcnt,
                                   xDWORD windowWidth, xDWORD windowHeight );
        void ResizeViewport ( xDWORD windowWidth, xDWORD windowHeight );

        void InitViewport   ( xDWORD left, xDWORD top, xDWORD width, xDWORD height,
                              xDWORD windowWidth = 0, xDWORD windowHeight = 0 );
        void InitPerspective( xFLOAT angle = 45.f, xFLOAT frontClip = 0.1f,
                              xFLOAT backClip = xFLOAT_HUGE_POSITIVE );
        void InitOrthogonal ( xFLOAT frontClip = 0.1f, xFLOAT backClip = 1000.f );

        void Update();

        const xMatrix &MX_Projection_Get() const
        { return MX_Projection; }

        const Camera *Camera_Get() const
        { return camera; }

        bool    ViewportContains(xLONG ScreenX, xLONG ScreenY)
        {
            return ScreenX >= ViewportLeft && ScreenY >= ViewportTop &&
                   ScreenX <= ViewportLeft + ViewportWidth &&
                   ScreenY <= ViewportTop + ViewportHeight;
        }
        xPoint3 Get3dPos(xLONG ScreenX, xLONG ScreenY, xPlane  PN_plane);
        xPoint3 Get3dPos(xLONG ScreenX, xLONG ScreenY, xPoint3 P_onPlane);

        bool CheckSphere(const xPoint3 &P_center, xFLOAT S_radius) const;
        bool CheckSphere(const Math::Figures::xSphere &sphere) const
        { return CheckSphere(sphere.P_center, sphere.S_radius); }

        bool CheckBox(const xPoint3 P_corners[8]) const;
        bool CheckBox(Math::Figures::xBoxA &box, const xMatrix &MX_LocaltoWorld) const
        {
            if (Empty || box.P_min == box.P_max) return true;
            return CheckBox(box.fillCornersTransformed(MX_LocaltoWorld));
        }
        bool CheckBox(Math::Figures::xBoxO &box) const
        {
            if (Empty || box.S_top == 0.f) return true;
            return CheckBox(box.P_corners);
        }

        bool CheckPoints(const xPoint4 *P_points, xWORD I_count) const;
        bool CheckPoints(const xPoint3 *P_points, xWORD I_count) const;
    };

} } // namespace Math::Cameras

#endif
