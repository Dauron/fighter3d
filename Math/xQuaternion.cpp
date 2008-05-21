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

xVector4 xQuaternion::getRotation(const xVector3 &srcP, const xVector3 &dstP, const xVector3 &center)
{
    xVector4 quat;
    xVector3 v2 = (dstP-center).normalize();
    xVector3 v1 = (srcP-center).normalize();
    xFLOAT cosang = xVector3::DotProduct(v1, v2);
    if (cosang > 1.f) cosang = 1.f;
    else
    if (cosang < -1.f) cosang = -1.f;
    xFLOAT hangle = acos(cosang) * 0.5f;
    quat.vector3 = xVector3::CrossProduct(v1, v2).normalize() * sinf(hangle);
    quat.w = cosf(hangle);
    return quat;
}

xVector3 xQuaternion::rotate(const xVector4 &q, const xVector3 &p)
{
    //return (xVector3)QuaternionProduct(QuaternionProduct(q,(xVector4)p),(QuaternionComplement(q)));
    xVector4 q_minus; q_minus.init(-q.x,-q.y,-q.z,q.w);

    xVector4 ret;
    ret.w = - q.x*p.x - q.y*p.y - q.z*p.z;
    ret.x = q.y*p.z - q.z*p.y + q.w*p.x;
    ret.y = q.z*p.x - q.x*p.z + q.w*p.y;
    ret.z = q.x*p.y - q.y*p.x + q.w*p.z;

    return product(ret, q_minus).vector3;
}

xVector3 xQuaternion::rotate(const xVector4 &q, const xVector3 &p, const xVector3 &center)
{
    xVector3 ret = p - center;
    ret = rotate(q, ret);
    return ret + center;
}

xVector3 xQuaternion::angularVelocity(const xVector4 &q)
{
    xVector3 res = q.vector3;
    res.normalize();
    res *= acos(q.w);
    return res;
}

// Quaternion product of two xVector4's
xVector4 xQuaternion::product(const xVector4 &a, const xVector4 &b)
{
  xVector4 ret;
  ret.w = a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z;
  ret.x = a.y*b.z - a.z*b.y + a.w*b.x + a.x*b.w;
  ret.y = a.z*b.x - a.x*b.z + a.w*b.y + a.y*b.w;
  ret.z = a.x*b.y - a.y*b.x + a.w*b.z + a.z*b.w;
  return ret;
}

xVector4 xQuaternion::interpolate(const xVector4 &q, xFLOAT weight)
{
    xVector4 ret = q;
    if (q.w != 1.f)
    {
        float angleH = (q.w > 0) ? acos(q.w) : PI-acos(q.w);
        float sinH   = sin(angleH);
        angleH *= weight;
        ret.vector3 *= sin(angleH)/sinH;
        ret.w = cos(angleH);
    }
    return ret;
}

xVector4 xQuaternion::interpolateFull(const xVector4 &q, xFLOAT weight)
{
    xVector4 ret = q;
    if (q.w != 1.f)
    {
        float angleH = acos(q.w);
        float sinH   = sin(angleH);
        angleH *= weight;
        ret.vector3 *= sin(angleH)/sinH;
        ret.w = cos(angleH);
    }
    return ret;
}

xVector4 xQuaternion::exp(const xVector4 &q)
{
    float a = static_cast<float>(sqrt(q.x*q.x + q.y*q.y + q.z*q.z));
    float sina = static_cast<float>(sin(a));
    float cosa = static_cast<float>(cos(a));
    xVector4 ret;
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

xVector4 xQuaternion::log(const xVector4 &q)
{
    float a    = static_cast<float>(acos(q.w));
    float sina = static_cast<float>(sin(a));
    xVector4 ret;
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
xVector4 xQuaternion::lerp(const xVector4 &q1,const xVector4 &q2,float t)
{
    return (q1 + t*(q2-q1)).normalize();
}

/*
Spherical linear interpolation between two xVector4s
*/
xVector4 xQuaternion::slerp(const xVector4 &q1,const xVector4 &q2,float t)
{
    xVector4 q3;
    float dot;
    dot = q1.x*q2.x + q1.y*q2.y + q1.z*q2.z + q1.w*q2.w;

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
        return (q1*sinaomt+q3*sinat)/sina;
    }
    /*
    if the angle is small, use linear interpolation
    */
    return lerp(q1,q3,t);
}

/*
This version of slerp, used by squad, does not check for theta > 90.
*/
xVector4 xQuaternion::slerpNoInvert(const xVector4 &q1,const xVector4 &q2,float t)
{
    float dot = q1.x*q2.x + q1.y*q2.y + q1.z*q2.z + q1.w*q2.w;

    if (dot >= -0.99f && dot <= 0.99f)
    {
        float angle = static_cast<float>(acos(dot));
        float sina,sinat,sinaomt;
        sina = static_cast<float>(sin(angle));
        sinat = static_cast<float>(sin(angle*t));
        sinaomt = static_cast<float>(sin(angle*(1-t)));
        return (q1*sinaomt+q2*sinat)/sina;
    }
    /*
    if the angle is small, use linear interpolation
    */
    return lerp(q1,q2,t);
}

/*
Spherical cubic interpolation
*/
xVector4 xQuaternion::squad(const xVector4 &q1,const xVector4 &q2,const xVector4 &a,const xVector4 &b,float t)
{
    xVector4 c,d;
    c = slerpNoInvert(q1,q2,t);
    d = slerpNoInvert(a,b,t);
    return slerpNoInvert(c,d,2*t*(1-t));
}

/*
Given 3 xVector4s, qn-1,qn and qn+1, calculate a control point to be used in spline interpolation
*/
xVector4 xQuaternion::spline(const xVector4 &qnm1,const xVector4 &qn,const xVector4 &qnp1)
{
    xVector4 qni;

    qni.x = -qn.x;
    qni.y = -qn.y;
    qni.z = -qn.z;
    qni.w = qn.w;

    return product( qn, exp((log(product(qni,qnm1))+log(product(qni,qnp1)))/-4) );
}
