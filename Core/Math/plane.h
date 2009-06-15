#ifndef Core_Math_Types
#    error Include "Math/types.h" instead of this
#else

struct Plane : Vector4
{
    Plane &init(const Vector3 &normal, const Point3 &point)
    {
        Vector4::init( normal,
              - Vector3::DotProduct( normal, point ) );
        return *this;
    }
    Plane &init(const Point3 &P_0, const Point3 &P_1, const Point3 &P_2)
    {
        Vector4::init( Vector3::CrossProduct( P_1-P_0, P_2-P_0 ).normalize(),
              - Vector3::DotProduct( P_0, vector3() ) );
        return *this;
    }
    Plane &init(const Point3 points[3])
    {
        return init(points[0],points[1],points[2]);
    }

    Float32 DistanceToPoint(const Point3 &point) const
    {
        return Vector3::DotProduct(vector3(),point) + w;
    }
    Float32 DistanceToPoint(const Point4 &point) const
    {
        return Vector4::DotProduct(*this,point);
    }

    Vector3 CastPoint(const Point3 &point) const
    {
        return point - vector3() * DistanceToPoint(point);
    }

    Vector3 IntersectRay(const Point3 &pntRay, const Vector3 &nrmRay) const
    {
        const Float32 fCos = Vector3::DotProduct(vector3(), nrmRay); // get cos between vectors
        if (fCos != 0) // if ray is not parallel to the plane (is not perpendicular to the plane normal)
            return pntRay - nrmRay * (DistanceToPoint(pntRay) / fCos);
        return Vector3(Float_HUGE_NEGATIVE, Float_HUGE_NEGATIVE, Float_HUGE_NEGATIVE);
    }
    Vector3 IntersectRayPositive(const Vector3 &pntRay, const Vector3 &nrmRay) const
    {
        const Float32 fCos = Vector3::DotProduct(vector3(), nrmRay); // get cos between vectors
        if (fCos < 0) // if ray is not parallel to the plane (is not perpendicular to the plane normal)
            return pntRay - nrmRay * (DistanceToPoint(pntRay) / fCos);
        return Vector3(Float_HUGE_NEGATIVE, Float_HUGE_NEGATIVE, Float_HUGE_NEGATIVE);
    }
};

#endif

