// Included in FigureCollider.cpp
// Depends on xCapsule.h

////////////////////////////// Capsule - Capsule

void DistanceSegmentSegmentSqr(const xVector3 &NW_axis_1, const xVector3 &P_axis_1,
                               const xVector3 &NW_axis_2, const xVector3 &P_axis_2,
                               xVector3 &P_closest_1,     xVector3 &P_closest_2)
{
    // Based on http://local.wasp.uwa.edu.au/~pbourke/geometry/lineline3d/
    //dnmpo = dot(nm, po);
    //mua = ( dN1NW12 * dN1N2 - dN2NW12 * dN1N1 ) / ( dN2N2 * dN1N1 - dN1N2 * dN1N2 ) // dN2N2 = dN1N1 = 1
    //mub = ( dN1NW12 + mua * dN1N2 ) / dN1N1
    xVector3 NW_12 = P_axis_2 - P_axis_1;

    xFLOAT dN1N1   = xVector3::DotProduct(NW_axis_1, NW_axis_1);
    xFLOAT dN1N2   = xVector3::DotProduct(NW_axis_1, NW_axis_2);
    xFLOAT dN2N2   = xVector3::DotProduct(NW_axis_2, NW_axis_2);
    xFLOAT dN1NW12 = xVector3::DotProduct(NW_axis_1, NW_12);
    xFLOAT mub = (dN1NW12 * dN1N2 - xVector3::DotProduct(NW_axis_2, NW_12) * dN1N1) / (dN1N1*dN2N2 - dN1N2*dN1N2);
    xFLOAT mua = (dN1NW12 + mub * dN1N2) / dN1N1;

    // clamp
    if (mua <= 0.f) P_closest_1 = P_axis_1;
    else
    if (mua >= 1.f) P_closest_1 = P_axis_1 + NW_axis_1;
    else            P_closest_1 = P_axis_1 + mua * NW_axis_1;

    if (mub <= 0.f) P_closest_2 = P_axis_2;
    else
    if (mub >= 1.f) P_closest_2 = P_axis_2 + NW_axis_2;
    else            P_closest_2 = P_axis_2 + mub * NW_axis_2;
}

bool          TestCapsuleCapsule (const xCapsule &cp1, const xCapsule &cp2)
{
    xVector3 NW_top_1 = cp1.N_top * cp1.S_top;
    xVector3 NW_top_2 = cp2.N_top * cp2.S_top;
    xVector3 P_A_1 = cp1.P_center - NW_top_1;
    xVector3 P_A_2 = cp2.P_center - NW_top_2;
    xVector3 P_closest_1;
    xVector3 P_closest_2;

    DistanceSegmentSegmentSqr(NW_top_1 * 2, P_A_1, NW_top_2 * 2, P_A_2, P_closest_1, P_closest_2);

    xVector3 NW_correction = P_closest_1 - P_closest_2;
    xFLOAT   S_distance    = NW_correction.lengthSqr();
    xFLOAT   S_radius_dist = cp1.S_radius + cp2.S_radius;

    if ( S_distance + EPSILON2 > S_radius_dist*S_radius_dist ) return false;
    return true;
}

CollisionInfo CollideCapsuleCapsule (const xCapsule &cp1, const xCapsule &cp2)
{
    xVector3 NW_top_1 = cp1.N_top * cp1.S_top;
    xVector3 NW_top_2 = cp2.N_top * cp2.S_top;
    xVector3 P_A_1 = cp1.P_center - NW_top_1;
    xVector3 P_A_2 = cp2.P_center - NW_top_2;
    xVector3 P_closest_1;
    xVector3 P_closest_2;

    DistanceSegmentSegmentSqr(NW_top_1 * 2, P_A_1, NW_top_2 * 2, P_A_2, P_closest_1, P_closest_2);

    xVector3 NW_correction = P_closest_1 - P_closest_2;
    xFLOAT   S_distance    = NW_correction.lengthSqr();
    xFLOAT   S_radius_dist = cp1.S_radius + cp2.S_radius;

    if ( S_distance + EPSILON2 > S_radius_dist*S_radius_dist ) return CollisionInfo(false);
    S_distance = sqrt(S_distance);
    NW_correction /= S_distance;
    return CollisionInfo(NW_correction * (S_radius_dist - S_distance),
                         P_closest_1 - NW_correction * cp1.S_radius,
                         P_closest_2 + NW_correction * cp2.S_radius);
}
