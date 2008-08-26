#include "FieldOfView.h"
#include "Camera.h"

using namespace Math::Cameras;

void FieldOfView :: InitPerspective( xFLOAT angleDeg, xFLOAT frontClip, xFLOAT backClip )
{
    Empty      = false;
    PerspAngle = angleDeg;
    FrontClip  = frontClip;
    BackClip   = backClip;
    Projection = PROJECT_PERSPECTIVE;

    xFLOAT Width, Height;
    Height = frontClip * tan( DegToRad(angleDeg) * 0.5f );
    Width  = Height * Aspect;
    xFLOAT W_2_rt_minus_lt_Inv = 1.f / Width;
    xFLOAT W_2_tp_minus_bt_Inv = 1.f / Height;
    if (backClip == xFLOAT_HUGE_POSITIVE)
    {
        MX_Projection.row0.init(frontClip*W_2_rt_minus_lt_Inv, 0.f, 0.f, 0.f);
        MX_Projection.row1.init(0.f, frontClip*W_2_tp_minus_bt_Inv, 0.f, 0.f);
        MX_Projection.row2.init(0.f, 0.f, -1.f, -1.f); // x = 2(rt - lt) * W_2_rt_minus_lt_Inv, y = 2(tp - bt) * W_2_tp_minus_bt_Inv
        MX_Projection.row3.init(0.f, 0.f, -2.f*frontClip, 0.f);
    }
    else
    {
        xFLOAT W_bc_minus_fr_Inv = 1.f / (backClip - frontClip);
        MX_Projection.row0.init(frontClip*W_2_rt_minus_lt_Inv, 0.f, 0.f, 0.f);
        MX_Projection.row1.init(0.f, frontClip*W_2_tp_minus_bt_Inv, 0.f, 0.f);
        MX_Projection.row2.init(0.f, 0.f, -(frontClip+backClip)*W_bc_minus_fr_Inv, -1.f); // x = 2(rt - lt) * W_2_rt_minus_lt_Inv, y = 2(tp - bt) * W_2_tp_minus_bt_Inv
        MX_Projection.row3.init(0.f, 0.f, -2.f*(frontClip*backClip)*W_bc_minus_fr_Inv, 0.f);
    }

    xVector3 p0, p1, p2;

    p0.init(0.f,0.f,0.f);
    p1.z = BackClip;
    p1.y = -p1.z * (xFLOAT)tan( PerspAngle * 0.5f );
    p1.x = p1.y * Aspect;
    p2.init(p1.x, -p1.y, p1.z);
    LeftPlane.init(p0, p1, p2);

    p1.y = -p1.y; p2.x = -p2.x;
    TopPlane.init(p0, p1, p2);

    p1.x = -p1.x; p2.y = -p1.y;
    RightPlane.init(p0, p1, p2);

    p1.y = -p1.y; p2.x = -p1.x;
    BottomPlane.init(p0, p1, p2);
}

void FieldOfView :: InitOrthogonal ( xFLOAT frontClip, xFLOAT backClip )
{
    xFLOAT scale = (camera->P_eye - camera->P_center).length();

    Empty      = false;
    OrthoScale = scale;
    FrontClip  = frontClip;
    BackClip   = backClip;
    Projection = PROJECT_ORTHOGONAL;

    xFLOAT W_2_tp_minus_bt_Inv = 1.f / (scale);
    xFLOAT W_2_rt_minus_lt_Inv = 1.f / (scale * Aspect);
    if (backClip == xFLOAT_HUGE_POSITIVE)
    {
        MX_Projection.row0.init( W_2_rt_minus_lt_Inv, 0.f, 0.f, 0.f); // w = 2(rt + lt) * W_2_rt_minus_lt_Inv
        MX_Projection.row1.init(0.f,  W_2_tp_minus_bt_Inv, 0.f, 0.f); // w = 2(tp + bt) * W_2_tp_minus_bt_Inv
        MX_Projection.row2.init(0.f, 0.f, 0.f, 0.f);                  // w = 2(ft + bc) * W_2_bc_minus_fr_Inv
        MX_Projection.row3.init(0.f, 0.f, -1.f, 1.f);
    }
    else
    {
        xFLOAT W_2_bc_minus_fr_Inv = 2.f / (backClip - frontClip);
        MX_Projection.row0.init( W_2_rt_minus_lt_Inv, 0.f, 0.f, 0.f); // x3 = 0.5(rt + lt) * W_2_rt_minus_lt_Inv
        MX_Projection.row1.init(0.f,  W_2_tp_minus_bt_Inv, 0.f, 0.f); // y3 = 0.5(tp + bt) * W_2_tp_minus_bt_Inv
        MX_Projection.row2.init(0.f, 0.f, -W_2_bc_minus_fr_Inv, 0.f);
        MX_Projection.row3.init(0.f, 0.f, -0.5f*(frontClip+backClip)*W_2_bc_minus_fr_Inv, 1.f);
    }

    xVector3 p0;
    p0.init(-scale * Aspect, -scale, 0.f);
    LeftPlane.init(xVector3::Create(1.f, 0.f, 0.f), p0);
    TopPlane.init(xVector3::Create(0.f, 1.f, 0.f), p0);
    p0.init(scale * Aspect, scale, 0.f);
    RightPlane.init(xVector3::Create(-1.f, 0.f, 0.f), p0);
    BottomPlane.init(xVector3::Create(0.f, -1.f, 0.f), p0);
}

void FieldOfView :: InitViewport   ( xDWORD left, xDWORD top, xDWORD width, xDWORD height,
                                     xDWORD windowWidth, xDWORD windowHeight )
{
    if (windowWidth == 0)  windowWidth  = left + width;
    if (windowHeight == 0) windowHeight = top + height;

    ViewportLeftPercent   = ((xFLOAT)left) / windowWidth;
    ViewportWidthPercent  = ((xFLOAT)width) / windowWidth;
    ViewportTopPercent    = ((xFLOAT)top) / windowHeight;
    ViewportHeightPercent = ((xFLOAT)height) / windowHeight;
    ResizeViewport(windowWidth, windowHeight);
}

void FieldOfView :: InitViewportPercent ( xFLOAT leftPcnt,    xFLOAT topPcnt,
                                          xFLOAT widthPcnt,   xFLOAT heightPcnt,
                                          xDWORD windowWidth, xDWORD windowHeight )
{
    ViewportLeftPercent   = leftPcnt;
    ViewportTopPercent    = topPcnt;
    ViewportWidthPercent  = widthPcnt;
    ViewportHeightPercent = heightPcnt;
    ResizeViewport(windowWidth, windowHeight);
}

void FieldOfView :: ResizeViewport ( xDWORD windowWidth, xDWORD windowHeight )
{
    ViewportLeft   = (xDWORD)(ViewportLeftPercent   * windowWidth);
    ViewportWidth  = (xDWORD)(ViewportWidthPercent  * windowWidth);
    ViewportTop    = (xDWORD)(ViewportTopPercent    * windowHeight);
    ViewportHeight = (xDWORD)(ViewportHeightPercent * windowHeight);
    Aspect         = ViewportWidth / (xFLOAT)ViewportHeight;

    if (Projection == PROJECT_PERSPECTIVE)
        InitPerspective(PerspAngle, FrontClip, BackClip);
    else
    if (Projection == PROJECT_ORTHOGONAL)
        InitOrthogonal(FrontClip, BackClip);
}

void FieldOfView :: Update()
{
    const xMatrix &MX_ViewToWorld = camera->MX_ViewToWorld_Get();

    if (Projection == PROJECT_PERSPECTIVE)
    {
        Corners3D[0].z = FrontClip;
        Corners3D[0].y = -Corners3D[0].z * (xFLOAT)tan( PerspAngle * 0.5f );
        Corners3D[0].x = Corners3D[0].y * Aspect;
        Corners3D[1].init(-Corners3D[0].x,  Corners3D[0].y, FrontClip);
        Corners3D[2].init(-Corners3D[0].x, -Corners3D[0].y, FrontClip);
        Corners3D[3].init( Corners3D[0].x, -Corners3D[0].y, FrontClip);
        xPoint3 eye; eye.zero();

        Corners3D[0] = MX_ViewToWorld.preTransformP(Corners3D[0]);
        Corners3D[1] = MX_ViewToWorld.preTransformP(Corners3D[1]);
        Corners3D[2] = MX_ViewToWorld.preTransformP(Corners3D[2]);
        Corners3D[3] = MX_ViewToWorld.preTransformP(Corners3D[3]);
        eye = MX_ViewToWorld.preTransformP(eye);

        Planes[0].init(Corners3D[1], Corners3D[3], Corners3D[0]);
        Planes[1].init(eye, Corners3D[3], Corners3D[0]);
        Planes[2].init(eye, Corners3D[2], Corners3D[3]);
        Planes[3].init(eye, Corners3D[1], Corners3D[2]);
        Planes[4].init(eye, Corners3D[0], Corners3D[1]);
    }
    else
    {
        xVector3 p0;
        Corners3D[0].init(-OrthoScale * Aspect, -OrthoScale, FrontClip);
        Corners3D[1].init(-Corners3D[0].x,  Corners3D[0].y, FrontClip);
        Corners3D[2].init(-Corners3D[0].x, -Corners3D[0].y, FrontClip);
        Corners3D[3].init( Corners3D[0].x, -Corners3D[0].y, FrontClip);

        Corners3D[0] = MX_ViewToWorld.preTransformP(Corners3D[0]);
        Corners3D[1] = MX_ViewToWorld.preTransformP(Corners3D[1]);
        Corners3D[2] = MX_ViewToWorld.preTransformP(Corners3D[2]);
        Corners3D[3] = MX_ViewToWorld.preTransformP(Corners3D[3]);

        Planes[0].init(MX_ViewToWorld.preTransformV(xVector3::Create(0.f,0.f,1.f)), Corners3D[0]);
        Planes[1].init(MX_ViewToWorld.preTransformV(xVector3::Create(-1.f,0.f,0.f)), Corners3D[0]);
        Planes[2].init(MX_ViewToWorld.preTransformV(xVector3::Create(0.f,-1.f,0.f)), Corners3D[0]);
        Planes[3].init(MX_ViewToWorld.preTransformV(xVector3::Create(1.f,0.f,0.f)), Corners3D[2]);
        Planes[4].init(MX_ViewToWorld.preTransformV(xVector3::Create(0.f,1.f,0.f)), Corners3D[2]);
    }
}

xPoint3 FieldOfView :: Get3dPos(xLONG ScreenX, xLONG ScreenY, xPoint3 P_onPlane)
{
    if (!ViewportContains(ScreenX, ScreenY))
        return xPoint3::Create(xFLOAT_HUGE_NEGATIVE, xFLOAT_HUGE_NEGATIVE, xFLOAT_HUGE_NEGATIVE);

    xFLOAT norm_x = 1.f - ScreenX / (ViewportWidth * 0.5f);
    xFLOAT norm_y = 1.f - ScreenY / (ViewportHeight * 0.5f);

    // get ray of the mouse
    xVector3 N_ray;
    xPoint3  P_ray;
    if (Projection == PROJECT_PERSPECTIVE)
    {
        xFLOAT near_height = 0.1f * tan( DegToRad( PerspAngle ) * 0.5f );
        P_ray = camera->MX_ViewToWorld_Get().preTransformP(xPoint3::Create(0.f,0.f,0.f));
        N_ray.init(near_height * Aspect * norm_x, near_height * norm_y, 0.1f);
    }
    else
    {
        P_ray = camera->MX_ViewToWorld_Get().preTransformP(
            xPoint3::Create(-OrthoScale * Aspect * norm_x, -OrthoScale * norm_y, 0.0f) );
        N_ray.init(0.f, 0.f, 0.1f);
    }
    N_ray = camera->MX_ViewToWorld_Get().preTransformV(N_ray);

    // get plane of ray intersection
    xPlane PN_plane; PN_plane.init((camera->P_eye - camera->P_center).normalize(), P_onPlane);
    return PN_plane.intersectRay(P_ray, N_ray);
}

bool FieldOfView :: CheckSphere(const xPoint3 &P_center, xFLOAT S_radius) const
{
    if (Empty || S_radius == 0.f) return true;

    for (int ip = 0; ip < 5; ++ip)
        if (Planes[ip].distanceToPoint(P_center) > S_radius)
            return false;

    return true;
/*
    const xMatrix &MX_WorldToView = camera->MX_WorldToView_Get();

    float dist;

    xPoint3 viewPos;
    viewPos.z = -(MX_WorldToView.z0 * P_center.x
                + MX_WorldToView.z1 * P_center.y
                + MX_WorldToView.z2 * P_center.z
                + MX_WorldToView.z3);
    // In front of the Front Clipping plane
    if ( viewPos.z + S_radius < FrontClip )
        return false;
    // Behind the Back Clipping plane
    if ( viewPos.z - S_radius > BackClip )
        return false;

    if (Projection == PROJECT_PERSPECTIVE)
    {
        viewPos.x = -(MX_WorldToView.x0 * P_center.x
                    + MX_WorldToView.x1 * P_center.y
                    + MX_WorldToView.x2 * P_center.z
                    + MX_WorldToView.x3);
        // Test the left plane
        dist = viewPos.x * LeftPlane.x + viewPos.z * LeftPlane.z;
        if ( dist > S_radius )
            return false;
        // Test the right plane
        dist = viewPos.x * RightPlane.x + viewPos.z * RightPlane.z;
        if ( dist > S_radius )
            return false;

        viewPos.y = -(MX_WorldToView.y0 * P_center.x
                    + MX_WorldToView.y1 * P_center.y
                    + MX_WorldToView.y2 * P_center.z
                    + MX_WorldToView.y3);
        // Test the top plane
        dist = viewPos.y * TopPlane.y + viewPos.z * TopPlane.z;
        if ( dist > S_radius )
            return false;
        // Test the right plane
        dist = viewPos.y * BottomPlane.y + viewPos.z * BottomPlane.z;
        if ( dist > S_radius )
            return false;
    }
    else
    {
        viewPos.x = -(MX_WorldToView.x0 * P_center.x
                    + MX_WorldToView.x1 * P_center.y
                    + MX_WorldToView.x2 * P_center.z
                    + MX_WorldToView.x3);
        // Test the left plane
        dist = viewPos.x * LeftPlane.x + LeftPlane.w;
        if ( dist > S_radius )
            return false;
        // Test the right plane
        dist = viewPos.x * RightPlane.x + RightPlane.w;
        if ( dist > S_radius )
            return false;

        viewPos.y = -(MX_WorldToView.y0 * P_center.x
                    + MX_WorldToView.y1 * P_center.y
                    + MX_WorldToView.y2 * P_center.z
                    + MX_WorldToView.y3);
        // Test the top plane
        dist = viewPos.y * TopPlane.y + TopPlane.w;
        if ( dist > S_radius )
            return false;
        // Test the right plane
        dist = viewPos.y * BottomPlane.y + BottomPlane.w;
        if ( dist > S_radius )
            return false;
    }
    // Inside the field of view
    return true;
*/
}

bool FieldOfView :: CheckBox(const xPoint3 P_corners[8]) const
{
    if (Empty) return true;

    for (int ip = 0; ip < 5; ++ip)
    {
        bool culled = true;
        for (int iv = 0; iv < 8 && culled; ++iv)
            culled = Planes[ip].distanceToPoint(P_corners[iv]) > 0;
        if (culled) return false;
    }

    return true;
/*
    const xMatrix &MX_WorldToView = camera->MX_WorldToView_Get();

    xPoint3 viewPos[8];
    const xPoint3 *iterS;
    xPoint3 *iterD;
    bool culled;
    int v;

    iterS = P_corners;
    iterD = viewPos;
    for (v = 8; v; --v, ++iterS, ++iterD)
        iterD->z = -(MX_WorldToView.z0 * iterS->x
                   + MX_WorldToView.z1 * iterS->y
                   + MX_WorldToView.z2 * iterS->z
                   + MX_WorldToView.z3);
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

    if (Projection == PROJECT_PERSPECTIVE)
    {
        iterS = P_corners;
        iterD = viewPos;
        for (v = 8; v; --v, ++iterS, ++iterD)
            iterD->x = -(MX_WorldToView.x0 * iterS->x
                       + MX_WorldToView.x1 * iterS->y
                       + MX_WorldToView.x2 * iterS->z
                       + MX_WorldToView.x3);
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

        iterS = P_corners;
        iterD = viewPos;
        for (v = 8; v; --v, ++iterS, ++iterD)
            iterD->x = -(MX_WorldToView.y0 * iterS->x
                       + MX_WorldToView.y1 * iterS->y
                       + MX_WorldToView.y2 * iterS->z
                       + MX_WorldToView.y3);
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
    }
    else
    {
        iterS = P_corners;
        iterD = viewPos;
        for (v = 8; v; --v, ++iterS, ++iterD)
            iterD->x = -(MX_WorldToView.x0 * iterS->x
                       + MX_WorldToView.x1 * iterS->y
                       + MX_WorldToView.x2 * iterS->z
                       + MX_WorldToView.x3);
        // Test the left plane
        iterD = viewPos; culled = true;
        for (v = 8; v && culled; --v, ++iterD)
            culled = iterD->x * LeftPlane.x + LeftPlane.w > 0;
        if (culled) return false;
        // Test the right plane
        iterD = viewPos; culled = true;
        for (v = 8; v && culled; --v, ++iterD)
            culled = iterD->x * RightPlane.x + RightPlane.w > 0;
        if (culled) return false;

        iterS = P_corners;
        iterD = viewPos;
        for (v = 8; v; --v, ++iterS, ++iterD)
            iterD->x = -(MX_WorldToView.y0 * iterS->x
                       + MX_WorldToView.y1 * iterS->y
                       + MX_WorldToView.y2 * iterS->z
                       + MX_WorldToView.y3);
        // Test the top plane
        iterD = viewPos; culled = true;
        for (v = 8; v && culled; --v, ++iterD)
            culled = iterD->y * TopPlane.x + TopPlane.w > 0;
        if (culled) return false;
        // Test the bottom plane
        iterD = viewPos; culled = true;
        for (v = 8; v && culled; --v, ++iterD)
            culled = iterD->y * BottomPlane.x + BottomPlane.w > 0;
        if (culled) return false;
    }
    // None of the planes could cull this box
    return true;
*/
}

bool FieldOfView :: CheckPoints(const xPoint4 *P_points, xWORD I_count) const
{
    if (Empty) return true;
    if (!I_count) return false;

    for (int i = 0; i < 5; ++i)
    {
        const xPlane  &plane  = Planes[i];
        const xPoint4 *P_curr = P_points;
        bool  culled = true;
        for (int j = I_count; j && culled; --j, ++P_curr)
            culled = plane.distanceToPoint(*P_curr) > 0.f;
        if (culled) return false;
    }
    // None of the planes could cull this box
    return true;
}

bool FieldOfView :: CheckPoints(const xPoint3 *P_points, xWORD I_count) const
{
    if (Empty) return true;
    if (!I_count) return false;

    for (int i = 0; i < 5; ++i)
    {
        const xPlane  &plane  = Planes[i];
        const xPoint3 *P_curr = P_points;
        bool  culled = true;
        for (int j = I_count; j && culled; --j, ++P_curr)
            culled = plane.distanceToPoint(*P_curr) > 0.f;
        if (culled) return false;
    }
    // None of the planes could cull this box
    return true;
}
