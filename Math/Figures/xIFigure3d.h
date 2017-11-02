#ifndef __incl_Math_xIFigure3d_h
#define __incl_Math_xIFigure3d_h

#include <cstdio>
#include "../xMath.h"

namespace Math { namespace Figures {

    enum PointPosition {
        POINT_Inside     = 0,
        POINT_NearFace   = 1,
        POINT_NearEdge   = 2,
        POINT_NearCorner = 3,
        POINT_FarAway    = 4
    };

    struct xIFigure3d
    {
        enum xFigure3dType
        {
            Unset       = 0,
            Segment     = 1,
            Sphere      = 2,
            Capsule     = 3,
            BoxAligned  = 4,
            BoxOriented = 5,
            Cylinder    = 6,
            Triangle    = 7,
            Mesh        = 8
        } Type;

        xVector3 P_center;

        virtual ~xIFigure3d() = default;

        virtual void   P_MinMax_Get( xPoint3 &P_min, xPoint3 &P_max ) = 0;
        virtual xFLOAT S_Radius_Sqr_Get() = 0;
        virtual xFLOAT W_Volume_Get() = 0;

        virtual void ComputeSpan(const xVector3 &N_axis, xFLOAT &P_min, xFLOAT &P_max, int axis = -1) const = 0;

        virtual xIFigure3d * Transform(const xMatrix  &MX_LocalToWorld) = 0;

        virtual void free(bool transformationOnly = false) {}

        void               Save( FILE *file );
        static xIFigure3d *Load( FILE *file );

    protected:
        virtual void saveInstance( FILE *file ) {}
        virtual void loadInstance( FILE *file ) {}
    };

} } // namespace Math.Figures

#endif
