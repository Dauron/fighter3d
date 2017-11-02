#include "Camera.h"

using namespace Math::Cameras;

//const char * Camera::SCRIPT_EyeSeeAll_Center    = "EyeSeeAll_Center";
//const char * Camera::SCRIPT_EyeSeeAll_CenterTop = "EyeSeeAll_CenterTop";
//const char * Camera::SCRIPT_EyeSeeAll_Radius    = "EyeSeeAll_Radius";

void Script_EyeSeeAll_Center   (ObjectTracker &tracker, xBYTE *CameraDataPtr);
void Script_EyeSeeAll_CenterTop(ObjectTracker &tracker, xBYTE *CameraDataPtr);
void Script_EyeSeeAll_Radius   (ObjectTracker &tracker, xBYTE *CameraDataPtr);

const TrackingScript Camera::SCRIPT_EyeSeeAll_Center    = Script_EyeSeeAll_Center;
const TrackingScript Camera::SCRIPT_EyeSeeAll_CenterTop = Script_EyeSeeAll_CenterTop;
const TrackingScript Camera::SCRIPT_EyeSeeAll_Radius    = Script_EyeSeeAll_Radius;


void Camera :: SetCamera (xFLOAT eyex, xFLOAT eyey, xFLOAT eyez,
                          xFLOAT centerx, xFLOAT centery, xFLOAT centerz,
                          xFLOAT upx, xFLOAT upy, xFLOAT upz)
{
    P_eye.init   (eyex, eyey, eyez);
    P_center.init(centerx, centery, centerz);
    NW_up.init   (upx, upy, upz);
}

// Based on MESA
void Camera :: LookAtMatrix(xMatrix &MX_WorldToView)
{
    /* Forward = center - eye */
    xVector3 N_forward = (P_center - P_eye).normalize();
    /* Side = forward x up */
    xVector3 N_side = xVector3::CrossProduct(N_forward, NW_up).normalize();
    /* Recompute up as: up = side x forward */
    xVector3 N_up = xVector3::CrossProduct(N_side, N_forward);
    /* Fill matrix */
    MX_WorldToView.row0.init(N_side.x, N_up.x, -N_forward.x, 0.f);
    MX_WorldToView.row1.init(N_side.y, N_up.y, -N_forward.y, 0.f);
    MX_WorldToView.row2.init(N_side.z, N_up.z, -N_forward.z, 0.f);
    MX_WorldToView.row3.init(0.f,0.f,0.f,1.f);
    MX_WorldToView.preTranslateT(-P_eye);
}



void Camera :: Update(xFLOAT T_delta)
{
    EyeTracker.P_destination    = P_eye;
    CenterTracker.P_destination = P_center;
    EyeTracker.UpdateDestination();
    CenterTracker.UpdateDestination();

    xFLOAT W_weightEye = min(W_TrackingSpeedEye * T_delta, 1.f);
    EyeTracker.InterpolatePosition(P_eye, CenterTracker.P_destination, W_weightEye);
    xFLOAT W_weightCtr = min(W_TrackingSpeedCtr * T_delta, 1.f);
    CenterTracker.InterpolatePosition(P_center, EyeTracker.P_destination, W_weightCtr);

    xVector3 NW_front = P_center - P_eye;
    NW_up = xVector3::CrossProduct(
        xVector3::CrossProduct(NW_front, NW_up),
        NW_front).normalize();

    if (EyeTracker.Mode != ObjectTracker::TRACK_NOTHING && NW_up.z < 1.f-EPSILON)
    {
        xVector3 N_up = fabs(NW_front.z) > fabs(NW_front.x) && fabs(NW_front.z) > fabs(NW_front.y)
            ? xVector3::Create(1.f,0.f,0.f)
            : xVector3::Create(0.f,0.f,1.f);
        if (NW_up.isZero())
            NW_up = N_up;
        else
        {
            xQuaternion QT_rot = xQuaternion::GetRotation(NW_up, N_up).
                interpolate(max(W_weightEye,W_weightCtr));
            if (QT_rot.w < 0.99f)
                NW_up = QT_rot.rotate(NW_up);
            else
                NW_up = N_up;
        }
    }

    if (FOV.Projection == FieldOfView::PROJECT_ORTHOGONAL)
        FOV.InitOrthogonal(FOV.FrontClip, FOV.BackClip);

    LookAtMatrix(MX_WorldToView);
    xMatrix::Invert(MX_WorldToView, MX_ViewToWorld);

    FOV.Update();
}

void Script_EyeSeeAll_Center   (ObjectTracker &tracker, xBYTE *CameraDataPtr)
{
    if (tracker.Targets->L_objects.size() == 0) return;
    CameraTrackingData &ctd = *(CameraTrackingData*) CameraDataPtr;

    xPoint3 P_min_d, P_max_d, P_min, P_max, P_min_t, P_max_t, P_tmp;
    Vec_TrackedObject::iterator TO_curr = tracker.Targets->L_objects.begin(),
                                TO_last = tracker.Targets->L_objects.end();

    // Find Bounding Box
    P_min = P_max = (*TO_curr)->P_center_Trfm;
    xSphere::P_MinMax_Get((*TO_curr)->P_center_Trfm, (*TO_curr)->S_radius, P_min_d, P_max_d);
    for(++TO_curr; TO_curr != TO_last; ++TO_curr)
    {
        xSphere::P_MinMax_Get((*TO_curr)->P_center_Trfm, (*TO_curr)->S_radius, P_min_t, P_max_t);
        if (P_min_t.x < P_min_d.x) { P_min_d.x = P_min_t.x; }
        if (P_max_t.x > P_max_d.x) { P_max_d.x = P_max_t.x; }
        if (P_min_t.y < P_min_d.y) { P_min_d.y = P_min_t.y; }
        if (P_max_t.y > P_max_d.y) { P_max_d.y = P_max_t.y; }
        if (P_min_t.z < P_min_d.z) { P_min_d.z = P_min_t.z; }
        if (P_max_t.z > P_max_d.z) { P_max_d.z = P_max_t.z; }

        P_tmp = (*TO_curr)->P_center_Trfm;
        if (P_tmp.x < P_min.x) { P_min.x = P_tmp.x; }
        else
        if (P_tmp.x > P_max.x) { P_max.x = P_tmp.x; }
        if (P_tmp.y < P_min.y) { P_min.y = P_tmp.y; }
        else
        if (P_tmp.y > P_max.y) { P_max.y = P_tmp.y; }
        if (P_tmp.z < P_min.z) { P_min.z = P_tmp.z; }
        else
        if (P_tmp.z > P_max.z) { P_max.z = P_tmp.z; }
    }

    xPoint3  P_center = (P_min + P_max) * 0.5f;

    // Find a diagonal with the longest span covering
    xVector3 NW_diagonal[4];
    NW_diagonal[0].init(P_max.x - P_min.x, P_max.y - P_min.y, P_max.z - P_min.z);
    NW_diagonal[1].init(P_min.x - P_max.x, P_max.y - P_min.y, P_max.z - P_min.z);
    NW_diagonal[2].init(P_max.x - P_min.x, P_min.y - P_max.y, P_max.z - P_min.z);
    NW_diagonal[3].init(P_min.x - P_max.x, P_min.y - P_max.y, P_max.z - P_min.z);
    xFLOAT  S_min, S_max;
    xFLOAT4 S_span_min, S_span_max;
    S_span_min[0] = S_span_min[1] = S_span_min[2] = S_span_min[3] = xFLOAT_HUGE_POSITIVE;
    S_span_max[0] = S_span_max[1] = S_span_max[2] = S_span_max[3] = xFLOAT_HUGE_NEGATIVE;

    for(TO_curr = tracker.Targets->L_objects.begin(); TO_curr != TO_last; ++TO_curr)
    {
        for (int i = 0; i < 4; ++i)
        {
            xSphere::ComputeSpan((*TO_curr)->P_center_Trfm, (*TO_curr)->S_radius,
                NW_diagonal[i], S_min, S_max);
            if (S_min < S_span_min[i]) S_span_min[i] = S_min;
            if (S_max > S_span_max[i]) S_span_max[i] = S_max;
        }
    }

    S_max = S_span_max[0] - S_span_min[0];
    int I_max = 0;
    for (int i = 1; i < 4; ++i)
    {
        S_min = S_span_max[i] - S_span_min[i];
        if (S_min > S_max)
        {
            S_max = S_min;
            I_max = i;
        }
    }

    // Find vector to new eye
    xVector3 N_up;
    if (NW_diagonal[I_max].x == 0.f && NW_diagonal[I_max].y == 0.f)
        N_up.init(1.f, 0.f, 0.f);
    else
        N_up.init(0.f, 0.f, 1.f);
    xVector3 N_front  = xVector3::CrossProduct(N_up, NW_diagonal[I_max]).normalize();
    xPlane   PN_screen; PN_screen.init(N_front, P_center);
    if (PN_screen.distanceToPoint(*ctd.P_current) < 0.f)
    {
        N_front.invert();
        PN_screen.vector3.invert();
    }
    xVector3 N_side = xVector3::CrossProduct(N_up, N_front);

    // Find span in the screen direction
    S_span_max[0] = xFLOAT_HUGE_NEGATIVE;
    for(TO_curr = tracker.Targets->L_objects.begin(); TO_curr != TO_last; ++TO_curr)
    {
        xSphere::ComputeSpan((*TO_curr)->P_center_Trfm, (*TO_curr)->S_radius,
            N_front, S_min, S_max);
        if (S_max > S_span_max[0]) S_span_max[0] = S_max;
    }

    S_min = xVector3::DotProduct(N_front, P_center);
    xFLOAT S_front = S_span_max[0] - S_min;
    xFLOAT S_up    = fabs(xVector3::DotProduct(N_up,   P_max_d-P_min_d)) * 0.5f;
    xFLOAT S_side  = fabs(xVector3::DotProduct(N_side, NW_diagonal[I_max])) * 0.5f;

    if (ctd.camera->FOV.Projection == FieldOfView::PROJECT_PERSPECTIVE)
    {
        S_up = max(S_up, S_side/ctd.camera->FOV.Aspect) * 1.5f;
        xFLOAT S_aux = S_up / tan( DegToRad(ctd.camera->FOV.PerspAngle) *0.5f );
        tracker.P_destination = P_center + N_front * (S_front + S_aux);// + tracker.NW_destination_shift;
    }
    else
    {
        S_up = max(S_up * ctd.camera->FOV.Aspect, S_side);
        tracker.P_destination = P_center + N_front * S_up;// + tracker.NW_destination_shift;
    }
    tracker.P_destination += N_front * tracker.NW_destination_shift.y
                           + N_side  * tracker.NW_destination_shift.x
                           + N_up    * tracker.NW_destination_shift.z;
}
void Script_EyeSeeAll_Radius   (ObjectTracker &tracker, xBYTE *CameraDataPtr)
{
    if (tracker.Targets->L_objects.size() == 0) return;
    CameraTrackingData &ctd = *(CameraTrackingData*) CameraDataPtr;

    xPoint3 P_min, P_max, P_min_t, P_max_t;
    Vec_TrackedObject::iterator TO_curr = tracker.Targets->L_objects.begin(),
                                TO_last = tracker.Targets->L_objects.end();

    // Find Bounding Box
    xSphere::P_MinMax_Get((*TO_curr)->P_center_Trfm, (*TO_curr)->S_radius, P_min, P_max);
    for(++TO_curr; TO_curr != TO_last; ++TO_curr)
    {
        xSphere::P_MinMax_Get((*TO_curr)->P_center_Trfm, (*TO_curr)->S_radius, P_min_t, P_max_t);
        if (P_min_t.x < P_min.x) { P_min.x = P_min_t.x; }
        if (P_max_t.x > P_max.x) { P_max.x = P_max_t.x; }
        if (P_min_t.y < P_min.y) { P_min.y = P_min_t.y; }
        if (P_max_t.y > P_max.y) { P_max.y = P_max_t.y; }
        if (P_min_t.z < P_min.z) { P_min.z = P_min_t.z; }
        if (P_max_t.z > P_max.z) { P_max.z = P_max_t.z; }
    }

    xPoint3  P_center = (P_min + P_max) * 0.5f;

    // Find a diagonal with the longest span covering
    xVector3 NW_diagonal[4];
    NW_diagonal[0].init(P_max.x - P_min.x, P_max.y - P_min.y, P_max.z - P_min.z);
    NW_diagonal[1].init(P_min.x - P_max.x, P_max.y - P_min.y, P_max.z - P_min.z);
    NW_diagonal[2].init(P_max.x - P_min.x, P_min.y - P_max.y, P_max.z - P_min.z);
    NW_diagonal[3].init(P_min.x - P_max.x, P_min.y - P_max.y, P_max.z - P_min.z);
    xFLOAT  S_min, S_max;
    xFLOAT4 S_span_min, S_span_max;
    S_span_min[0] = S_span_min[1] = S_span_min[2] = S_span_min[3] = xFLOAT_HUGE_POSITIVE;
    S_span_max[0] = S_span_max[1] = S_span_max[2] = S_span_max[3] = xFLOAT_HUGE_NEGATIVE;

    for(TO_curr = tracker.Targets->L_objects.begin(); TO_curr != TO_last; ++TO_curr)
    {
        for (int i = 0; i < 4; ++i)
        {
            xSphere::ComputeSpan((*TO_curr)->P_center_Trfm, (*TO_curr)->S_radius,
                NW_diagonal[i], S_min, S_max);
            if (S_min < S_span_min[i]) S_span_min[i] = S_min;
            if (S_max > S_span_max[i]) S_span_max[i] = S_max;
        }
    }

    S_max = S_span_max[0] - S_span_min[0];
    int I_max = 0;
    for (int i = 1; i < 4; ++i)
    {
        S_min = S_span_max[i] - S_span_min[i];
        if (S_min > S_max)
        {
            S_max = S_min;
            I_max = i;
        }
    }

    // Find vector to new eye
    xVector3 N_up;
    if (NW_diagonal[I_max].x == 0.f && NW_diagonal[I_max].y == 0.f)
        N_up.init(1.f, 0.f, 0.f);
    else
        N_up.init(0.f, 0.f, 1.f);
    xVector3 N_front  = xVector3::CrossProduct(N_up, NW_diagonal[I_max]).normalize();
    xPlane   PN_screen; PN_screen.init(N_front, P_center);
    if (PN_screen.distanceToPoint(*ctd.P_current) < 0.f)
    {
        N_front.invert();
        PN_screen.vector3.invert();
    }
    xVector3 N_side = xVector3::CrossProduct(N_up, N_front);

    // Find span in the screen direction
    S_span_max[0] = xFLOAT_HUGE_NEGATIVE;
    for(TO_curr = tracker.Targets->L_objects.begin(); TO_curr != TO_last; ++TO_curr)
    {
        xSphere::ComputeSpan((*TO_curr)->P_center_Trfm, (*TO_curr)->S_radius,
            N_front, S_min, S_max);
        if (S_max > S_span_max[0]) S_span_max[0] = S_max;
    }

    S_min = xVector3::DotProduct(N_front, P_center);
    xFLOAT S_front = S_span_max[0] - S_min;
    xFLOAT S_up    = fabs(xVector3::DotProduct(N_up,   NW_diagonal[I_max])) * 0.5f;
    xFLOAT S_side  = fabs(xVector3::DotProduct(N_side, NW_diagonal[I_max])) * 0.5f;

    if (ctd.camera->FOV.Projection == FieldOfView::PROJECT_PERSPECTIVE)
    {
        S_up = max(S_up, S_side/ctd.camera->FOV.Aspect) * 1.5f;
        xFLOAT S_aux = S_up / tan( DegToRad(ctd.camera->FOV.PerspAngle) *0.5f );
        tracker.P_destination = P_center + N_front * (S_front + S_aux);// + tracker.NW_destination_shift;
    }
    else
    {
        S_up = max(S_up * ctd.camera->FOV.Aspect, S_side);
        tracker.P_destination = P_center + N_front * S_up;// + tracker.NW_destination_shift;
    }
    tracker.P_destination += N_front * tracker.NW_destination_shift.y
                           + N_side  * tracker.NW_destination_shift.x
                           + N_up    * tracker.NW_destination_shift.z;

}
void Script_EyeSeeAll_CenterTop(ObjectTracker &tracker, xBYTE *CameraDataPtr)
{
    if (tracker.Targets->L_objects.size() == 0) return;
    CameraTrackingData &ctd = *(CameraTrackingData*) CameraDataPtr;

    xPoint3 P_min_d, P_max_d, P_min, P_max, P_min_t, P_max_t, P_tmp;
    Vec_TrackedObject::iterator TO_curr = tracker.Targets->L_objects.begin(),
                                TO_last = tracker.Targets->L_objects.end();

    // Find Bounding Box
    P_min = P_max = (*TO_curr)->P_center_Trfm;
    xSphere::P_MinMax_Get((*TO_curr)->P_center_Trfm, (*TO_curr)->S_radius, P_min_d, P_max_d);
    for(++TO_curr; TO_curr != TO_last; ++TO_curr)
    {
        xSphere::P_MinMax_Get((*TO_curr)->P_center_Trfm, (*TO_curr)->S_radius, P_min_t, P_max_t);
        if (P_min_t.x < P_min_d.x) { P_min_d.x = P_min_t.x; }
        if (P_max_t.x > P_max_d.x) { P_max_d.x = P_max_t.x; }
        if (P_min_t.y < P_min_d.y) { P_min_d.y = P_min_t.y; }
        if (P_max_t.y > P_max_d.y) { P_max_d.y = P_max_t.y; }
        if (P_min_t.z < P_min_d.z) { P_min_d.z = P_min_t.z; }
        if (P_max_t.z > P_max_d.z) { P_max_d.z = P_max_t.z; }

        P_tmp = (*TO_curr)->P_center_Trfm;
        if (P_tmp.x < P_min.x) { P_min.x = P_tmp.x; }
        else
        if (P_tmp.x > P_max.x) { P_max.x = P_tmp.x; }
        if (P_tmp.y < P_min.y) { P_min.y = P_tmp.y; }
        else
        if (P_tmp.y > P_max.y) { P_max.y = P_tmp.y; }
        if (P_tmp.z < P_min.z) { P_min.z = P_tmp.z; }
        else
        if (P_tmp.z > P_max.z) { P_max.z = P_tmp.z; }
    }

    xPoint3  P_center = (P_min + P_max) * 0.5f;

    // Find a diagonal with the longest span covering
    xVector3 NW_diagonal[4];
    NW_diagonal[0].init(P_max.x - P_min.x, P_max.y - P_min.y, P_max.z - P_min.z);
    NW_diagonal[1].init(P_min.x - P_max.x, P_max.y - P_min.y, P_max.z - P_min.z);
    NW_diagonal[2].init(P_max.x - P_min.x, P_min.y - P_max.y, P_max.z - P_min.z);
    NW_diagonal[3].init(P_min.x - P_max.x, P_min.y - P_max.y, P_max.z - P_min.z);
    xFLOAT  S_min, S_max;
    xFLOAT4 S_span_min, S_span_max;
    S_span_min[0] = S_span_min[1] = S_span_min[2] = S_span_min[3] = xFLOAT_HUGE_POSITIVE;
    S_span_max[0] = S_span_max[1] = S_span_max[2] = S_span_max[3] = xFLOAT_HUGE_NEGATIVE;

    for(TO_curr = tracker.Targets->L_objects.begin(); TO_curr != TO_last; ++TO_curr)
    {
        for (int i = 0; i < 4; ++i)
        {
            xSphere::ComputeSpan((*TO_curr)->P_center_Trfm, (*TO_curr)->S_radius,
                NW_diagonal[i], S_min, S_max);
            if (S_min < S_span_min[i]) S_span_min[i] = S_min;
            if (S_max > S_span_max[i]) S_span_max[i] = S_max;
        }
    }

    S_max = S_span_max[0] - S_span_min[0];
    int I_max = 0;
    for (int i = 1; i < 4; ++i)
    {
        S_min = S_span_max[i] - S_span_min[i];
        if (S_min > S_max)
        {
            S_max = S_min;
            I_max = i;
        }
    }

    // Find vector to new eye
    xVector3 N_up, N_front;
    if (NW_diagonal[I_max].x == 0.f && NW_diagonal[I_max].y == 0.f)
    {
        N_up.init(1.f, 0.f, 0.f);
        N_front.init(0.f, 1.f, 0.f);
    }
    else
    {
        N_up.init(-NW_diagonal[I_max].y, NW_diagonal[I_max].x, 0.f).normalize();
        N_front.init(0.f, 0.f, 1.f);
    }
    xVector3 N_side = xVector3::CrossProduct(N_up, N_front);

    // Find span in the screen direction
    S_span_max[0] = xFLOAT_HUGE_NEGATIVE;
    for(TO_curr = tracker.Targets->L_objects.begin(); TO_curr != TO_last; ++TO_curr)
    {
        xSphere::ComputeSpan((*TO_curr)->P_center_Trfm, (*TO_curr)->S_radius,
            N_front, S_min, S_max);
        if (S_max > S_span_max[0]) S_span_max[0] = S_max;
    }

    S_min = xVector3::DotProduct(N_front, P_center);
    xFLOAT S_front = S_span_max[0] - S_min;
    xFLOAT S_up    = fabs(xVector3::DotProduct(N_up,   P_max_d-P_min_d)) * 0.5f;
    xFLOAT S_side  = fabs(xVector3::DotProduct(N_side, NW_diagonal[I_max])) * 0.5f;

    if (ctd.camera->FOV.Projection == FieldOfView::PROJECT_PERSPECTIVE)
    {
        S_up = max(S_up, S_side/ctd.camera->FOV.Aspect) * (1.5f + 3.f);
        xFLOAT S_aux = S_up / tan( DegToRad(ctd.camera->FOV.PerspAngle) *0.5f );
        tracker.P_destination = P_center + N_front * (S_front + S_aux);// + tracker.NW_destination_shift;
    }
    else
    {
        S_up = max(S_up * ctd.camera->FOV.Aspect, S_side) * (0.5f + 3.f);
        tracker.P_destination = P_center + N_front * S_up;// + tracker.NW_destination_shift;
    }

    tracker.P_destination += N_front * tracker.NW_destination_shift.y
                           + N_side  * tracker.NW_destination_shift.x
                           + N_up    * tracker.NW_destination_shift.z;
}
