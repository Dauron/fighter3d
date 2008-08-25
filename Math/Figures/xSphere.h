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
            ComputeSpan(P_center, S_radius, N_axis, P_min, P_max, axis);
        }
        static void ComputeSpan(const xPoint3 &P_center, xFLOAT S_radius,
                                const xVector3 &N_axis, xFLOAT &P_min, xFLOAT &P_max, int axis = -1)
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

        virtual void   P_MinMax_Get( xPoint3 &P_min, xPoint3 &P_max )
        {
            P_MinMax_Get(P_center, S_radius, P_min, P_max);
        }
        static void P_MinMax_Get( const xPoint3 &P_center, xFLOAT S_radius,
                                  xPoint3 &P_min, xPoint3 &P_max )
        {
            P_min = P_max = P_center;
            P_min.x -= S_radius; P_max.x += S_radius;
            P_min.y -= S_radius; P_max.y += S_radius;
            P_min.z -= S_radius; P_max.z += S_radius;
        }
        virtual xFLOAT S_Radius_Sqr_Get() { return S_radius*S_radius; }
        virtual xFLOAT W_Volume_Get()     { return 1.333333333333333f * PI * S_radius * S_radius * S_radius; }

    protected:
        virtual void loadInstance( FILE *file )
        {
            fread(&S_radius, sizeof(S_radius), 1, file);
        }
        virtual void saveInstance( FILE *file )
        {
            fwrite(&S_radius, sizeof(S_radius), 1, file);
        }
    };

} } // namespace Math.Figures

#endif
