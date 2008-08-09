#include "xCylinder.h"

using namespace Math::Figures;

xVector3 xCylinder :: GetInnerRayCollisionPoint(const xPoint3 &P_ray, const xVector3 &N_ray,
                                                const xVector3 &N_side) const
{
    xVector3 P_capCenter = P_center + N_top;
    xPlane   PN_collision; PN_collision.init(-N_top, P_capCenter);

    xVector3 P_collision = PN_collision.intersectRayPositive(P_ray, N_ray);

    // test collision with bottom cap
    if (P_collision.x == xFLOAT_HUGE_NEGATIVE)
    {
        P_capCenter = P_center - N_top;
        PN_collision.init(N_top, P_capCenter);
        P_collision = PN_collision.intersectRayPositive(P_ray, N_ray);
    }

    // it is collision with cylinder side
    if ((P_collision - P_capCenter).lengthSqr() >= S_radius*S_radius)
    {
        PN_collision.init(-N_side, P_center + N_side * S_radius);
        P_collision = PN_collision.intersectRayPositive(P_ray, N_ray);
    }

    return P_collision;
}
