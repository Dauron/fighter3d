// Included in FigureCollider.cpp
// Depends on xBoxo.h
// Depends on xSphere.h

////////////////////////////// Sphere - BoxO

#define set_if_less(val,min)    ((val) < min ? (min = (val)), true : false)

bool          TestSphereBoxO (const xSphere &sp1, const xBoxO &bo2)
{
    // Intersection test
    xFLOAT   S_radius_Sqr = sp1.S_radius * sp1.S_radius,
             S_tmp, S_distance = 0.f;
    xVector3 P_center_rel = sp1.P_center - bo2.P_center;

    for (int i = 0; i < 3; ++i)
    {
        const xFLOAT &P_sphere = xVector3::DotProduct(P_center_rel, bo2.N_axis[i]);
        const xFLOAT &S_extend = bo2.S_extend[i];
        if (P_sphere > S_extend)
        {
            S_tmp       = S_extend - P_sphere;
            S_distance += S_tmp * S_tmp;
        }
        else
        if (P_sphere < -S_extend)
        {
            S_tmp       = S_extend + P_sphere;
            S_distance += S_tmp * S_tmp;
        }
        if (S_distance + EPSILON2 >= S_radius_Sqr) return false;
    }
    return true;
}

CollisionInfo CollideSphereBoxO(const xSphere &sp1, const xBoxO &bo2)
{
    // Intersection test
    xFLOAT   S_radius_Sqr = sp1.S_radius * sp1.S_radius,
             S_tmp, S_distance = 0.f;
    xVector3 P_center_rel = sp1.P_center - bo2.P_center,
             P_sphere_loc;

    for (int i = 0; i < 3; ++i)
    {
        const xFLOAT &P_sphere = P_sphere_loc.xyz[i]
                               = xVector3::DotProduct(P_center_rel, bo2.N_axis[i]);
        const xFLOAT &S_extend = bo2.S_extend[i];
        if (P_sphere > S_extend)
        {
            S_tmp       = S_extend - P_sphere;
            S_distance += S_tmp * S_tmp;
        }
        else
        if (P_sphere < -S_extend)
        {
            S_tmp       = S_extend + P_sphere;
            S_distance += S_tmp * S_tmp;
        }
        if (S_distance + EPSILON2 >= S_radius_Sqr) return CollisionInfo(false);
    }

    // Intersection correction - sphere center outside the box
    if (S_distance > 0.f)
    {
        xVector3 N_fix, N_collision; N_collision.zero();

        for (int i = 0; i < 3; ++i)
        {
            const xFLOAT &P_sphere = P_sphere_loc[i];
            const xFLOAT &S_extend = bo2.S_extend[i];
            if (P_sphere >= 0)
                N_collision += bo2.N_axis[i] * (S_extend <=  P_sphere ?  S_extend : P_sphere);
            else
                N_collision += bo2.N_axis[i] * (S_extend <= -P_sphere ? -S_extend : P_sphere);
        }

        xVector3 P_collision = bo2.P_center + N_collision;
        N_fix = sp1.P_center - P_collision;
        S_distance = N_fix.lengthSqr();
        if (S_distance + EPSILON2 >= S_radius_Sqr) return CollisionInfo(false);

        S_distance = sqrt(S_distance);
        N_fix /= S_distance;
        return CollisionInfo(N_fix * (sp1.S_radius - S_distance),
                             sp1.P_center - N_fix * sp1.S_radius,
                             P_collision);
    }
    // Intersection correction - sphere center inside the box
    else
    {
        xFLOAT   S_fix = xFLOAT_HUGE_POSITIVE;
        xVector3 N_fix;

        for (int i = 0; i < 3; ++i)
        {
            const xFLOAT &P_sphere = P_sphere_loc[i];
            const xFLOAT &S_extend = bo2.S_extend[i];
            if (P_sphere >= 0)
            {
                S_tmp = S_extend - P_sphere;
                //if (set_if_less(S_tmp, S_fix)) N_fix = bo2.N_axis[i];
                if (S_tmp < S_fix)
                {
                    S_fix = S_tmp;
                    N_fix = bo2.N_axis[i];
                }
            }
            else
            {
                S_tmp = S_extend + P_sphere;
                //if (set_if_less(S_tmp, S_fix)) N_fix = -bo2.N_axis[i];
                if (S_tmp < S_fix)
                {
                    S_fix = S_tmp;
                    N_fix = -bo2.N_axis[i];
                }
            }
        }
        return CollisionInfo(N_fix * (sp1.S_radius + S_fix),
                             sp1.P_center - N_fix * sp1.S_radius,
                             sp1.P_center + N_fix * S_fix);
    }
}
