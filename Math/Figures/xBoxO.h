#ifndef __incl_Math_xBoxO_h
#define __incl_Math_xBoxO_h

#include "xIFigure3d.h"

namespace Math { namespace Figures {
    using namespace ::Math::Figures;

    struct xBoxO : xIFigure3d
    {
        union {
            struct {
                xVector3 N_top;
                xVector3 N_front;
                xVector3 N_side;
            };
            xVector3 N_axis[3];
        };
        union {
            struct {
                xFLOAT   S_top;
                xFLOAT   S_front;
                xFLOAT   S_side;
            };
            xFLOAT S_extend[3];
        };

        xBoxO() { Type = xIFigure3d::BoxOriented; }

        void ComputeSpan(const xVector3 &N_axis, xFLOAT &P_min, xFLOAT &P_max, int axis = -1) const
        {
            xFLOAT P_middle = xVector3::DotProduct(N_axis, P_center);

            xFLOAT S_radius;
            if (axis >= 0 && axis <= 2)
                S_radius = S_extend[axis];
            else
                S_radius = fabs(xVector3::DotProduct(N_axis, N_front)) * S_front
                                + fabs(xVector3::DotProduct(N_axis, N_side))  * S_side
                                + fabs(xVector3::DotProduct(N_axis, N_top))   * S_top;
            P_min = P_middle-S_radius;
            P_max = P_middle+S_radius;
        }

        virtual xIFigure3d * Transform(const xMatrix  &MX_LocalToWorld)
        {
            xBoxO *res = new xBoxO();
            res->S_top    = S_top;
            res->S_side   = S_side;
            res->S_front  = S_front;
            res->P_center = MX_LocalToWorld.preTransformP(P_center);
            res->N_top    = MX_LocalToWorld.preTransformV(N_top);
            res->N_side   = MX_LocalToWorld.preTransformV(N_side);
            res->N_front  = MX_LocalToWorld.preTransformV(N_front);
            return res;
        }

        PointPosition PointRelation(const xVector3 &P_capCenter, xVector3 &NW_closest) const;
    };

} } // namespace Math.Figures

#endif
