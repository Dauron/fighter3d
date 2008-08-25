#include "CameraFree.h"

using namespace Math::Cameras;

void CameraFree::Move(xFLOAT frwd, xFLOAT side, xFLOAT vert)
{
    // move forward/backwards
    xVector3 NW_shift = xVector3::Normalize(P_center - P_eye) * frwd;
    P_eye    += NW_shift;
    P_center += NW_shift;

    // move left/right
    NW_shift = xVector3::Normalize(
        xVector3::CrossProduct(P_center - P_eye, NW_up) ) * side;
    P_eye    += NW_shift;
    P_center += NW_shift;

    // move up/down
    NW_shift = NW_up.normalize() * vert;
    P_eye    += NW_shift;
    P_center += NW_shift;
}

void CameraFree::Rotate(xFLOAT heading, xFLOAT pitch, xFLOAT roll)
{
    xVector3 NW_forward = P_center - P_eye;

    if (!IsZero(heading))
    {
        heading = DegToRad (heading) * 0.5f;
        xQuaternion q; q.init( NW_up.normalize()*sin(heading), cos(heading));
        NW_forward = q.rotate(NW_forward);
    }

    if (!IsZero(pitch))
    {
        pitch = DegToRad (pitch) * 0.5f;
        xVector3 N_side = xVector3::CrossProduct(NW_forward, NW_up).normalize();
        xQuaternion q; q.init(N_side*sin(pitch), cos(pitch));
        NW_forward = q.rotate(NW_forward);
        NW_up      = q.rotate(NW_up);
    }

    if (!IsZero(roll))
    {
        roll = DegToRad (roll) * 0.5f;
        xVector3 N_front = xVector3::Normalize(NW_forward);
        xQuaternion q; q.init(N_front*sin(roll), cos(roll));
        NW_up = q.rotate(NW_up);
    }

    P_center = P_eye + NW_forward;
}

void CameraFree::Orbit(xFLOAT horz, xFLOAT vert)
{
    xVector3 NW_backward = P_eye - P_center;

    if (!IsZero(horz))
    {
        horz = DegToRad (horz) * 0.5f;
        NW_up.normalize();
        xQuaternion q; q.init(NW_up*sin(horz), cos(horz));
        NW_backward = q.rotate(NW_backward);
    }

    if (!IsZero(vert))
    {
        vert = DegToRad (vert) * 0.5f;
        xVector3 N_side = xVector3::CrossProduct(NW_backward, NW_up).normalize();
        xQuaternion q; q.init(N_side * sin(vert), cos(vert));
        NW_backward = q.rotate(NW_backward);
        NW_up       = q.rotate(NW_up);
    }

    P_eye = P_center + NW_backward;
}
