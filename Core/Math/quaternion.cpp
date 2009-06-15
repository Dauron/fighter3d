/* Based on: */
/* Copyright (C) Jason Shankel, 2000.
 * All rights reserved worldwide.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code, for example:
 * "Portions Copyright (C) Jason Shankel, 2000"
 */
/*
Interpolating Quaternions
Jason Shankel
*/

#include "types.h"

using namespace Math;

Quaternion &Quaternion::initByRotation(const Point3 &srcV, const Point3 &dstV)
{
    Float32 cosang = Point3::DotProduct(srcV.Normalize(), dstV.Normalize());
    if (cosang < -1.f + Float_EPSILON_RADS)
        return init(1.f,0.f,0.f,0.f);
    if (cosang >  1.f - Float_EPSILON_RADS)
        return init(0.f,0.f,0.f,1.f);

    Float32 hangle = acos(cosang) * 0.5f;

    init( Point3::CrossProduct(srcV, dstV).normalize() * sinf(hangle), cosf(hangle) );
    return *this;
}

Vector3 Quaternion::Rotate(const Vector3 &p) const
{
    //return (Vector3)QuaternionProduct(QuaternionProduct(q,(Vector4)p),(QuaternionComplement(q)));
    Quaternion ret;
    ret.x =   y*p.z - z*p.y + w*p.x;
    ret.y =   z*p.x - x*p.z + w*p.y;
    ret.z =   x*p.y - y*p.x + w*p.z;
    ret.w = - x*p.x - y*p.y - z*p.z;

    return Product(ret, Quaternion(-x,-y,-z,w)).vector3();
}

Vector3 Quaternion::Unrotate(const Vector3 &p) const
{
    //return (Vector3)QuaternionProduct(QuaternionProduct((QuaternionComplement(q),(Vector4)p),q));
    Quaternion ret;
    ret.x = - y*p.z + z*p.y + w*p.x;
    ret.y = - z*p.x + x*p.z + w*p.y;
    ret.z = - x*p.y + y*p.x + w*p.z;
    ret.w =   x*p.x + y*p.y + z*p.z;

    return Product(ret, *this).vector3();
}

Vector3 Quaternion::ToAngularVelocity() const
{
    Float32 rw  = w;
    if (rw > 1.f)  rw = 1.f;
    else
    if (rw < -1.f) rw = -1.f;
    return vector3().Normalize() * 2.f * acos(rw);
}

Quaternion &Quaternion::initAngularVel(const Vector3 &omega)
{
    Float32 alpha = omega.Length();
    if (alpha < Float_EPSILON) return zeroQ();
    return init(omega * (sin(alpha * 0.5f) / alpha), cos(alpha * 0.5f));
}

// Quaternion product of two Vector4's
Quaternion Quaternion::Product(const Quaternion &a, const Quaternion &b)
{
  Quaternion ret;
  ret.w = a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z;
  ret.x = a.y*b.z - a.z*b.y + a.w*b.x + a.x*b.w;
  ret.y = a.z*b.x - a.x*b.z + a.w*b.y + a.y*b.w;
  ret.z = a.x*b.y - a.y*b.x + a.w*b.z + a.z*b.w;
  return ret;
}

Quaternion &Quaternion::interpolate(Float32 weight)
{
    if (w < 1.f)
    {
        Float32 angleH = (w > 0) ? acos(w) : Pi-acos(w);
        angleH *= weight;
        vector3().normalize() *= sin(angleH);
        w = cos(angleH);
    }
    return *this;
}

Quaternion &Quaternion::interpolateFull(Float32 weight)
{
    if (w < 1.f)
    {
        Float32 angleH = acos(w);
        angleH *= weight;
        vector3().normalize() *= sin(angleH);
        w = cos(angleH);
    }
    return *this;
}

Quaternion Quaternion::Exp() const
{
    Float32 a = vector3().LengthSqr();
    if(a > Float_EPSILON)
    {
        a = sqrt(a);
        Float32 sina = static_cast<Float32>(sin(a) / a);
        Float32 cosa = static_cast<Float32>(cos(a));

        return Quaternion( sina * vector3(), cosa );
    }
    else
        return Quaternion( 0.f, 0.f, 0.f, 1.f );
}

Quaternion Quaternion::Log() const
{
    Float32 a    = static_cast<Float32>(acos(w));
    Float32 sina = static_cast<Float32>(sin(a));
    if (sina > Float_EPSILON)
        return Quaternion( (a/sina) * vector3(), 0.f );
    else
        return Quaternion( 0.f, 0.f, 0.f, 0.f );
}

/*
Linear interpolation between two Vector4s
*/
Quaternion Quaternion::Lerp(const Quaternion &q1,const Quaternion &q2,Float32 t)
{
    return QuaternionCast ((q1 + t*(q2-q1)).normalize());
}

/*
Spherical linear interpolation between two Vector4s
*/
Quaternion Quaternion::SLerp(const Quaternion &q1,const Quaternion &q2,Float32 t)
{
    Quaternion q3;
    Float32 dot = q1.x*q2.x + q1.y*q2.y + q1.z*q2.z + q1.w*q2.w;

    /*
    dot = cos(theta)
    if (dot < 0), q1 and q2 are more than 90 degrees apart,
    so we can invert one to reduce spinning
    */
    if (dot < 0)
    {
        dot = -dot;
        q3 = QuaternionCast (-1*q2);
    }
    else
        q3 = q2;

    if (dot <= 0.99f)
    {
        Float32 angle = static_cast<Float32>(acos(dot));
        Float32 sina,sinat,sinaomt;
        sina = static_cast<Float32>(sin(angle));
        sinat = static_cast<Float32>(sin(angle*t));
        sinaomt = static_cast<Float32>(sin(angle*(1-t)));
        return QuaternionCast ((q1*sinaomt+q3*sinat)/sina);
    }
    /*
    if the angle is small, use linear interpolation
    */
    return Lerp(q1,q3,t);
}

/*
This version of slerp, used by squad, does not check for theta > 90.
*/
Quaternion Quaternion::SLerpNoInvert(const Quaternion &q1,const Quaternion &q2,Float32 t)
{
    Float32 dot = q1.x*q2.x + q1.y*q2.y + q1.z*q2.z + q1.w*q2.w;

    if (dot >= -0.99f && dot <= 0.99f)
    {
        Float32 angle = static_cast<Float32>(acos(dot));
        Float32 sina,sinat,sinaomt;
        sina = static_cast<Float32>(sin(angle));
        sinat = static_cast<Float32>(sin(angle*t));
        sinaomt = static_cast<Float32>(sin(angle*(1-t)));
        return QuaternionCast ((q1*sinaomt+q2*sinat)/sina);
    }
    /*
    if the angle is small, use linear interpolation
    */
    return Lerp(q1,q2,t);
}

/*
Spherical cubic interpolation
*/
Quaternion Quaternion::SQuad(const Quaternion &q1,const Quaternion &q2,const Quaternion &a,const Quaternion &b,Float32 t)
{
    Quaternion c,d;
    c = SLerpNoInvert(q1,q2,t);
    d = SLerpNoInvert(a,b,t);
    return SLerpNoInvert(c,d,2*t*(1-t));
}

/*
Given 3 Vector4s, qn-1,qn and qn+1, calculate a control point to be used in spline interpolation
*/
Quaternion Quaternion::Spline(const Quaternion &qnm1,const Quaternion &qn,const Quaternion &qnp1)
{
    Quaternion qni;

    qni.x = -qn.x;
    qni.y = -qn.y;
    qni.z = -qn.z;
    qni.w = qn.w;

    return QuaternionCast ( Product( qn,
                                            QuaternionCast (
                                                             ( Product(qni,qnm1).Log() + Product(qni,qnp1).Log() )
                                                             / -4
                                                           ).Exp()
                                   )
                          );
}
