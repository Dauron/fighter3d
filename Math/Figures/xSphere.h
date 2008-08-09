#ifndef __incl_Math_xSphere_h
#define __incl_Math_xSphere_h

#include "xIFigure3d.h"

namespace Math { namespace Figures {
    using namespace ::Math::Figures;

    struct xSphere : xIFigure3d
    {
        xFLOAT   S_radius;

        xSphere() { Type = xIFigure3d::Sphere; }

        virtual void ComputeSpan(const xVector3 &N_axis, xFLOAT &P_min, xFLOAT &P_max, int axis = -1) const
        {
            xFLOAT P_middle = xVector3::DotProduct(N_axis, P_center);
            P_min = P_middle - S_radius;
            P_max = P_middle + S_radius;
        }

        virtual xIFigure3d * Transform(const xMatrix  &MX_LocalToWorld)
        {
            xSphere *res  = new xSphere();
            res->S_radius = S_radius;
            res->P_center = MX_LocalToWorld.preTransformP(P_center);
            return res;
        }
    };

} } // namespace Math.Figures

#endif
