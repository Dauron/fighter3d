#include "xBoxO.h"

using namespace Math::Figures;

// 0 = inside, 1 = near face, 2 = near edge, 3 = near vertex
PointPosition xBoxO :: PointRelation(const xVector3 &P_capCenter, xVector3 &NW_closest) const
{
    xFLOAT   S_tmp, S_distance = 0.f;
    xVector3 P_center_rel = P_capCenter - P_center;
    int      I_position   = POINT_Inside;
    NW_closest.zero();

    for (int i = 0; i < 3; ++i)
    {
        const xFLOAT &P_sphere = xVector3::DotProduct(P_center_rel, N_axis[i]);
        const xFLOAT &S_size   = S_extend[i];

        if (P_sphere >= S_size)
        {
            ++I_position;
            NW_closest += N_axis[i] * S_size;
        }
        else
        if (P_sphere <= -S_size)
        {
            ++I_position;
            NW_closest -= N_axis[i] * S_size;
        }
        else
            NW_closest += N_axis[i] * P_sphere;
    }

    return (PointPosition) I_position;
}
