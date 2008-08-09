#ifndef __incl_Math_xCapsule_h
#define __incl_Math_xCapsule_h

#include "xIFigure3d.h"

namespace Math { namespace Figures {

    struct xCapsule : xIFigure3d
    {
        xVector3 N_top;
        xFLOAT   S_top;
        xFLOAT   S_radius;

        xCapsule() { Type = xIFigure3d::Capsule; }

        virtual void ComputeSpan(const xVector3 &N_axis, xFLOAT &P_min, xFLOAT &P_max, int axis = -1) const
        {
            // Based on http://www.geometrictools.com/Documentation/IntersectionOfCylinders.pdf
            xFLOAT P_middle = xVector3::DotProduct(N_axis, P_center);
            xFLOAT S_span = (axis == 0) ? S_top : fabs(xVector3::DotProduct(N_axis, N_top)) * S_top;
            P_min = P_middle-S_radius-S_span;
            P_max = P_middle+S_radius+S_span;
        }

        virtual xIFigure3d * Transform(const xMatrix  &MX_LocalToWorld)
        {
            xCapsule *res = new xCapsule();
            res->S_top    = S_top;
            res->S_radius = S_radius;
            res->P_center = MX_LocalToWorld.preTransformP(P_center);
            res->N_top    = MX_LocalToWorld.preTransformV(N_top);
            return res;
        }
    };

} } // namespace Math.Figures

#endif
