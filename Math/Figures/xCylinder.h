#ifndef __incl_Math_xCylinder_h
#define __incl_Math_xCylinder_h

#include "xIFigure3d.h"

namespace Math { namespace Figures {
    using namespace ::Math::Figures;

    struct xCylinder : xIFigure3d
    {
        xVector3 N_top;
        xFLOAT   S_top;
        xFLOAT   S_radius;

        xCylinder() { Type = xIFigure3d::Cylinder; }

        xVector3 GetInnerRayCollisionPoint(const xVector3 &P_ray, const xVector3 &N_ray,
                                           const xVector3 &N_side) const;

        virtual void ComputeSpan(const xVector3 &N_axis, xFLOAT &P_min, xFLOAT &P_max, int axis = -1) const
        {
            // Based on http://www.geometrictools.com/Documentation/IntersectionOfCylinders.pdf
            xFLOAT P_middle = xVector3::DotProduct(N_axis, P_center);
            xFLOAT S_span;
            if (axis == 0)
                S_span = S_top;
            else
            {
                xFLOAT dNaNt = xVector3::DotProduct(N_axis, N_top);
                S_span = fabs(dNaNt) * S_top +
                       + S_radius * sqrt (N_axis.lengthSqr() - dNaNt*dNaNt);
            }
            P_min = P_middle-S_span;
            P_max = P_middle+S_span;
        }

        virtual xIFigure3d * Transform(const xMatrix  &MX_LocalToWorld)
        {
            xCylinder *res = new xCylinder();
            res->S_top     = S_top;
            res->S_radius  = S_radius;
            res->P_center  = MX_LocalToWorld.preTransformP(P_center);
            res->N_top     = MX_LocalToWorld.preTransformV(N_top);
            return res;
        }

        virtual void   P_MinMax_Get( xPoint3 &P_min, xPoint3 &P_max )
        {
            P_min = P_max = P_center;
            xFLOAT s = fabs(N_top.x) * S_top + S_radius * sqrt(1 - N_top.x*N_top.x);
            P_min.x -= s; P_max.x += s;
            s = fabs(N_top.y) * S_top + S_radius * sqrt(1 - N_top.y*N_top.y);
            P_min.y -= s; P_max.y += s;
            s = fabs(N_top.z) * S_top + S_radius * sqrt(1 - N_top.z*N_top.z);
            P_min.z -= s; P_max.z += s;
        }
        virtual xFLOAT S_Radius_Sqr_Get() { return S_top * S_top + S_radius * S_radius; }
        virtual xFLOAT W_Volume_Get()     { return S_top * PI * S_radius * S_radius; }
    };

} } // namespace Math.Figures

#endif
