#include "xBoxO.h"

using namespace Math::Figures;

// 0 = inside, 1 = near face, 2 = near edge, 3 = near vertex
PointPosition xBoxO :: PointRelation(const xVector3 &P_test, xVector3 &NW_closest) const
{
    xFLOAT   S_tmp, S_distance = 0.f;
    xVector3 P_center_rel = P_test - P_center;
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

PointPosition xBoxO :: PointsClosestToPlane(const xVector3 &NW_plane, xPoint3 P_points[4]) const
{
    memset(P_points, 0, sizeof(xPoint3)*4);

    bool first_perpendicular = true;
    int  I_position = POINT_Inside;
    xFLOAT FL_axis_dir[3];

    for (int i = 0; i < 3; ++i)
    {
        FL_axis_dir[i] = Sign(xVector3::DotProduct(NW_plane, N_axis[i]));
        if (FL_axis_dir[i] != 0.f)
        {
            ++I_position;
            P_points[0] -= N_axis[i] * (S_extend[i] * FL_axis_dir[i]);
            P_points[1] -= N_axis[i] * (S_extend[i] * FL_axis_dir[i]);
            P_points[2] -= N_axis[i] * (S_extend[i] * FL_axis_dir[i]);
            P_points[3] -= N_axis[i] * (S_extend[i] * FL_axis_dir[i]);
        }
        else if (first_perpendicular)
        {
            first_perpendicular = false;
            P_points[0] += N_axis[i] * S_extend[i];
            P_points[1] -= N_axis[i] * S_extend[i];
            P_points[2] -= N_axis[i] * S_extend[i];
            P_points[3] += N_axis[i] * S_extend[i];
        }
        else
        {
            P_points[0] += N_axis[i] * S_extend[i];
            P_points[1] += N_axis[i] * S_extend[i];
            P_points[2] -= N_axis[i] * S_extend[i];
            P_points[3] -= N_axis[i] * S_extend[i];
        }
    }

    if (I_position == POINT_NearCorner)
    {
        P_points[0] += P_center;
        xVector3 NW_axis_0 = N_axis[0] * (S_extend[0] * FL_axis_dir[0]);
        xVector3 NW_axis_1 = N_axis[1] * (S_extend[1] * FL_axis_dir[1]);
        xVector3 NW_axis_2 = N_axis[2] * (S_extend[2] * FL_axis_dir[2]);
        P_points[1] = P_center - NW_axis_0 - NW_axis_1 + NW_axis_2;
        P_points[2] = P_center - NW_axis_0 + NW_axis_1 - NW_axis_2;
        P_points[3] = P_center + NW_axis_0 - NW_axis_1 - NW_axis_2;
    }
    else
    if (I_position == POINT_NearEdge)
    {
        P_points[0] += P_center;
        P_points[1] += P_center;
    }
    else
    {
        P_points[0] += P_center;
        P_points[1] += P_center;
        P_points[2] += P_center;
        P_points[3] += P_center;
    }

    return (PointPosition) I_position;
}
