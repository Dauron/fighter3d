// Included in FigureCollider.cpp
// Depends on AxisSpans.h
// Depends on xBoxo.h
// Depends on xCapsule.h

////////////////////////////// Capsule - BoxO

bool   TestCapsuleBoxO (const xCapsule &cp1, const xBoxO &bo2)
{
    AxisSpans spans;
    if (AxisSpans::AxisNotOverlap(bo2, cp1, spans.init(bo2.N_axis[0]), 0) ||
        AxisSpans::AxisNotOverlap(bo2, cp1, spans.init(bo2.N_axis[1]), 1) ||
        AxisSpans::AxisNotOverlap(bo2, cp1, spans.init(bo2.N_axis[2]), 2) ||
        AxisSpans::AxisNotOverlap(cp1, bo2, spans.init(cp1.N_top), 0)     ||

        AxisSpans::AxisNotOverlap(cp1, bo2, spans.init(xVector3::CrossProduct(cp1.N_top, bo2.N_front)) ) ||
        AxisSpans::AxisNotOverlap(cp1, bo2, spans.init(xVector3::CrossProduct(cp1.N_top, bo2.N_side)) )  ||
        AxisSpans::AxisNotOverlap(cp1, bo2, spans.init(xVector3::CrossProduct(cp1.N_top, bo2.N_top)) ) )
        return false;

    xVector3 NW_closest;
    spans.wrong = true;
    xVector3 P_capCenter = cp1.P_center + cp1.N_top * cp1.S_top;
    if (bo2.PointRelation(P_capCenter, NW_closest) >= POINT_NearEdge &&
        xVector3::DotProduct(NW_closest, cp1.N_top) < -0.70710678118654752440084436210485f-EPSILON) // 45deg
        spans.init(bo2.P_center + NW_closest - P_capCenter);
    else
    {
        P_capCenter = cp1.P_center - cp1.N_top * cp1.S_top;
        if (bo2.PointRelation(P_capCenter, NW_closest) >= POINT_NearEdge &&
            xVector3::DotProduct(NW_closest, cp1.N_top) > 0.70710678118654752440084436210485f+EPSILON) // 45deg
            spans.init(bo2.P_center + NW_closest - P_capCenter);
    }
    if (!spans.wrong && AxisSpans::AxisNotOverlap(cp1, bo2, spans))
        return false;

    return true;
}

void HitPointsCapsuleBoxO(const xCapsule &cp2,
                          const xPoint3   P_closest[4], PointPosition PPosition,
                          xPoint3         P_hits[8],    xBYTE &I_hits)
{
    int I_touch = 0;
    xPoint3 P_min, P_max;

    if (PPosition == POINT_NearCorner)
    {
        for (int I_A = 1; I_A < 4; ++I_A)
        {
            if (SegmentCapsuleMinMax(P_closest[0], P_closest[I_A], cp2, P_min, P_max))
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

void HitPointsCapsuleBoxO(const xBoxO    &bo2,
                          const xPoint3   P_closest[2], PointPosition PPosition,
                          xPoint3         P_hits[8],    xBYTE &I_hits)
{
    if (PPosition == POINT_NearCorner)
    {
        xVector3 NW_closest;
        if (bo2.PointRelation(P_closest[0], NW_closest) == POINT_Inside || (P_closest[0] - bo2.P_center - NW_closest).lengthSqr() < EPSILON2 )
        {
            bool FL_found = false;
            for (int i = 0; i < I_hits; ++i)
                if (P_closest[0] == P_hits[i]) { FL_found = true; break; }
            if (!FL_found)
                P_hits[I_hits++] = P_closest[0];
        }
        return;
    }

    xPoint3 P_min, P_max;

    if (SegmentBoxOMinMax(P_closest[0], P_closest[1], bo2, P_min, P_max))
    {
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
        return;
    }
    /*
    for (int I_A = 0; I_A < 2; ++I_A)
    {
        bool FL_found = false;
        for (int i = 0; i < I_hits; ++i)
            if (P_closest[I_A] == P_hits[i]) { FL_found = true; break; }
        if (!FL_found)
            P_hits[I_hits++] = P_closest[I_A];
    }
    */
}

xDWORD CollideCapsuleBoxO(const xCapsule &cp1, const xBoxO &bo2,
                          IPhysicalBody *body1, IPhysicalBody *body2, CollisionSet &cset)
{
    AxisSpans spans[8];
    if (AxisSpans::AxisNotOverlap(bo2, cp1, spans[0].init(bo2.N_axis[0]), 0) ||
        AxisSpans::AxisNotOverlap(bo2, cp1, spans[1].init(bo2.N_axis[1]), 1) ||
        AxisSpans::AxisNotOverlap(bo2, cp1, spans[2].init(bo2.N_axis[2]), 2) ||
        AxisSpans::AxisNotOverlap(cp1, bo2, spans[3].init(cp1.N_top), 0)     ||

        AxisSpans::AxisNotOverlap(cp1, bo2, spans[4].init(xVector3::CrossProduct(cp1.N_top, bo2.N_front)) ) ||
        AxisSpans::AxisNotOverlap(cp1, bo2, spans[5].init(xVector3::CrossProduct(cp1.N_top, bo2.N_side)) )  ||
        AxisSpans::AxisNotOverlap(cp1, bo2, spans[6].init(xVector3::CrossProduct(cp1.N_top, bo2.N_top)) ) )
        return false;

    xVector3 NW_closest;
    spans[7].wrong = true;
    xVector3 P_capCenter = cp1.P_center + cp1.N_top * cp1.S_top;
    if (bo2.PointRelation(P_capCenter, NW_closest) > 1 && xVector3::DotProduct(NW_closest, cp1.N_top) < -0.70710678118654752440084436210485f-EPSILON) // 45deg
        spans[7].init(bo2.P_center + NW_closest - P_capCenter);
    else
    {
        P_capCenter = cp1.P_center - cp1.N_top * cp1.S_top;
        if (bo2.PointRelation(P_capCenter, NW_closest) > 1 && xVector3::DotProduct(NW_closest, cp1.N_top) > 0.70710678118654752440084436210485f+EPSILON) // 45deg
            spans[7].init(bo2.P_center + NW_closest - P_capCenter);
    }
    if (!spans[7].wrong && AxisSpans::AxisNotOverlap(cp1, bo2, spans[7]))
        return false;

    int I_bestSpan = 0;
    xVector3 NW_fix = AxisSpans::GetMinimalFix(spans, 8, I_bestSpan);
    if (I_bestSpan >= 0 && I_bestSpan <= 2)
        NW_fix.invert();

    //cset.Add(CollisionInfo(cp1, bo2, NW_fix, xPoint3::Create(0,0,0), xPoint3::Create(0,0,0)));

    xBYTE   I_hits = 0;
    xPoint3 P_closest[4], P_closest2[2], P_hits[16];

    xBoxO bo = bo2;
    bo.P_center -= NW_fix;

    PointPosition PPosition = bo.PointsClosestToPlane(-NW_fix, P_closest);
    HitPointsCapsuleBoxO(cp1, P_closest, PPosition, P_hits, I_hits);
    PointPosition PPosition2 = cp1.PointsClosestToPlane(NW_fix, P_closest2);
    HitPointsCapsuleBoxO(bo, P_closest2, PPosition2, P_hits, I_hits);

    if (I_hits == 0)
        if (PPosition == POINT_NearCorner)
            P_hits[I_hits++] = P_closest[0];
        else
        {
            PPosition = bo2.PointsClosestToPlane(-NW_fix, P_closest);
            HitPointsCapsuleBoxO(cp1, P_closest, PPosition, P_hits, I_hits);
            if (I_hits)
            {
                for (int i = 1; i < I_hits; ++i)
                    P_hits[0] += P_hits[i];
                P_hits[0] /= I_hits;
                P_hits[0] -= NW_fix;
                I_hits = 1;
            }
            else
            if (PPosition2 == POINT_NearCorner) P_hits[I_hits++] = P_closest2[0];
            else                                P_hits[I_hits++] = (P_closest2[0]+P_closest2[1])*0.5f;
        }

    for (int i = 0; i < I_hits; ++i)
        cset.Add(CollisionInfo(body1, body2, cp1, bo2, NW_fix, P_hits[i], P_hits[i]+NW_fix));

    return I_hits;
}
