#ifndef __incl_Math_xBoxO_h
#define __incl_Math_xBoxO_h

#include "xIFigure3d.h"
#include "xBoxA.h"

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
            xFLOAT   S_extend[3];
        };

        xBoxO() { Type = xIFigure3d::BoxOriented; }

        xBoxO &Init(xBoxA boxA)
        {
            P_center = (boxA.P_max + boxA.P_min) * 0.5f;
            N_top.init(0,0,1);
            N_side.init(1,0,0);
            N_front.init(0,1,0);
            S_top   = (boxA.P_max.z - boxA.P_min.z) * 0.5f;
            S_side  = (boxA.P_max.x - boxA.P_min.x) * 0.5f;
            S_front = (boxA.P_max.y - boxA.P_min.y) * 0.5f;
            return *this;
        }

        xPoint3 P_corners[8];

        const xPoint3 *FillCorners()
        {
            xVector3 NW_span_T = S_top   * N_top;
            xVector3 NW_span_F = S_front * N_front;
            xVector3 NW_span_S = S_side  * N_side;
            P_corners[0] = P_center + NW_span_T + NW_span_F + NW_span_S;
            P_corners[1] = P_center + NW_span_T + NW_span_F - NW_span_S;
            P_corners[2] = P_center + NW_span_T - NW_span_F + NW_span_S;
            P_corners[3] = P_center + NW_span_T - NW_span_F - NW_span_S;
            P_corners[4] = P_center - NW_span_T + NW_span_F + NW_span_S;
            P_corners[5] = P_center - NW_span_T + NW_span_F - NW_span_S;
            P_corners[6] = P_center - NW_span_T - NW_span_F + NW_span_S;
            P_corners[7] = P_center - NW_span_T - NW_span_F - NW_span_S;
            return P_corners;
        }

        bool CulledBy(const xPlane *PN_planes, int I_count)
        {
            // See if there is one plane for which all of the
            // vertices are in the negative half space.
            for (int p = 0; p < I_count; ++p) {

                bool FL_culled = true;
                int v;
                // Assume this plane culls all points.  See if there is a point
                // not culled by the plane... early out when at least one point
                // is in the positive half space.
                for (v = 0; (v < 8) && FL_culled; ++v)
                    FL_culled = PN_planes[p].distanceToPoint(P_corners[v]) > 0.2f;
                if (FL_culled) return true;
            }
            // None of the planes could cull this box
            return false;
        }

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

        PointPosition PointRelation(const xVector3 &P_test, xVector3 &NW_closest) const;
        PointPosition PointsClosestToPlane(const xVector3 &NW_plane, xPoint3 P_points[4]) const ;

        virtual void   P_MinMax_Get( xPoint3 &P_min, xPoint3 &P_max )
        {
            P_min = P_max = P_center;
            xFLOAT s = fabs(N_top.x) * S_top + fabs(N_side.x) * S_side + fabs(N_front.x) * S_front;
            P_min.x -= s; P_max.x += s;
            s = fabs(N_top.y) * S_top + fabs(N_side.y) * S_side + fabs(N_front.y) * S_front;
            P_min.y -= s; P_max.y += s;
            s = fabs(N_top.z) * S_top + fabs(N_side.z) * S_side + fabs(N_front.z) * S_front;
            P_min.z -= s; P_max.z += s;
        }
        virtual xFLOAT S_Radius_Sqr_Get() { return S_top*S_top+S_side*S_side+S_front*S_front; }
        virtual xFLOAT W_Volume_Get()     { return S_top * S_side * S_front * 8.f; }

    protected:
        virtual void loadInstance( FILE *file )
        {
            fread(&S_top,   sizeof(S_top), 1, file);
            fread(&N_top,   sizeof(N_top), 1, file);
            fread(&S_front, sizeof(S_front), 1, file);
            fread(&N_front, sizeof(N_front), 1, file);
            fread(&S_side,  sizeof(S_side), 1, file);
            fread(&N_side,  sizeof(N_side), 1, file);
        }
        virtual void saveInstance( FILE *file )
        {
            fwrite(&S_top, sizeof(S_top), 1, file);
            fwrite(&N_top, sizeof(N_top), 1, file);
            fwrite(&S_front, sizeof(S_front), 1, file);
            fwrite(&N_front, sizeof(N_front), 1, file);
            fwrite(&S_side,  sizeof(S_side), 1, file);
            fwrite(&N_side,  sizeof(N_side), 1, file);
        }
    };

} } // namespace Math.Figures

#endif
