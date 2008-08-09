struct xPlane : xVector4
{
    xFLOAT distanceToPoint(const xPoint3 &P_point) const
    {
        return xVector3::DotProduct(vector3,P_point) + w;
    }
    xPlane &init(const xVector3 &N_normal, const xPoint3 &P_point)
    {
        vector3 = N_normal;
        w       = - xVector3::DotProduct( N_normal, P_point );
        return *this;
    }
    xPlane &init(const xPoint3 &P_0, const xPoint3 &P_1, const xPoint3 &P_2)
    {
        vector3 = xVector3::CrossProduct( P_1-P_0, P_2-P_0 ).normalize();
        w       = - xVector3::DotProduct( P_0, vector3 );
        return *this;
    }
    xPlane &init(const xPoint3 P_points[3])
    {
        return init(P_points[0],P_points[1],P_points[2]);
    }
    xVector3 castPoint(const xPoint3 &P_point)
    {
        return P_point - vector3 * distanceToPoint(P_point);
    }
    xVector3 intersectRay(const xPoint3 &P_ray, const xVector3 &N_ray)
    {
        const xFLOAT W_cos = xVector3::DotProduct(vector3, N_ray); // get cos between vectors
        if (W_cos != 0) // if ray is not parallel to the plane (is not perpendicular to the plane normal)
            return P_ray - N_ray * (distanceToPoint(P_ray) / W_cos);
        return xVector3::Create(xFLOAT_HUGE_NEGATIVE, xFLOAT_HUGE_NEGATIVE, xFLOAT_HUGE_NEGATIVE);
    }
    xVector3 intersectRayPositive(const xVector3 &P_ray, const xVector3 &N_ray)
    {
        const xFLOAT W_cos = xVector3::DotProduct(vector3, N_ray); // get cos between vectors
        if (W_cos < 0) // if ray is not parallel to the plane (is not perpendicular to the plane normal)
            return P_ray - N_ray * (distanceToPoint(P_ray) / W_cos);
        return xVector3::Create(xFLOAT_HUGE_NEGATIVE, xFLOAT_HUGE_NEGATIVE, xFLOAT_HUGE_NEGATIVE);
    }
};
