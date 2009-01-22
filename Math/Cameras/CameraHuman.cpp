#include "CameraHuman.h"

using namespace Math::Cameras;

void CameraHuman::SetCamera(xFLOAT eyex, xFLOAT eyey, xFLOAT eyez,
                            xFLOAT centerx, xFLOAT centery, xFLOAT centerz,
                            xFLOAT upx, xFLOAT upy, xFLOAT upz)
{
    Camera::SetCamera(eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz);
    N_front.init(centerx-eyex, centery-eyey).normalize();
    stepv = step = 0.f;
}

void CameraHuman::Move(xFLOAT frwd, xFLOAT side, xFLOAT vert)
{
    // move forward/backwards
    xVector3 NW_shift; NW_shift.init(N_front.x * frwd, N_front.y * frwd, 0);
    P_eye    += NW_shift;
    P_center += NW_shift;

    // move left/right
    NW_shift.init(N_front.y, -N_front.x, 0) *= side;
    P_eye    += NW_shift;
    P_center += NW_shift;

    // move up/down
    P_eye.z    += vert;
    P_center.z += vert;
}

void CameraHuman::Rotate(xFLOAT heading, xFLOAT pitch, xFLOAT roll)
{
    xVector3 NW_forward = P_center - P_eye;

    if (!IsZero(heading))
    {
        /*
        heading = DegToRad (heading)/2.0F;
        xVector4 q(0.0F, 0.0F, sin(heading), cos(heading));
        a = QuaternionRotate(q, a);
        up = QuaternionRotate(q, up);
        front = QuaternionRotate(q, front);
        */
        heading = DegToRad (heading);
        RotatePoint(NW_forward.x, NW_forward.y, heading);
        RotatePoint(NW_up.x,      NW_up.y,      heading);
        RotatePoint(N_front.x,    N_front.y,    heading);
    }

    if (!IsZero(pitch))
    {
        /*
        xFLOAT radius = sqrt ( a.x * a.x + a.y * a.y );
        xFLOAT radius2 = radius;
        xFLOAT z = a.z;
        RotatePoint(radius2, z, DegToRad(pitch));
        if (radius2 > 0.0F)
        {
            radius2 /= radius;
            a.x *= radius2;
            a.y *= radius2;
            a.z = z;

            RotatePointPitch(a, up.y, up.x, up.z, DegToRad(pitch));
        }
        */
        // 'human' head pitch
        if (!( (pitch < 0 && NW_up.z < 0 && NW_forward.z < 0) ||
               (pitch > 0 && NW_up.z < 0 && NW_forward.z > 0) ) )
        {
            pitch = DegToRad (pitch) * 0.5f;
            xFLOAT s = sin(pitch);
            xQuaternion q; q.init(N_front.y*s, -N_front.x*s, 0.f, cos(pitch));

            NW_forward = q.rotate(NW_forward);
            NW_up      = q.rotate(NW_up);
        }
    }

    if (!IsZero(roll))
    {
        // 'human' head roll
        roll = DegToRad (roll) * 0.5f;
        xFLOAT s = sin(roll);
        xQuaternion q; q.init(N_front.x*s, N_front.y*s, 0, cos(roll));
        NW_up = q.rotate(NW_up);
    }

    P_center = P_eye + NW_forward;
}

void CameraHuman::Orbit(xFLOAT horz, xFLOAT vert)
{
    xVector3 NW_backward = P_eye - P_center;

    if (!IsZero(horz))
    {
        horz = DegToRad (horz);
        RotatePoint(NW_backward.x, NW_backward.y, horz);
        RotatePoint(N_front.x,     N_front.y,     horz);
    }

    if (!IsZero(vert))
    {
        // Quaternions are 3x slower than my solution
        /*
        vert = DegToRad (vert)/2.0F;
        xFLOAT s = sin(vert);
        if (!IsZero(a.y))
        {
            xQuaternion q(s, -a.x/a.y * s, 0, cos(vert));
            a = QuaternionRotate(q, a);
        }
        else
        if (!IsZero(a.x))
        {
            xQuaternion q(0, -Sign(a.x) * s, 0, cos(vert));
            a = QuaternionRotate(q, a);
        }*/

        xFLOAT radius = sqrt ( NW_backward.x * NW_backward.x + NW_backward.y * NW_backward.y );
        xFLOAT radius2 = radius;
        xFLOAT z = NW_backward.z;
        RotatePoint(radius2, z, DegToRad(vert));
        if (radius2 > 0.f)
        {
            radius2 /= radius;
            NW_backward.x *= radius2;
            NW_backward.y *= radius2;
            NW_backward.z = z;
        }
    }

    NW_up = OrthoPointUp(NW_backward, NW_up);

    P_eye = P_center + NW_backward;
}

// Finds the vector orhogonal to given vector, that points "up" the most
xVector3 CameraHuman::OrthoPointUp(const xVector3 &source, const xVector3 &oldUp)
{
    if (IsZero(source.z))
        return xVector3::Create(0.f,0.f,1.f);
    if (IsZero(source.x) && IsZero(source.y))
        return xVector3::Create(oldUp.x,oldUp.y,0.f);

    xVector3 dest; dest.init(source.x, source.y,
                             -(source.x*source.x+source.y*source.y)/source.z);

    if (dest.z*oldUp.z < 0.f)
        dest = -dest;

    return dest;
}


// rotate 2D point by given angle
void CameraHuman::RotatePoint(xFLOAT &pX, xFLOAT &pY, xFLOAT angle)
{
    if ( pX*pX < EPSILON && pY*pY < EPSILON ) {
        return;
    }
    xFLOAT radius = sqrt ( pX * pX + pY * pY );

    angle = atan2(pY,pX) + angle;

    pX = radius * (xFLOAT)cos (angle);
    pY = radius * (xFLOAT)sin (angle);
}
