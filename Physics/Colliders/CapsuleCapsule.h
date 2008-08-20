// Included in FigureCollider.cpp
// Depends on xCapsule.h

////////////////////////////// Capsule - Capsule

bool   TestCapsuleCapsule (const xCapsule &cp1, const xCapsule &cp2)
{
    xVector3 NW_top_1 = cp1.N_top * cp1.S_top;
    xVector3 NW_top_2 = cp2.N_top * cp2.S_top;
    xVector3 P_A_1 = cp1.P_center - NW_top_1;
    xVector3 P_A_2 = cp2.P_center - NW_top_2;
    xVector3 P_closest_1;
    xVector3 P_closest_2;

    xSegment::DistanceSegmentSegment(NW_top_1 * 2, P_A_1, NW_top_2 * 2, P_A_2, P_closest_1, P_closest_2);

    xVector3 NW_correction = P_closest_1 - P_closest_2;
    xFLOAT   S_distance    = NW_correction.lengthSqr();
    xFLOAT   S_radius_dist = cp1.S_radius + cp2.S_radius;

    if ( S_distance + EPSILON2 > S_radius_dist*S_radius_dist ) return false;
    return true;
}

xDWORD CollideCapsuleCapsule (const xCapsule &cp1, const xCapsule &cp2,
                              IPhysicalBody *body1, IPhysicalBody *body2, CollisionSet &cset)
{
    xVector3 NW_top_1 = cp1.N_top * cp1.S_top;
    xVector3 NW_top_2 = cp2.N_top * cp2.S_top;
    xVector3 P_A_1 = cp1.P_center - NW_top_1;
    xVector3 P_A_2 = cp2.P_center - NW_top_2;
    xVector3 P_closest_1;
    xVector3 P_closest_2;

    xSegment::DistanceSegmentSegment(NW_top_1 * 2, P_A_1, NW_top_2 * 2, P_A_2, P_closest_1, P_closest_2);

    xVector3 NW_correction = P_closest_1 - P_closest_2;
    xFLOAT   S_distance    = NW_correction.lengthSqr();
    xFLOAT   S_radius_dist = cp1.S_radius + cp2.S_radius;

    if ( S_distance + EPSILON2 > S_radius_dist*S_radius_dist ) return false;
    S_distance = sqrt(S_distance);
    NW_correction /= S_distance;
    cset.Add(CollisionInfo(body1, body2, cp1, cp2,
                           NW_correction * (S_radius_dist - S_distance),
                           P_closest_1 - NW_correction * cp1.S_radius,
                           P_closest_2 + NW_correction * cp2.S_radius));

    return 1;
}
