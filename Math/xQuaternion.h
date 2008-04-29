// Performs quaternion operations on Vector4
class xQuaternion
{
  public:
    static xVector3 rotate          (const xVector4 &q, const xVector3 &p);

    static xVector3 angularVelocity (const xVector4 &q);

    static xVector4 product         (const xVector4 &a, const xVector4 &b);
    static xVector4 interpolate     (const xVector4 &q, xFLOAT weight);
    static xVector4 interpolateFull (const xVector4 &q, xFLOAT weight);

    static xVector4 exp             (const xVector4 &q);
    static xVector4 log             (const xVector4 &q);

    static xVector4 lerp            (const xVector4 &q1,const xVector4 &q2,float t);
    static xVector4 slerp           (const xVector4 &q1,const xVector4 &q2,float t);
    static xVector4 slerpNoInvert   (const xVector4 &q1,const xVector4 &q2,float t);
    static xVector4 squad           (const xVector4 &q1,const xVector4 &q2,const xVector4 &a,const xVector4 &b,float t);
    static xVector4 squad           (const xVector4 &q1,const xVector4 &q2,const xVector4 &a,float t);
    static xVector4 spline          (const xVector4 &q1,const xVector4 &q2,const xVector4 &q3);
};
