class xQuaternion : public xVector4
{
public:
    static xQuaternion Create(xFLOAT X, xFLOAT Y, xFLOAT Z, xFLOAT W) { xQuaternion res; return res.init(X,Y,Z,W); }
    static xQuaternion Create(const xVector3 &src, xFLOAT W)          { xQuaternion res; return res.init(src,W); }
    static xQuaternion Create(const xVector3 &src)                    { xQuaternion res; return res.init(src); }
    xQuaternion       &init  (xFLOAT X, xFLOAT Y, xFLOAT Z, xFLOAT W) { x = X; y = Y; z = Z; w = W; return *this; }
    xQuaternion       &init  (const xVector3 &src, xFLOAT W)          { vector3 = src; w = W;       return *this; }
    xQuaternion       &init  (const xVector3 &src)                    { vector3 = src; w = 0.f;     return *this; }
    xQuaternion       &init  (const xFLOAT   *src)                    { memcpy(this, src, 4*sizeof(xFLOAT)); return *this; }
    xQuaternion       &zeroQ ()                                       { x = y = z = 0.f; w = 1.f;   return *this; }

    xQuaternion &operator = (const xVector4 &source) {
        memcpy(this, &source, sizeof(xVector4));
        return *this;
    }

    static xQuaternion getRotation     (const xPoint3 &srcP, const xPoint3 &dstP);

    static xQuaternion getRotation     (const xPoint3 &srcP, const xPoint3 &dstP, const xPoint3 &center)
    {
        return getRotation(srcP-center, dstP-center);
    }

    static xPoint3 rotate              (const xQuaternion &q, const xPoint3 &p);
    static xPoint3 rotate              (const xQuaternion &q, const xPoint3 &p, const xPoint3 &center);

    xVector3    angularVelocity();
    static xQuaternion angularVelocity(const xVector3 &omega);

    static xQuaternion product         (const xQuaternion &a, const xQuaternion &b);
    static xQuaternion interpolate     (const xQuaternion &q, xFLOAT weight);
    static xQuaternion interpolateFull (const xQuaternion &q, xFLOAT weight);

    static xQuaternion exp             (const xQuaternion &q);
    static xQuaternion log             (const xQuaternion &q);

    static xQuaternion lerp            (const xQuaternion &q1,const xQuaternion &q2,float t);
    static xQuaternion slerp           (const xQuaternion &q1,const xQuaternion &q2,float t);
    static xQuaternion slerpNoInvert   (const xQuaternion &q1,const xQuaternion &q2,float t);
    static xQuaternion squad           (const xQuaternion &q1,const xQuaternion &q2,const xQuaternion &a,const xQuaternion &b,float t);
    static xQuaternion squad           (const xQuaternion &q1,const xQuaternion &q2,const xQuaternion &a,float t);
    static xQuaternion spline          (const xQuaternion &q1,const xQuaternion &q2,const xQuaternion &q3);
};

inline xQuaternion &xQuaternionCast(xVector4 &QT)
{ return *(xQuaternion*)&QT; }

inline const xQuaternion &xQuaternionCast(const xVector4 &QT)
{ return *(xQuaternion*)&QT; }