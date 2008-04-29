#include "CameraHuman.h"

void CameraHuman::SetCamera(xFLOAT eyex, xFLOAT eyey, xFLOAT eyez,
                            xFLOAT centerx, xFLOAT centery, xFLOAT centerz,
                            xFLOAT upx, xFLOAT upy, xFLOAT upz)
{
    Camera::SetCamera(eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz);
    front.Init(centerx-eyex, centery-eyey, 0);
    stepv = step = 0.f;
}

void CameraHuman::Move(xFLOAT frwd, xFLOAT side, xFLOAT vert)
{
    // move forward/backwards
    xVector3 vect = front.normalize();
    vect *= frwd;

    eye += vect;
    center += vect;

    // move left/right
    vect.Init(front.y, -front.x, front.z);
    vect *= side;

    eye += vect;
    center += vect;

    // move up/down
    eye.z += vert;
    center.z += vert;
}

void CameraHuman::Rotate(xFLOAT heading, xFLOAT pitch, xFLOAT roll)
{
    xVector3 a = center - eye;

    if (!IsZero(heading))
    {
        /*
        heading = DegToRad (heading)/2.0F;
        xVector4 q(0.0F, 0.0F, sin(heading), cos(heading));
        a = QuaternionRotate(q, a);
        up = QuaternionRotate(q, up);
        front = QuaternionRotate(q, front);
        */
        RotatePoint(a.x, a.y, DegToRad (heading));
        RotatePoint(up.x, up.y, DegToRad (heading));
        RotatePoint(front.x, front.y, DegToRad (heading));
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
        pitch = DegToRad (pitch)/2.f;
        xFLOAT s = sin(pitch);
        front.normalize();
        xVector4 q; q.Init(front.y*s, -front.x*s, 0.f, cos(pitch));
        a = xQuaternion::rotate(q, a);
        up = xQuaternion::rotate(q, up);
    }

    if (!IsZero(roll))
    {
        // 'human' head roll
        roll = DegToRad (roll)/2.f;
        xFLOAT s = sin(roll);
        front.normalize();
        xVector4 q; q.Init(front.x*s, front.y*s, front.z*s, cos(roll));
        up = xQuaternion::rotate(q, up);
    }

    center = a + eye;
}

void CameraHuman::Orbit(xFLOAT horz, xFLOAT vert)
{
    xVector3 a = eye - center;


    if (!IsZero(horz))
    {
        RotatePoint(a.x, a.y, DegToRad (horz));
        RotatePoint(front.x, front.y, DegToRad (horz));
    }

    if (!IsZero(vert))
    {
        // Quaternions are 3x slower than my solution
        /*
        vert = DegToRad (vert)/2.0F;
        xFLOAT s = sin(vert);
        if (!IsZero(a.y))
        {
            xVector4 q(s, -a.x/a.y * s, 0, cos(vert));
            a = QuaternionRotate(q, a);
        }
        else
        if (!IsZero(a.x))
        {
            xVector4 q(0, -Sign(a.x) * s, 0, cos(vert));
            a = QuaternionRotate(q, a);
        }*/

        xFLOAT radius = sqrt ( a.x * a.x + a.y * a.y );
        xFLOAT radius2 = radius;
        xFLOAT z = a.z;
        RotatePoint(radius2, z, DegToRad(vert));
        if (radius2 > 0.f)
        {
            radius2 /= radius;
            a.x *= radius2;
            a.y *= radius2;
            a.z = z;
        }
    }

    up = OrthoPointUp(a, up);

    eye = a + center;
}

// Finds the vector orhogonal to given vector, that points "up" the most
xVector3 CameraHuman::OrthoPointUp(const xVector3 &source, const xVector3 &oldUp)
{
    if (IsZero(source.z))
        return xVector3::Create(0.f,0.f,1.f);
    if (IsZero(source.x) && IsZero(source.y))
        return xVector3::Create(oldUp.x,oldUp.y,0.f);

    xVector3 dest; dest.Init(source.x, source.y,
                             -(source.x*source.x+source.y*source.y)/source.z);

    if (dest.z*oldUp.z < 0.f)
        dest = -dest;

    return dest;
}


// rotate 2D point by given angle
void CameraHuman::RotatePoint(xFLOAT &pX, xFLOAT &pY, xFLOAT angle)
{
    xFLOAT radius = sqrt ( pX * pX + pY * pY );

    angle = atan2(pY,pX) + angle;

    pX = radius * (xFLOAT)cos (angle);
    pY = radius * (xFLOAT)sin (angle);
}

void CameraHuman::RotatePointPitch(const xVector3 front, xFLOAT &pX, xFLOAT &pY, xFLOAT &pZ, xFLOAT angle)
{
    xFLOAT angleXY = atan2(front.x,front.y);
    if (IsZero(angleXY))
        angleXY = -atan2(pY,pX);
    xFLOAT radiusXY = sqrt ( pX * pX + pY * pY );
    radiusXY *= (xFLOAT)(-Sign(pZ)*Sign(front.z));

    RotatePoint (radiusXY, pZ, angle);

    pX = radiusXY * cos (angleXY);
    pY = radiusXY * sin (angleXY);
}
