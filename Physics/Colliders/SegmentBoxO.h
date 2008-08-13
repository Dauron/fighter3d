// Included in FigureCollider.cpp
// Depends on xSegment.h
// Depends on xBoxO.h

////////////////////////////// Segment - BoxO

bool SegmentBoxOMinMax(const xPoint3 &P_A, const xPoint3 &P_B, const xBoxO &bo2,
                       xPoint3 &P_min, xPoint3 &P_max)
{
    AxisSpans spans[3];
    xBYTE S_AV[3];

    for (int i = 0; i < 3; ++i)
    {
        spans[i].init(bo2.N_axis[i]);
        bo2.ComputeSpan(spans[i].N_axis, spans[i].P_min1, spans[i].P_max1, i);
        S_AV[i] = xSegment::ComputeSpan(P_A, P_B, spans[i].N_axis, spans[i].P_min2, spans[i].P_max2);
        if (spans[i].P_min1 > spans[i].P_max2 + EPSILON2 || spans[i].P_max1 + EPSILON2 < spans[i].P_min2) return false;
    }

    xFLOAT S_min = xFLOAT_HUGE_NEGATIVE, S_max = xFLOAT_HUGE_POSITIVE, S_tmp;

    for (int i = 0; i < 3; ++i)
        if (spans[i].P_max2 - spans[i].P_min2 > EPSILON)
        {
            if (S_AV[i] == 1)
            {
                S_tmp = (spans[i].P_min1 < spans[i].P_min2)
                    ? 0.f
                    : (spans[i].P_min1 - spans[i].P_min2) / (spans[i].P_max2 - spans[i].P_min2);
                if (S_tmp > S_min) S_min = S_tmp;

                S_tmp = (spans[i].P_max1 > spans[i].P_max2)
                    ? 1.f
                    : (spans[i].P_max1 - spans[i].P_min2) / (spans[i].P_max2 - spans[i].P_min2);
                if (S_tmp < S_max) S_max = S_tmp;
            }
            else
            {
                S_tmp = (spans[i].P_min1 < spans[i].P_min2)
                    ? 1.f
                    : (spans[i].P_min1 - spans[i].P_max2) / (spans[i].P_min2 - spans[i].P_max2);
                if (S_tmp < S_max) S_max = S_tmp;

                S_tmp = (spans[i].P_max1 > spans[i].P_max2)
                    ? 0.f
                    : (spans[i].P_max1 - spans[i].P_max2) / (spans[i].P_min2 - spans[i].P_max2);
                if (S_tmp > S_min) S_min = S_tmp;
            }
        }

    //if (S_max + EPSILON2 < S_min) return false;
    if (S_max < S_min)
    {
        S_min = S_max = (S_min + S_max) * 0.5f;
    }
    xVector3 NW_dir = P_B - P_A;
    P_min = P_A + NW_dir * S_min;
    P_max = P_A + NW_dir * S_max;
    return true;
}

bool SegmentBoxOMinMax(const xSegment &sg1, const xBoxO &bo2,
                       xPoint3 &P_min, xPoint3 &P_max, bool fast = true)
{
    AxisSpans spans[7];
    if (AxisSpans::AxisNotOverlapClose(bo2, sg1, spans[0].init(bo2.N_axis[0]), 0) ||
        AxisSpans::AxisNotOverlapClose(bo2, sg1, spans[1].init(bo2.N_axis[1]), 1) ||
        AxisSpans::AxisNotOverlapClose(bo2, sg1, spans[2].init(bo2.N_axis[2]), 2) )
        return false;

    if (!fast && (
        AxisSpans::AxisNotOverlapClose(sg1, bo2, spans[3].init(sg1.N_dir), 0)  ||
        AxisSpans::AxisNotOverlapClose(sg1, bo2, spans[4].init(xVector3::CrossProduct(sg1.N_dir, bo2.N_front)) ) ||
        AxisSpans::AxisNotOverlapClose(sg1, bo2, spans[5].init(xVector3::CrossProduct(sg1.N_dir, bo2.N_side)) )  ||
        AxisSpans::AxisNotOverlapClose(sg1, bo2, spans[6].init(xVector3::CrossProduct(sg1.N_dir, bo2.N_top)) ) ) )
        return false;

    xFLOAT S_min = xFLOAT_HUGE_NEGATIVE, S_max = xFLOAT_HUGE_POSITIVE, S_tmp;

    for (int i = 0; i < 3; ++i)
        if (spans[i].P_max2 - spans[i].P_min2 > EPSILON)
        {
            S_tmp = (spans[i].P_min1 < spans[i].P_min2)
                ? 0
                : (spans[i].P_min1 - spans[i].P_min2) / (spans[i].P_max2 - spans[i].P_min2);
            if (S_tmp > S_min) S_min = S_tmp;

            S_tmp = (spans[i].P_max1 > spans[i].P_max2)
                ? 1
                : (spans[i].P_max1 - spans[i].P_min2) / (spans[i].P_max2 - spans[i].P_min2);
            if (S_tmp < S_max) S_max = S_tmp;
        }

    if (S_max + EPSILON2 < S_min) return false;
    if (S_max < S_min)
    {
        S_min = S_max = (S_min + S_max) * 0.5f;
    }
    P_min = sg1.P_center + sg1.N_dir * ( sg1.S_dir * (2.f * S_min - 1.f) );
    P_max = sg1.P_center + sg1.N_dir * ( sg1.S_dir * (2.f * S_max - 1.f) );
    return true;
}