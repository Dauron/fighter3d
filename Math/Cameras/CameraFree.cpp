#include "CameraFree.h"

void CameraFree::Move(xFLOAT frwd, xFLOAT side, xFLOAT vert)
{
    // move forward/backwards
    xVector3 vect = center - eye;
    vect.normalize();
    vect *= frwd;

    eye += vect;
    center += vect;

    // move left/right
    vect = xVector3::CrossProduct (center - eye, up);
    vect.normalize();
    vect *= side;

    eye += vect;
    center += vect;

    // move up/down
    vect = up;
    vect.normalize();
    vect *= vert;

    eye += vect;
    center += vect;
}

void CameraFree::Rotate(xFLOAT heading, xFLOAT pitch, xFLOAT roll)
{
    xVector3 a = center - eye;

    if (!IsZero(heading))
    {
        heading = DegToRad (heading)/2.F;
        xFLOAT s = sin(heading);
        up.normalize();
        xVector4 q; q.Init(up.x*s, up.y*s, up.z*s, cos(heading));
        a = xQuaternion::rotate(q, a);
    }

    if (!IsZero(pitch))
    {
        pitch = DegToRad (pitch)/2.f;
        xFLOAT s = sin(pitch);

        xVector3 cross = xVector3::CrossProduct(a, up);
        cross.normalize();

        xVector4 q; q.Init(cross.x*s, cross.y*s, cross.z*s, cos(pitch));
        a = xQuaternion::rotate(q, a);
        up = xQuaternion::rotate(q, up);
    }

    if (!IsZero(roll))
    {
        roll = DegToRad (roll)/2.f;
        xFLOAT s = sin(roll);

        xVector3 front = a;
        front.normalize();

        xVector4 q; q.Init(front.x*s, front.y*s, front.z*s, cos(roll));
        up = xQuaternion::rotate(q, up);
    }

    center = a + eye;
}

void CameraFree::Orbit(xFLOAT horz, xFLOAT vert)
{
    xVector3 a = eye - center;

    if (!IsZero(horz))
    {
        horz = DegToRad (horz)/2.f;
        xFLOAT s = sin(horz);
        up.normalize();
        xVector4 q; q.Init(up.x*s, up.y*s, up.z*s, cos(horz));
        a = xQuaternion::rotate(q, a);
    }

    if (!IsZero(vert))
    {
        vert = DegToRad (vert)/2.f;
        xFLOAT s = sin(vert);

        xVector3 cross = xVector3::CrossProduct(a, up);
        cross.normalize();

        xVector4 q; q.Init(cross.x*s, cross.y*s, cross.z*s, cos(vert));
        a = xQuaternion::rotate(q, a);
        up = xQuaternion::rotate(q, up);
    }

    eye = a + center;
}
