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

#include "xMath.h"
#include <cmath>

xQuaternion xQuaternion::GetRotation(const xPoint3 &srcV, const xPoint3 &dstV)
{
    xFLOAT cosang = xPoint3::DotProduct(xPoint3::Normalize(srcV), xPoint3::Normalize(dstV));
    if (cosang < -1.f + EPSILON3)
        return xQuaternion::Create(1.f,0.f,0.f,0.f);
    if (cosang > 1.f - EPSILON3)
        return xQuaternion::Create(0.f,0.f,0.f,1.f);

    xFLOAT hangle = acos(cosang) * 0.5f;
    xQuaternion quat;
    quat.vector3 = xPoint3::CrossProduct(srcV, dstV).normalize() * sinf(hangle);
    quat.w = cosf(hangle);
    return quat;
}

xVector3 xQuaternion::rotate(const xVector3 &p) const
{
    //return (xVector3)QuaternionProduct(QuaternionProduct(q,(xVector4)p),(QuaternionComplement(q)));
    xQuaternion ret;
    ret.x =   y*p.z - z*p.y + w*p.x;
    ret.y =   z*p.x - x*p.z + w*p.y;
    ret.z =   x*p.y - y*p.x + w*p.z;
    ret.w = - x*p.x - y*p.y - z*p.z;

    return Product(ret, xQuaternion::Create(-x,-y,-z,w)).vector3;
}

xVector3 xQuaternion::unrotate(const xVector3 &p) const
{
    //return (xVector3)QuaternionProduct(QuaternionProduct((QuaternionComplement(q),(xVector4)p),q));
    xQuaternion ret;
    ret.x = - y*p.z + z*p.y + w*p.x;
    ret.y = - z*p.x + x*p.z + w*p.y;
    ret.z = - x*p.y + y*p.x + w*p.z;
    ret.w =   x*p.x + y*p.y + z*p.z;

    return Product(ret, *this).vector3;
}

xVector3 xQuaternion::angularVelocity() const
{
    xVector3 res = vector3;
    xFLOAT rw = w;
    if (rw > 1.f)  rw = 1.f;
    else
    if (rw < -1.f) rw = -1.f;
    res.normalize();
    res *= 2.f * acos(rw);
    return res;
}

xQuaternion xQuaternion::AngularVelocity(const xVector3 &omega)
{
    xFLOAT alpha = omega.length();
    if (alpha < EPSILON) return xQuaternion::Create(0.f,0.f,0.f,1.f);
    return xQuaternion::Create(omega * (sin(alpha * 0.5f) / alpha), cos(alpha * 0.5f));
}

// Quaternion product of two xVector4's
xQuaternion xQuaternion::Product(const xQuaternion &a, const xQuaternion &b)
{
  xQuaternion ret;
  ret.w = a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z;
  ret.x = a.y*b.z - a.z*b.y + a.w*b.x + a.x*b.w;
  ret.y = a.z*b.x - a.x*b.z + a.w*b.y + a.y*b.w;
  ret.z = a.x*b.y - a.y*b.x + a.w*b.z + a.z*b.w;
  return ret;
}

xQuaternion &xQuaternion::interpolate(xFLOAT weight)
{
    if (w < 1.f)
    {
        float angleH = (w > 0) ? acos(w) : PI-acos(w);
        angleH *= weight;
        vector3.normalize() *= sin(angleH);
        w = cos(angleH);
    }
    return *this;
}

xQuaternion &xQuaternion::interpolateFull(xFLOAT weight)
{
    if (w < 1.f)
    {
        float angleH = acos(w);
        angleH *= weight;
        vector3.normalize() *= sin(angleH);
        w = cos(angleH);
    }
    return *this;
}

xQuaternion xQuaternion::Exp(const xQuaternion &q)
{
    float a = static_cast<float>(sqrt(q.x*q.x + q.y*q.y + q.z*q.z));
    float sina = static_cast<float>(sin(a));
    float cosa = static_cast<float>(cos(a));
    xQuaternion ret;
    ret.w = cosa;
    if(a > 0)
    {
        ret.x = sina * q.x / a;
        ret.y = sina * q.y / a;
        ret.z = sina * q.z / a;
    }
    else
    {
        ret.x = ret.y = ret.z = 0;
    }
    return ret;
}

xQuaternion xQuaternion::Log(const xQuaternion &q)
{
    float a    = static_cast<float>(acos(q.w));
    float sina = static_cast<float>(sin(a));
    xQuaternion ret;
    ret.w = 0;
    if (sina > 0)
    {
        ret.x = a*q.x/sina;
        ret.y = a*q.y/sina;
        ret.z = a*q.z/sina;
    }
    else
    {
        ret.x=ret.y=ret.z=0;
    }
    return ret;
}

/*
Linear interpolation between two xVector4s
*/
xQuaternion xQuaternion::Lerp(const xQuaternion &q1,const xQuaternion &q2,float t)
{
    return xQuaternionCast ((q1 + t*(q2-q1)).normalize());
}

/*
Spherical linear interpolation between two xVector4s
*/
xQuaternion xQuaternion::SLerp(const xQuaternion &q1,const xQuaternion &q2,float t)
{
    xQuaternion q3;
    float dot = q1.x*q2.x + q1.y*q2.y + q1.z*q2.z + q1.w*q2.w;

    /*
    dot = cos(theta)
    if (dot < 0), q1 and q2 are more than 90 degrees apart,
    so we can invert one to reduce spinning
    */
    if (dot < 0)
    {
        dot = -dot;
        q3 = -1*q2;
    }
    else
        q3 = q2;

    if (dot <= 0.99f)
    {
        float angle = static_cast<float>(acos(dot));
        float sina,sinat,sinaomt;
        sina = static_cast<float>(sin(angle));
        sinat = static_cast<float>(sin(angle*t));
        sinaomt = static_cast<float>(sin(angle*(1-t)));
        return xQuaternionCast ((q1*sinaomt+q3*sinat)/sina);
    }
    /*
    if the angle is small, use linear interpolation
    */
    return Lerp(q1,q3,t);
}

/*
This version of slerp, used by squad, does not check for theta > 90.
*/
xQuaternion xQuaternion::SLerpNoInvert(const xQuaternion &q1,const xQuaternion &q2,float t)
{
    float dot = q1.x*q2.x + q1.y*q2.y + q1.z*q2.z + q1.w*q2.w;

    if (dot >= -0.99f && dot <= 0.99f)
    {
        float angle = static_cast<float>(acos(dot));
        float sina,sinat,sinaomt;
        sina = static_cast<float>(sin(angle));
        sinat = static_cast<float>(sin(angle*t));
        sinaomt = static_cast<float>(sin(angle*(1-t)));
        return xQuaternionCast ((q1*sinaomt+q2*sinat)/sina);
    }
    /*
    if the angle is small, use linear interpolation
    */
    return Lerp(q1,q2,t);
}

/*
Spherical cubic interpolation
*/
xQuaternion xQuaternion::SQuad(const xQuaternion &q1,const xQuaternion &q2,const xQuaternion &a,const xQuaternion &b,float t)
{
    xQuaternion c,d;
    c = SLerpNoInvert(q1,q2,t);
    d = SLerpNoInvert(a,b,t);
    return SLerpNoInvert(c,d,2*t*(1-t));
}

/*
Given 3 xVector4s, qn-1,qn and qn+1, calculate a control point to be used in spline interpolation
*/
xQuaternion xQuaternion::Spline(const xQuaternion &qnm1,const xQuaternion &qn,const xQuaternion &qnp1)
{
    xQuaternion qni;

    qni.x = -qn.x;
    qni.y = -qn.y;
    qni.z = -qn.z;
    qni.w = qn.w;

    return xQuaternionCast ( Product( qn,
                                       Exp(
                                            xQuaternionCast (
                                                             ( Log(Product(qni,qnm1)) + Log(Product(qni,qnp1)) )
                                                             / -4
                                                            )
                                          )
                                    )
                           );
}
