// Included in FigureCollider.cpp
// Depends on AxisSpans.h
// Depends on xBoxo.h

////////////////////////////// BoxO - BoxO

bool   TestBoxOBoxO (const xBoxO &bo1, const xBoxO &bo2)
{
    AxisSpans spans;
    if (AxisSpans::AxisNotOverlap(bo1, bo2, spans.init(bo1.N_axis[0]), 0) ||
        AxisSpans::AxisNotOverlap(bo1, bo2, spans.init(bo1.N_axis[1]), 1) ||
        AxisSpans::AxisNotOverlap(bo1, bo2, spans.init(bo1.N_axis[2]), 2) ||

        AxisSpans::AxisNotOverlap(bo2, bo1, spans.init(bo2.N_axis[0]), 0) ||
        AxisSpans::AxisNotOverlap(bo2, bo1, spans.init(bo2.N_axis[1]), 1) ||
        AxisSpans::AxisNotOverlap(bo2, bo1, spans.init(bo2.N_axis[2]), 2) ||

        AxisSpans::AxisNotOverlap(bo1, bo2, spans.init(xVector3::CrossProduct(bo1.N_axis[0], bo2.N_axis[0])) ) ||
        AxisSpans::AxisNotOverlap(bo1, bo2, spans.init(xVector3::CrossProduct(bo1.N_axis[0], bo2.N_axis[1])) ) ||
        AxisSpans::AxisNotOverlap(bo1, bo2, spans.init(xVector3::CrossProduct(bo1.N_axis[0], bo2.N_axis[2])) ) ||

        AxisSpans::AxisNotOverlap(bo1, bo2, spans.init(xVector3::CrossProduct(bo1.N_axis[1], bo2.N_axis[0])) ) ||
        AxisSpans::AxisNotOverlap(bo1, bo2, spans.init(xVector3::CrossProduct(bo1.N_axis[1], bo2.N_axis[1])) ) ||
        AxisSpans::AxisNotOverlap(bo1, bo2, spans.init(xVector3::CrossProduct(bo1.N_axis[1], bo2.N_axis[2])) ) ||

        AxisSpans::AxisNotOverlap(bo1, bo2, spans.init(xVector3::CrossProduct(bo1.N_axis[2], bo2.N_axis[0])) ) ||
        AxisSpans::AxisNotOverlap(bo1, bo2, spans.init(xVector3::CrossProduct(bo1.N_axis[2], bo2.N_axis[1])) ) ||
        AxisSpans::AxisNotOverlap(bo1, bo2, spans.init(xVector3::CrossProduct(bo1.N_axis[2], bo2.N_axis[2])) ))
        return false;
    return true;
}

void HitPointsBoxOBoxO(const xBoxO  &bo2,
                       const xPoint3 P_closest[4], PointPosition PPosition,
                       xPoint3       P_hits[16],   xBYTE &I_hits)
{
    int I_touch = 0;
    xPoint3 P_min, P_max;

    if (PPosition == POINT_NearCorner)
    {
        for (int I_A = 1; I_A < 4; ++I_A)
        {
            if (SegmentBoxOMinMax(P_closest[0], P_closest[I_A], bo2, P_min, P_max))
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
        return;
    }

    int I_max = PPosition == POINT_NearEdge ? 1 : 4;

    for (int I_A = 0; I_A < I_max; ++I_A)
    {
        int I_B = (I_A + 1) % 4;
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

xDWORD CollideBoxOBoxO (const xBoxO &bo1, const xBoxO &bo2, CollisionSet &cset)
{
    AxisSpans spans[15];
    if (AxisSpans::AxisNotOverlap(bo1, bo2, spans[ 0].init(bo1.N_axis[0]), 0) ||
        AxisSpans::AxisNotOverlap(bo1, bo2, spans[ 1].init(bo1.N_axis[1]), 1) ||
        AxisSpans::AxisNotOverlap(bo1, bo2, spans[ 2].init(bo1.N_axis[2]), 2) ||

        AxisSpans::AxisNotOverlap(bo2, bo1, spans[ 3].init(bo2.N_axis[0]), 0) ||
        AxisSpans::AxisNotOverlap(bo2, bo1, spans[ 4].init(bo2.N_axis[1]), 1) ||
        AxisSpans::AxisNotOverlap(bo2, bo1, spans[ 5].init(bo2.N_axis[2]), 2) ||

        AxisSpans::AxisNotOverlap(bo1, bo2, spans[ 6].init(xVector3::CrossProduct(bo1.N_axis[0], bo2.N_axis[0])) ) ||
        AxisSpans::AxisNotOverlap(bo1, bo2, spans[ 7].init(xVector3::CrossProduct(bo1.N_axis[0], bo2.N_axis[1])) ) ||
        AxisSpans::AxisNotOverlap(bo1, bo2, spans[ 8].init(xVector3::CrossProduct(bo1.N_axis[0], bo2.N_axis[2])) ) ||

        AxisSpans::AxisNotOverlap(bo1, bo2, spans[ 9].init(xVector3::CrossProduct(bo1.N_axis[1], bo2.N_axis[0])) ) ||
        AxisSpans::AxisNotOverlap(bo1, bo2, spans[10].init(xVector3::CrossProduct(bo1.N_axis[1], bo2.N_axis[1])) ) ||
        AxisSpans::AxisNotOverlap(bo1, bo2, spans[11].init(xVector3::CrossProduct(bo1.N_axis[1], bo2.N_axis[2])) ) ||

        AxisSpans::AxisNotOverlap(bo1, bo2, spans[12].init(xVector3::CrossProduct(bo1.N_axis[2], bo2.N_axis[0])) ) ||
        AxisSpans::AxisNotOverlap(bo1, bo2, spans[13].init(xVector3::CrossProduct(bo1.N_axis[2], bo2.N_axis[1])) ) ||
        AxisSpans::AxisNotOverlap(bo1, bo2, spans[14].init(xVector3::CrossProduct(bo1.N_axis[2], bo2.N_axis[2])) ))
        return false;

    int I_bestSpan;
    xVector3 NW_fix = AxisSpans::GetMinimalFix(spans, 15, I_bestSpan);
    if (I_bestSpan >= 3 && I_bestSpan <= 5)
        NW_fix.invert();

    xBYTE   I_hits = 0;
    xPoint3 P_closest[4], P_hits[16];

    xBoxO bo = bo1;
    bo.P_center += NW_fix;

    PointPosition PPosition;
    PPosition = bo.PointsClosestToPlane(NW_fix, P_closest);
    HitPointsBoxOBoxO(bo2, P_closest, PPosition, P_hits, I_hits);
    PPosition = bo2.PointsClosestToPlane(-NW_fix, P_closest);
    HitPointsBoxOBoxO(bo, P_closest, PPosition, P_hits, I_hits);

    for (int i = 0; i < I_hits; ++i)
        cset.Add(CollisionInfo(bo1, bo2, NW_fix, P_hits[i]-NW_fix, P_hits[i]));

    return I_hits;
}
