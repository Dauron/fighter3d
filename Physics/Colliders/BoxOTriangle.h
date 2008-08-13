// Included in FigureCollider.cpp
// Depends on xBoxO.h
// Depends on xTriangle.h

////////////////////////////// BoxO - Triangle

bool   TestBoxOTriangle (const xBoxO &bo1, const xPoint3 &P_A,
                         const xPoint3 &P_B, const xPoint3 &P_C)
{
    AxisSpans spans;
    if (AxisSpans::AxisNotOverlap(bo1, P_A, P_B, P_C, spans.init(bo1.N_axis[0]), 0) ||
        AxisSpans::AxisNotOverlap(bo1, P_A, P_B, P_C, spans.init(bo1.N_axis[1]), 1) ||
        AxisSpans::AxisNotOverlap(bo1, P_A, P_B, P_C, spans.init(bo1.N_axis[2]), 2) )
        return false;

    xVector3 NW_AB = P_B - P_A;
    xVector3 NW_BC = P_C - P_B;
    xVector3 N_tri = xVector3::CrossProduct(NW_AB, NW_BC).normalize();

    if (AxisSpans::AxisNotOverlap(P_A, P_B, P_C, bo1, spans.init(N_tri), 0) ||
        AxisSpans::AxisNotOverlap(bo1, P_A, P_B, P_C, spans.init(xVector3::CrossProduct(bo1.N_front, NW_AB)) ) ||
        AxisSpans::AxisNotOverlap(bo1, P_A, P_B, P_C, spans.init(xVector3::CrossProduct(bo1.N_side,  NW_AB)) ) ||
        AxisSpans::AxisNotOverlap(bo1, P_A, P_B, P_C, spans.init(xVector3::CrossProduct(bo1.N_top,   NW_AB)) ) ||
        AxisSpans::AxisNotOverlap(bo1, P_A, P_B, P_C, spans.init(xVector3::CrossProduct(bo1.N_front, NW_BC)) ) ||
        AxisSpans::AxisNotOverlap(bo1, P_A, P_B, P_C, spans.init(xVector3::CrossProduct(bo1.N_side,  NW_BC)) ) ||
        AxisSpans::AxisNotOverlap(bo1, P_A, P_B, P_C, spans.init(xVector3::CrossProduct(bo1.N_top,   NW_BC)) ) )
        return false;

    xVector3 NW_CA = P_A - P_C;

    if (AxisSpans::AxisNotOverlap(bo1, P_A, P_B, P_C, spans.init(xVector3::CrossProduct(bo1.N_front, NW_CA)) ) ||
        AxisSpans::AxisNotOverlap(bo1, P_A, P_B, P_C, spans.init(xVector3::CrossProduct(bo1.N_side,  NW_CA)) ) ||
        AxisSpans::AxisNotOverlap(bo1, P_A, P_B, P_C, spans.init(xVector3::CrossProduct(bo1.N_top,   NW_CA)) ) )
        return false;
    return true;
}

inline bool TestBoxOTriangle (const xBoxO &bo1, const xTriangle &tr2)
{
    return TestBoxOTriangle(bo1, tr2.P_A, tr2.P_B, tr2.P_C);
}

void HitPointsBoxOTriangle(const xBoxO &bo2, const xPoint3 P_closest[3],
                           xPoint3       P_hits[4], xBYTE &I_hits)
{
    int I_touch = 0;
    xPoint3 P_min, P_max;

    for (int I_A = 0; I_A < 2; ++I_A)
    {
        int I_B = (I_A + 1 % 3);
        if (SegmentBoxOMinMax(P_closest[I_A], P_closest[I_B], bo2, P_min, P_max))
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

xDWORD CollideBoxOTriangle (const xBoxO &bo1, const xPoint3 &P_A,
                            const xPoint3 &P_B, const xPoint3 &P_C,
                            const xIFigure3d &figure2, xDWORD ID_tri, CollisionSet &cset)
{
    AxisSpans spans[13];
    if (AxisSpans::AxisNotOverlap(bo1, P_A, P_B, P_C, spans[0].init(bo1.N_axis[0]), 0) ||
        AxisSpans::AxisNotOverlap(bo1, P_A, P_B, P_C, spans[1].init(bo1.N_axis[1]), 1) ||
        AxisSpans::AxisNotOverlap(bo1, P_A, P_B, P_C, spans[2].init(bo1.N_axis[2]), 2) )
        return false;

    xVector3 NW_AB = P_B - P_A;
    xVector3 NW_BC = P_C - P_B;
    xVector3 N_tri = xVector3::CrossProduct(NW_AB, NW_BC).normalize();

    if (AxisSpans::AxisNotOverlap(P_A, P_B, P_C, bo1, spans[3].init(N_tri), 0) ||
        AxisSpans::AxisNotOverlap(bo1, P_A, P_B, P_C, spans[4].init(xVector3::CrossProduct(bo1.N_front, NW_AB)) ) ||
        AxisSpans::AxisNotOverlap(bo1, P_A, P_B, P_C, spans[5].init(xVector3::CrossProduct(bo1.N_side,  NW_AB)) ) ||
        AxisSpans::AxisNotOverlap(bo1, P_A, P_B, P_C, spans[6].init(xVector3::CrossProduct(bo1.N_top,   NW_AB)) ) ||
        AxisSpans::AxisNotOverlap(bo1, P_A, P_B, P_C, spans[7].init(xVector3::CrossProduct(bo1.N_front, NW_BC)) ) ||
        AxisSpans::AxisNotOverlap(bo1, P_A, P_B, P_C, spans[8].init(xVector3::CrossProduct(bo1.N_side,  NW_BC)) ) ||
        AxisSpans::AxisNotOverlap(bo1, P_A, P_B, P_C, spans[9].init(xVector3::CrossProduct(bo1.N_top,   NW_BC)) ) )
        return false;

    xVector3 NW_CA = P_A - P_C;

    if (AxisSpans::AxisNotOverlap(bo1, P_A, P_B, P_C, spans[10].init(xVector3::CrossProduct(bo1.N_front, NW_CA)) ) ||
        AxisSpans::AxisNotOverlap(bo1, P_A, P_B, P_C, spans[11].init(xVector3::CrossProduct(bo1.N_side,  NW_CA)) ) ||
        AxisSpans::AxisNotOverlap(bo1, P_A, P_B, P_C, spans[12].init(xVector3::CrossProduct(bo1.N_top,   NW_CA)) ) )
        return false;

    int I_bestSpan;
    xVector3 NW_fix = AxisSpans::GetMinimalFix(spans, 13, I_bestSpan);
    if (I_bestSpan == 3) NW_fix.invert();

    xPoint3 P_closest[3];
    P_closest[0] = P_A - NW_fix;
    P_closest[1] = P_B - NW_fix;
    P_closest[2] = P_C - NW_fix;
    xBYTE   I_hits = 0;
    xPoint3 P_hits[6];

    HitPointsBoxOTriangle(bo1, P_closest, P_hits, I_hits);
    for (int i = 0; i < I_hits; ++i)
        cset.Add(CollisionInfo(bo1, figure2, 0, ID_tri, NW_fix, P_hits[i], P_hits[i]+NW_fix));

    //xPoint3       P_closest;
    //xVector3      NW_closest;
    //xBYTE         ID_edge;
    //tr2.PointRelation(P_collision, xFLOAT_HUGE_POSITIVE, N_tri, P_closest, NW_closest, ID_edge);

    return I_hits;
}

inline xDWORD CollideBoxOTriangle (const xBoxO &bo1, const xTriangle &tr2, CollisionSet &cset)
{
    return CollideBoxOTriangle(bo1, tr2.P_A, tr2.P_B, tr2.P_C, tr2, 0, cset);
}
