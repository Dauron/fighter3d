// Included in FigureCollider.cpp
// Depends on xSphere.h
// Depends on xCapsule.h

////////////////////////////// Sphere - Capsule

bool   TestSphereCapsule (const xSphere &sp1, const xCapsule &cp2)
{
    xFLOAT S_projection = xVector3::DotProduct(sp1.P_center - cp2.P_center, cp2.N_top);
    if (S_projection >  cp2.S_top) S_projection =  cp2.S_top;
    else
    if (S_projection < -cp2.S_top) S_projection = -cp2.S_top;

    xVector3 P_closest = cp2.P_center + S_projection * cp2.N_top;
    xFLOAT   S_center_dist = (sp1.P_center - P_closest).lengthSqr();
    xFLOAT   S_radius_dist = sp1.S_radius + cp2.S_radius;

    if ( S_center_dist + EPSILON2 > S_radius_dist*S_radius_dist ) return false;
    return true;
}

xDWORD CollideSphereCapsule (const xSphere &sp1, const xCapsule &cp2,
                             IPhysicalBody *body1, IPhysicalBody *body2, CollisionSet &cset)
{
    xFLOAT S_projection = xVector3::DotProduct(sp1.P_center - cp2.P_center, cp2.N_top);
    if (S_projection >  cp2.S_top) S_projection =  cp2.S_top;
    else
    if (S_projection < -cp2.S_top) S_projection = -cp2.S_top;

    xVector3 P_closest = cp2.P_center + S_projection * cp2.N_top;
    xVector3 NW_correction = sp1.P_center - P_closest;
    xFLOAT   S_center_dist = NW_correction.lengthSqr();
    xFLOAT   S_radius_dist = sp1.S_radius + cp2.S_radius;

    if ( S_center_dist + EPSILON2 > S_radius_dist*S_radius_dist ) return false;

    S_center_dist     = sqrt(S_center_dist);
    xVector3 N_center = NW_correction / S_center_dist;
    cset.Add(CollisionInfo(body1, body2, sp1, cp2,
                           N_center * (S_radius_dist - S_center_dist),
                           sp1.P_center - N_center * sp1.S_radius,
                           P_closest    + N_center * cp2.S_radius));
    return 1;
}
