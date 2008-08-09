// Included in FigureCollider.cpp
// Depends on AxisSpans.h
// Depends on xBoxo.h
// Depends on xCapsule.h

////////////////////////////// Capsule - BoxO

bool          TestCapsuleBoxO (const xCapsule &cp1, const xBoxO &bo2)
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

CollisionInfo CollideCapsuleBoxO(const xCapsule &cp1, const xBoxO &bo2)
{
    AxisSpans spans[8];
    if (AxisSpans::AxisNotOverlap(bo2, cp1, spans[0].init(bo2.N_axis[0]), 0) ||
        AxisSpans::AxisNotOverlap(bo2, cp1, spans[1].init(bo2.N_axis[1]), 1) ||
        AxisSpans::AxisNotOverlap(bo2, cp1, spans[2].init(bo2.N_axis[2]), 2) ||
        AxisSpans::AxisNotOverlap(cp1, bo2, spans[3].init(cp1.N_top), 0)     ||

        AxisSpans::AxisNotOverlap(cp1, bo2, spans[4].init(xVector3::CrossProduct(cp1.N_top, bo2.N_front)) ) ||
        AxisSpans::AxisNotOverlap(cp1, bo2, spans[5].init(xVector3::CrossProduct(cp1.N_top, bo2.N_side)) )  ||
        AxisSpans::AxisNotOverlap(cp1, bo2, spans[6].init(xVector3::CrossProduct(cp1.N_top, bo2.N_top)) ) )
        return CollisionInfo(false);

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
        return CollisionInfo(false);

    int I_bestSpan;
    xVector3 NW_fix = AxisSpans::GetMinimalFix(spans, 8, I_bestSpan);
    if (I_bestSpan >= 0 && I_bestSpan <= 2)
        NW_fix.invert();
    return CollisionInfo(NW_fix, xVector3::Create(0,0,0), xVector3::Create(0,0,0));
}
