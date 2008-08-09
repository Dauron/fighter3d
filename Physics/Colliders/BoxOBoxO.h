// Included in FigureCollider.cpp
// Depends on AxisSpans.h
// Depends on xBoxo.h

////////////////////////////// BoxO - BoxO

bool          TestBoxOBoxO (const xBoxO &bo1, const xBoxO &bo2)
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

CollisionInfo CollideBoxOBoxO (const xBoxO &bo1, const xBoxO &bo2)
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
        return CollisionInfo(false);

    int I_bestSpan;
    xVector3 NW_fix = AxisSpans::GetMinimalFix(spans, 15, I_bestSpan);
    if (I_bestSpan >= 3 && I_bestSpan <= 5)
        NW_fix.invert();
    return CollisionInfo(NW_fix, xVector3::Create(0,0,0), xVector3::Create(0,0,0));
}
