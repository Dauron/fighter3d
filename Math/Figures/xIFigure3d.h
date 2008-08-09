#ifndef __incl_Math_xIFigure3d_h
#define __incl_Math_xIFigure3d_h

#include "../xMath.h"

namespace Math { namespace Figures {

    enum PointPosition {
        POINT_Inside     = 0,
        POINT_NearFace   = 1,
        POINT_NearEdge   = 2,
        POINT_NearCorner = 3
    };

    struct xIFigure3d
    {
        enum xFigure3dType
        {
            Unset       = 0,
            Sphere      = 1,
            Capsule     = 2,
            BoxAligned  = 3,
            BoxOriented = 4,
            Cylinder    = 5,
            Mesh        = 6
        } Type;

        xVector3 P_center;

        virtual void ComputeSpan(const xVector3 &N_axis, xFLOAT &P_min, xFLOAT &P_max, int axis = -1) const = 0;
        virtual xIFigure3d * Transform(const xMatrix  &MX_LocalToWorld) = 0;

        virtual void free(bool transformationOnly = false) {}
    };

} } // namespace Math.Figures

#endif
