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

        PointPosition PointsClosestToPlane(const xVector3 &NW_plane, xPoint3 P_points[2]) const
        {
            xFLOAT   S_plane = NW_plane.length();
            xVector3 N_plane = NW_plane / S_plane;
            xFLOAT W_cos = xVector3::DotProduct(N_plane, N_top);

            if (W_cos > -EPSILON && W_cos < EPSILON)
            {
                N_plane *= S_radius;
                P_points[0] = P_center + N_top * S_top - N_plane;
                P_points[1] = P_center - N_top * S_top - N_plane;
                return POINT_NearEdge;
            }
            if (W_cos < 0.f)
                P_points[1] = P_points[0] = P_center + N_top * S_top - N_plane * S_radius;
            else
                P_points[1] = P_points[0] = P_center - N_top * S_top - N_plane * S_radius;
            return POINT_NearCorner;
        }

        virtual xFLOAT S_Radius_Sqr_Get() { return (S_top+S_radius)*(S_top+S_radius); }
        virtual xFLOAT W_Volume_Get()     { return (1.333333333333333f * S_radius + 2.f * S_top ) * PI * S_radius * S_radius; }
    };

} } // namespace Math.Figures

#endif
