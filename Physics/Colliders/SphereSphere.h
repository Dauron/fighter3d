// Included in FigureCollider.cpp
// Depends on xSphere.h

////////////////////////////// Sphere - Sphere

bool   TestSphereSphere (const xSphere &sp1, const xSphere &sp2)
{
    xFLOAT   S_radius_dist = sp1.S_radius + sp2.S_radius;
    xFLOAT   S_center_dist = (sp1.P_center - sp2.P_center).lengthSqr();

    if ( S_center_dist + EPSILON2 > S_radius_dist*S_radius_dist ) return false;
    return true;
}

xDWORD CollideSphereSphere (const xSphere &sp1, const xSphere &sp2,
                            IPhysicalBody *body1, IPhysicalBody *body2, CollisionSet &cset)
{
    xVector3 NW_center     = sp1.P_center - sp2.P_center;
    xFLOAT   S_radius_dist = sp1.S_radius + sp2.S_radius;
    xFLOAT   S_center_dist = NW_center.lengthSqr();

    if ( S_center_dist + EPSILON2 > S_radius_dist*S_radius_dist ) return false;

    S_center_dist     = sqrt(S_center_dist);
    xVector3 N_center = NW_center / S_center_dist;
    cset.Add(CollisionInfo(body1, body2, sp1, sp2,
                           N_center * (S_radius_dist - S_center_dist),
                           sp1.P_center - N_center * sp1.S_radius,
                           sp2.P_center + N_center * sp2.S_radius));
    return 1;
}
