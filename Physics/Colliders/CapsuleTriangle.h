// Included in FigureCollider.cpp
// Depends on xCapsule.h
// Depends on xTriangle.h

////////////////////////////// Capsule - Triangle

bool TestCapsuleTriangle (const xCapsule &cp1, const xPoint3 &P_A,
                          const xPoint3 &P_B, const xPoint3 &P_C)
{
    AxisSpans spans[16];
    if (AxisSpans::AxisNotOverlap(cp1, P_A, P_B, P_C, spans[0].init(cp1.N_top), 0 ) )
        return false;

    xVector3 NW_AB = P_B - P_A;
    xVector3 NW_BC = P_C - P_B;
    xVector3 N_tri = xVector3::CrossProduct(NW_AB, NW_BC).normalize();

    if (AxisSpans::AxisNotOverlap(P_A, P_B, P_C, cp1, spans[1].init(N_tri), 0) ||
        AxisSpans::AxisNotOverlap(cp1, P_A, P_B, P_C, spans[2].init(NW_AB)) ||
        AxisSpans::AxisNotOverlap(cp1, P_A, P_B, P_C, spans[3].init(NW_BC)) ||
        AxisSpans::AxisNotOverlap(cp1, P_A, P_B, P_C, spans[4].init(xVector3::CrossProduct(N_tri, cp1.N_top)) ) ||
        AxisSpans::AxisNotOverlap(cp1, P_A, P_B, P_C, spans[5].init(xVector3::CrossProduct(N_tri, spans[2].N_axis)) ) ||
        AxisSpans::AxisNotOverlap(cp1, P_A, P_B, P_C, spans[6].init(xVector3::CrossProduct(N_tri, spans[3].N_axis)) ) ||
        AxisSpans::AxisNotOverlap(cp1, P_A, P_B, P_C, spans[7].init(xVector3::CrossProduct(cp1.N_top, spans[2].N_axis)) ) ||
        AxisSpans::AxisNotOverlap(cp1, P_A, P_B, P_C, spans[8].init(xVector3::CrossProduct(cp1.N_top, spans[3].N_axis)) ) ||
        AxisSpans::AxisNotOverlap(cp1, P_A, P_B, P_C, spans[9].init(xVector3::CrossProduct(xVector3::CrossProduct(cp1.P_center-P_A, cp1.N_top), cp1.N_top) )  ) ||
        AxisSpans::AxisNotOverlap(cp1, P_A, P_B, P_C, spans[10].init(xVector3::CrossProduct(xVector3::CrossProduct(cp1.P_center-P_B, cp1.N_top), cp1.N_top) )  ) ||
        AxisSpans::AxisNotOverlap(cp1, P_A, P_B, P_C, spans[11].init(xVector3::CrossProduct(xVector3::CrossProduct(cp1.P_center-P_C, cp1.N_top), cp1.N_top) )  ) )
        return false;

    xVector3 NW_CA = P_A - P_C;

    if (AxisSpans::AxisNotOverlap(cp1, P_A, P_B, P_C, spans[12].init(NW_CA)) ||
        AxisSpans::AxisNotOverlap(cp1, P_A, P_B, P_C, spans[13].init(xVector3::CrossProduct(N_tri, spans[12].N_axis)) ) ||
        AxisSpans::AxisNotOverlap(cp1, P_A, P_B, P_C, spans[14].init(xVector3::CrossProduct(cp1.N_top, spans[12].N_axis)) ) )
        return false;

    xPoint3       P_closest;
    xVector3      NW_closest;
    xBYTE         ID_edge;

    spans[15].wrong = true;
    xVector3 P_capCenter = cp1.P_center + cp1.N_top * cp1.S_top;
    if (xTriangle::PointRelation(P_A, P_B, P_C, P_capCenter, xFLOAT_HUGE_POSITIVE, N_tri, P_closest, NW_closest, ID_edge) >= POINT_NearEdge &&
        xVector3::DotProduct(NW_closest, cp1.N_top) < 0)
        spans[15].init(P_closest - P_capCenter);
    else
    {
        P_capCenter = cp1.P_center - cp1.N_top * cp1.S_top;
        if (xTriangle::PointRelation(P_A, P_B, P_C, P_capCenter, xFLOAT_HUGE_POSITIVE, N_tri, P_closest, NW_closest, ID_edge) >= POINT_NearEdge &&
            xVector3::DotProduct(NW_closest, cp1.N_top) > 0)
            spans[15].init(P_closest - P_capCenter);
    }
    if (AxisSpans::AxisNotOverlap(cp1, P_A, P_B, P_C, spans[15]))
        return false;

    return true;
}

bool TestCapsuleTriangle (const xCapsule &cp1, const xTriangle &tr2)
{
    return TestCapsuleTriangle (cp1, tr2.P_A, tr2.P_B, tr2.P_C);
}

void HitPointsCapsuleTriangle(const xCapsule &cp2, const xPoint3 P_closest[3],
                              xPoint3         P_hits[4], xBYTE  &I_hits)
{
    int I_touch = 0;
    xPoint3 P_min, P_max;

    for (int I_A = 0; I_A < 2; ++I_A)
    {
        int I_B = (I_A + 1 % 3);
        if (SegmentCapsuleMinMax(P_closest[I_A], P_closest[I_B], cp2, P_min, P_max))
        {
            ++I_touch;
            bool FL_found = false;
            for (int i = 0; i < I_hits; ++i)
                if (P_min == P_hits[i]) { FL_found = true; break; }
            if (!FL_found)
                P_hits[I_hits++] = P_min;
            FL_found = false;
            for (int i = 0; i < I_hits; ++i)
                if (P_max == P_hits[i]) { FL_found = true; break; }
            if (!FL_found)
                P_hits[I_hits++] = P_max;
        }
    }
    /*
    if (I_touch) return;

    I_max = PPosition == POINT_NearEdge ? 2 : 4;
    for (int I_A = 0; I_A < I_max; ++I_A)
    {
        bool FL_found = false;
        for (int i = 0; i < I_hits; ++i)
            if (P_closest[I_A] == P_hits[i]) { FL_found = true; break; }
        if (!FL_found)
            P_hits[I_hits++] = P_closest[I_A];
    }
    */
}

xDWORD CollideCapsuleTriangle (const xCapsule &cp1, const xPoint3 &P_A,
                               const xPoint3 &P_B, const xPoint3 &P_C,
                               const xIFigure3d &figure2, xDWORD ID_tri, CollisionSet &cset)
{
    AxisSpans spans[16];
    if (AxisSpans::AxisNotOverlap(cp1, P_A, P_B, P_C, spans[0].init(cp1.N_top), 0 ) )
        return false;

    xVector3 NW_AB = P_B - P_A;
    xVector3 NW_BC = P_C - P_B;
    xVector3 N_tri = xVector3::CrossProduct(NW_AB, NW_BC).normalize();

    if (AxisSpans::AxisNotOverlap(P_A, P_B, P_C, cp1, spans[1].init(N_tri), 0) ||
        AxisSpans::AxisNotOverlap(cp1, P_A, P_B, P_C, spans[2].init(NW_AB)) ||
        AxisSpans::AxisNotOverlap(cp1, P_A, P_B, P_C, spans[3].init(NW_BC)) ||
        AxisSpans::AxisNotOverlap(cp1, P_A, P_B, P_C, spans[4].init(xVector3::CrossProduct(N_tri, cp1.N_top)) ) ||
        AxisSpans::AxisNotOverlap(cp1, P_A, P_B, P_C, spans[5].init(xVector3::CrossProduct(N_tri, spans[2].N_axis)) ) ||
        AxisSpans::AxisNotOverlap(cp1, P_A, P_B, P_C, spans[6].init(xVector3::CrossProduct(N_tri, spans[3].N_axis)) ) ||
        AxisSpans::AxisNotOverlap(cp1, P_A, P_B, P_C, spans[7].init(xVector3::CrossProduct(cp1.N_top, spans[2].N_axis)) ) ||
        AxisSpans::AxisNotOverlap(cp1, P_A, P_B, P_C, spans[8].init(xVector3::CrossProduct(cp1.N_top, spans[3].N_axis)) ) ||
        AxisSpans::AxisNotOverlap(cp1, P_A, P_B, P_C, spans[9].init(xVector3::CrossProduct(xVector3::CrossProduct(cp1.P_center-P_A, cp1.N_top), cp1.N_top) )  ) ||
        AxisSpans::AxisNotOverlap(cp1, P_A, P_B, P_C, spans[10].init(xVector3::CrossProduct(xVector3::CrossProduct(cp1.P_center-P_B, cp1.N_top), cp1.N_top) )  ) ||
        AxisSpans::AxisNotOverlap(cp1, P_A, P_B, P_C, spans[11].init(xVector3::CrossProduct(xVector3::CrossProduct(cp1.P_center-P_C, cp1.N_top), cp1.N_top) )  ) )
        return false;

    xVector3 NW_CA = P_A - P_C;

    if (AxisSpans::AxisNotOverlap(cp1, P_A, P_B, P_C, spans[12].init(NW_CA)) ||
        AxisSpans::AxisNotOverlap(cp1, P_A, P_B, P_C, spans[13].init(xVector3::CrossProduct(N_tri, spans[12].N_axis)) ) ||
        AxisSpans::AxisNotOverlap(cp1, P_A, P_B, P_C, spans[14].init(xVector3::CrossProduct(cp1.N_top, spans[12].N_axis)) ) )
        return false;

    xPoint3       P_closest_cap;
    xVector3      NW_closest;
    xBYTE         ID_edge;

    spans[15].wrong = true;
    xVector3 P_capCenter = cp1.P_center + cp1.N_top * cp1.S_top;
    if (xTriangle::PointRelation(P_A, P_B, P_C, P_capCenter, xFLOAT_HUGE_POSITIVE, N_tri, P_closest_cap, NW_closest, ID_edge) >= POINT_NearEdge &&
        xVector3::DotProduct(NW_closest, cp1.N_top) < 0)
        spans[15].init(P_closest_cap - P_capCenter);
    else
    {
        P_capCenter = cp1.P_center - cp1.N_top * cp1.S_top;
        if (xTriangle::PointRelation(P_A, P_B, P_C, P_capCenter, xFLOAT_HUGE_POSITIVE, N_tri, P_closest_cap, NW_closest, ID_edge) >= POINT_NearEdge &&
            xVector3::DotProduct(NW_closest, cp1.N_top) > 0)
            spans[15].init(P_closest_cap - P_capCenter);
    }
    if (AxisSpans::AxisNotOverlap(cp1, P_A, P_B, P_C, spans[15]))
        return false;

    int I_bestSpan;
    xVector3 NW_fix = AxisSpans::GetMinimalFix(spans, 16, I_bestSpan);
    if (I_bestSpan == 1)
        NW_fix.invert();

    xPoint3 P_closest[3];
    P_closest[0] = P_A - NW_fix;
    P_closest[1] = P_B - NW_fix;
    P_closest[2] = P_C - NW_fix;
    xBYTE   I_hits = 0;
    xPoint3 P_hits[6];

    HitPointsCapsuleTriangle(cp1, P_closest, P_hits, I_hits);
    for (int i = 0; i < I_hits; ++i)
        cset.Add(CollisionInfo(cp1, figure2, 0, ID_tri, NW_fix, P_hits[i], P_hits[i]+NW_fix));

    //tr2.PointRelation(P_collision, xFLOAT_HUGE_POSITIVE, N_tri, P_closest, NW_closest, ID_edge);
    return I_hits;
}

xDWORD CollideCapsuleTriangle (const xCapsule &cp1, const xTriangle &tr2, CollisionSet &cset)
{
    return CollideCapsuleTriangle (cp1, tr2.P_A, tr2.P_B, tr2.P_C, tr2, 0, cset);
}