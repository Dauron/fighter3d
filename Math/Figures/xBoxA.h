#ifndef __incl_Math_xBoxA_h
#define __incl_Math_xBoxA_h

#include "xIFigure3d.h"

namespace Math { namespace Figures {
    using namespace ::Math::Figures;

    struct xBoxA
    {
        xPoint3 P_min;
        xPoint3 P_max;

        xPoint3 P_corners[8];

        bool Contains(const xPoint3 &P_point) const
        {
            return P_point >= P_min && P_point <= P_max;
        }

        const xPoint3 *fillCorners()
        {
            P_corners[0] = P_min;
            P_corners[1] = P_max;
            P_corners[2].init(P_min.x, P_min.y, P_max.z);
            P_corners[3].init(P_min.x, P_max.y, P_max.z);
            P_corners[4].init(P_max.x, P_min.y, P_max.z);
            P_corners[5].init(P_max.x, P_max.y, P_min.z);
            P_corners[6].init(P_max.x, P_min.y, P_min.z);
            P_corners[7].init(P_min.x, P_max.y, P_min.z);
            return P_corners;
        }

        const xVector3 *fillCornersTransformed(xMatrix MX_transform)
        {
            fillCorners();
            P_corners[0] = MX_transform.preTransformP(P_corners[0]);
            P_corners[1] = MX_transform.preTransformP(P_corners[1]);
            P_corners[2] = MX_transform.preTransformP(P_corners[2]);
            P_corners[3] = MX_transform.preTransformP(P_corners[3]);
            P_corners[4] = MX_transform.preTransformP(P_corners[4]);
            P_corners[5] = MX_transform.preTransformP(P_corners[5]);
            P_corners[6] = MX_transform.preTransformP(P_corners[6]);
            P_corners[7] = MX_transform.preTransformP(P_corners[7]);
            return P_corners;
        }

        bool culledBy(const xPlane *PN_planes, int I_count)
        {
            fillCorners();
            // See if there is one plane for which all of the
            // vertices are in the negative half space.
            for (int p = 0; p < I_count; ++p) {

                bool FL_culled = true;
                int v;
                // Assume this plane culls all points.  See if there is a point
                // not culled by the plane... early out when at least one point
                // is in the positive half space.
                for (v = 0; (v < 8) && FL_culled; ++v)
                    FL_culled = PN_planes[p].distanceToPoint(P_corners[v]) > 0;
                if (FL_culled) return true;
            }
            // None of the planes could cull this box
            return false;
        }
    };

} } // namespace Math.Figures

#endif
