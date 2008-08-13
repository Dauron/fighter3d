// Included in FigureCollider.cpp
// Depends on xSphere.h
// Depends on xTriangle.h

////////////////////////////// Sphere - Triangle

bool   TestSphereTriangle (const xSphere &sp1, const xPoint3 &P_A,
                           const xPoint3 &P_B, const xPoint3 &P_C)
{
    xVector3      N_tri;
    xPoint3       P_closest;
    xVector3      NW_closest;
    xBYTE         ID_edge;
    xFLOAT        S_radius_Sqr = sp1.S_radius*sp1.S_radius;
    PointPosition Position = xTriangle::PointRelation(P_A, P_B, P_C,
                                 sp1.P_center, S_radius_Sqr, N_tri, P_closest, NW_closest, ID_edge);

    if (Position == POINT_FarAway) return false;
    return true;
}

inline bool   TestSphereTriangle (const xSphere &sp1, const xTriangle &tr2)
{
    return TestSphereTriangle(sp1, tr2.P_A, tr2.P_B, tr2.P_C);
}

xDWORD CollideSphereTriangle (const xSphere &sp1, const xPoint3 &P_A,
                              const xPoint3 &P_B, const xPoint3 &P_C,
                              const xIFigure3d &figure2, xDWORD ID_tri, CollisionSet &cset)
{
    xVector3      N_tri;
    xPoint3       P_closest;
    xVector3      NW_closest;
    xBYTE         ID_edge;
    xFLOAT        S_radius_Sqr = sp1.S_radius*sp1.S_radius;
    PointPosition Position = xTriangle::PointRelation(P_A, P_B, P_C,
                                          sp1.P_center, S_radius_Sqr, N_tri, P_closest, NW_closest, ID_edge);

    if (Position == POINT_FarAway) return false;

    xFLOAT S_distance = NW_closest.lengthSqr();

    if (S_distance > EPSILON)
    {
        S_distance = sqrt(S_distance);
        NW_closest /= S_distance;
        return cset.Add(CollisionInfo(sp1, figure2, 0, ID_tri,
                                      NW_closest * (sp1.S_radius - S_distance),
                                      sp1.P_center - NW_closest * sp1.S_radius,
                                      P_closest)), 1;
    }

    if (Position == POINT_Inside)
        return cset.Add(CollisionInfo(sp1, figure2, 0, ID_tri,
                                      N_tri * sp1.S_radius,
                                      sp1.P_center - N_tri * sp1.S_radius,
                                      P_closest)), 1;

    if (Position == POINT_NearCorner)
    {
        xPoint3 P_tri_center = (P_A + P_B + P_C) * 0.3333333333333333f;
        NW_closest = (P_closest - P_tri_center).normalize();
        return cset.Add(CollisionInfo(sp1, figure2, 0, ID_tri,
                                      NW_closest * sp1.S_radius,
                                      sp1.P_center - NW_closest * sp1.S_radius,
                                      P_closest)), 1;
    }

    //if (Position == POINT_NearEdge)
    {
        if (ID_edge == 1)
            NW_closest = xVector3::CrossProduct(P_B - P_A, N_tri).normalize();
        else
        if (ID_edge == 2)
            NW_closest = xVector3::CrossProduct(P_A - P_C, N_tri).normalize();
        else
        if (ID_edge == 3)
            NW_closest = xVector3::CrossProduct(P_C - P_B, N_tri).normalize();
        return cset.Add(CollisionInfo(sp1, figure2, 0, ID_tri,
                                      NW_closest * sp1.S_radius,
                                      sp1.P_center - NW_closest * sp1.S_radius,
                                      P_closest)), 1;
    }
}

inline xDWORD CollideSphereTriangle (const xSphere &sp1, const xTriangle &tr2, CollisionSet &cset)
{
    return CollideSphereTriangle(sp1, tr2.P_A, tr2.P_B, tr2.P_C, tr2, 0, cset);
}
