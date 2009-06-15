#ifndef Core_Math_Types
#    error Include "Math/types.h" instead of this
#else

struct Quaternion : Vector4
{
    Quaternion() {}
    Quaternion(Float32 X, Float32 Y, Float32 Z, Float32 W) : Vector4(X, Y, Z, W) {}
    Quaternion(const Vector3 &src, Float32 W)              : Vector4(src, W)     {}

    static Quaternion  & ZeroQ () { static Quaternion ret(0.f,0.f,0.f,1.f); return ret; }
           Quaternion  & zeroQ () { x = y = z = 0.f; w = 1.f;   return *this; }

    Quaternion  & init  (Float32 X, Float32 Y, Float32 Z, Float32 W) { x = X; y = Y; z = Z; w = W; return *this; }
    Quaternion  & init  (const Vector3 &src, Float32 W)              { Vector4::init(src, W);      return *this; }
    Quaternion  & initByRotation (const Point3 &srcP, const Point3 &dstP);
    Quaternion  & initByRotation (const Point3 &srcP, const Point3 &dstP, const Point3 &center)
                                 { return initByRotation(srcP-center, dstP-center); }
    Quaternion  & initAngularVel (const Vector3 &omega);

    Vector3       ToAngularVelocity() const;

    Vector3 Rotate  (const Vector3 &p) const;
    Point3  Rotate  (const Point3 &p, const Point3 &center) const
                    { return Rotate(p - center) + center; }

    Vector3 Unrotate(const Vector3 &p) const;
    Point3  Unrotate(const Point3 &p, const Point3 &center) const
                    { return Unrotate(p - center) + center; }

    static Quaternion Product         (const Quaternion &a, const Quaternion &b);

    Quaternion & interpolate     (Float32 weight);
    Quaternion & interpolateFull (Float32 weight);
    Quaternion   Interpolate     (Float32 weight) const
                                 { return Quaternion(*this).interpolate(weight); }
    Quaternion   InterpolateFull (Float32 weight) const
                                 { return Quaternion(*this).interpolateFull(weight); }

    Quaternion Exp () const;
    Quaternion Log () const;

    static Quaternion Lerp            (const Quaternion &q1,const Quaternion &q2,Float32 t);
    static Quaternion SLerp           (const Quaternion &q1,const Quaternion &q2,Float32 t);
    static Quaternion SLerpNoInvert   (const Quaternion &q1,const Quaternion &q2,Float32 t);
    static Quaternion SQuad           (const Quaternion &q1,const Quaternion &q2,const Quaternion &a,const Quaternion &b,Float32 t);
    static Quaternion SQuad           (const Quaternion &q1,const Quaternion &q2,const Quaternion &a,Float32 t);
    static Quaternion Spline          (const Quaternion &q1,const Quaternion &q2,const Quaternion &q3);
};

inline Quaternion &QuaternionCast(Vector4 &QT)
{ return *(Quaternion*)&QT; }

inline const Quaternion &QuaternionCast(const Vector4 &QT)
{ return *(Quaternion*)&QT; }

#endif

