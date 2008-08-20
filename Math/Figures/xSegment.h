#ifndef __incl_Math_xSegment_h
#define __incl_Math_xSegment_h

#include "xIFigure3d.h"

namespace Math { namespace Figures {
    using namespace ::Math::Figures;

    struct xSegment : xIFigure3d
    {
        xVector3 N_dir;
        xFLOAT   S_dir;

        xSegment() { Type = xIFigure3d::Segment; }
        xSegment(const xPoint3 &P_Center, const xVector3 &N_Dir, xFLOAT S_Dir)
            : N_dir(N_Dir), S_dir(S_Dir)
        { Type = xIFigure3d::Segment; P_center = P_Center; }

        virtual void ComputeSpan(const xVector3 &N_axis, xFLOAT &P_min, xFLOAT &P_max, int axis = -1) const
        {
            xFLOAT P_middle = xVector3::DotProduct(N_axis, P_center);
            xFLOAT S_span = (axis == 0) ? S_dir : fabs(xVector3::DotProduct(N_axis, N_dir)) * S_dir;
            P_min = P_middle-S_span;
            P_max = P_middle+S_span;
        }
        static xBYTE ComputeSpan(const xPoint3 &P_A, const xPoint3 &P_B,
                                const xVector3 &N_axis, xFLOAT &P_min, xFLOAT &P_max, int axis = -1)
        {
            xFLOAT S_A = xVector3::DotProduct(N_axis, P_A);
            xFLOAT S_B = xVector3::DotProduct(N_axis, P_B);
            if (S_A <= S_B)
            { P_min = S_A; P_max = S_B; return 1; }
            else
            { P_min = S_B; P_max = S_A; return 2; }
        }

        virtual xIFigure3d * Transform(const xMatrix  &MX_LocalToWorld)
        {
            xSegment *res = new xSegment();
            res->P_center = MX_LocalToWorld.preTransformP(res->P_center);
            res->N_dir    = MX_LocalToWorld.preTransformV(res->N_dir);
            return res;
        }

        static void DistanceLineLine(const xVector3 &NW_axis_1, const xVector3 &P_axis_1,
                                        const xVector3 &NW_axis_2, const xVector3 &P_axis_2,
                                        xVector3 &P_closest_1,     xVector3 &P_closest_2)
        {
            xFLOAT dN1N2 = xVector3::DotProduct(NW_axis_1, NW_axis_2);
            if (dN1N2 >= 1.f - EPSILON || dN1N2 <= -1.f + EPSILON) // parallel
            {
                P_closest_1 = P_axis_1;
                xVector3 NW_21 = P_axis_2 - P_axis_1;
                xVector3 N_side = xVector3::CrossProduct(NW_axis_1, xVector3::CrossProduct(NW_axis_1, NW_21)).normalize();
                P_closest_2 = P_closest_1 + N_side * xVector3::DotProduct(N_side, NW_21);
                return;
            }

            // Based on http://local.wasp.uwa.edu.au/~pbourke/geometry/lineline3d/
            //dnmpo = dot(nm, po);
            //mua = ( dN1NW12 * dN1N2 - dN2NW12 * dN1N1 ) / ( dN2N2 * dN1N1 - dN1N2 * dN1N2 ) // dN2N2 = dN1N1 = 1
            //mub = ( dN1NW12 + mua * dN1N2 ) / dN1N1
            xVector3 NW_12 = P_axis_2 - P_axis_1;

            xFLOAT dN1N1   = xVector3::DotProduct(NW_axis_1, NW_axis_1);
            xFLOAT dN2N2   = xVector3::DotProduct(NW_axis_2, NW_axis_2);
            xFLOAT dN1NW12 = xVector3::DotProduct(NW_axis_1, NW_12);
            xFLOAT mub = (dN1NW12 * dN1N2 - xVector3::DotProduct(NW_axis_2, NW_12) * dN1N1) / (dN1N1*dN2N2 - dN1N2*dN1N2);
            xFLOAT mua = (dN1NW12 + mub * dN1N2) / dN1N1;

            P_closest_1 = P_axis_1 + mua * NW_axis_1;
            P_closest_2 = P_axis_2 + mub * NW_axis_2;
        }

        static void DistanceSegmentSegment(const xVector3 &NW_axis_1, const xVector3 &P_axis_1,
                                           const xVector3 &NW_axis_2, const xVector3 &P_axis_2,
                                           xVector3 &P_closest_1,     xVector3 &P_closest_2)
        {
            xFLOAT mua, mub;

            xFLOAT dN1N2 = xVector3::DotProduct(NW_axis_1, NW_axis_2);
            if (dN1N2 >= 1.f - EPSILON || dN1N2 <= -1.f + EPSILON) // parallel
            {
                xVector3 NW_21 = P_axis_2 - P_axis_1;
                xVector3 N_side = xVector3::CrossProduct(NW_axis_1, xVector3::CrossProduct(NW_axis_1, NW_21)).normalize();
                N_side = N_side * xVector3::DotProduct(N_side, NW_21);

                xFLOAT S_A1 = xVector3::DotProduct(P_axis_1, NW_axis_1);
                xFLOAT S_B1 = xVector3::DotProduct(P_axis_1+NW_axis_1, NW_axis_1);
                xFLOAT S_A2 = xVector3::DotProduct(P_axis_2, NW_axis_1);
                xFLOAT S_B2 = xVector3::DotProduct(P_axis_2+NW_axis_2, NW_axis_1);

                if ((dN1N2 > 0.f && S_A1 >= S_A2 && S_A1 <= S_B2) ||
                    (dN1N2 < 0.f && S_A1 >= S_B2 && S_A1 <= S_A2) )
                {
                    P_closest_1 = P_axis_1;
                    P_closest_2 = P_closest_1 + N_side;
                    return;
                }
                if ((dN1N2 > 0.f && S_B1 >= S_A2 && S_B1 <= S_B2) ||
                    (dN1N2 < 0.f && S_B1 >= S_B2 && S_B1 <= S_A2) )
                {
                    P_closest_1 = P_axis_1 + NW_axis_1;
                    P_closest_2 = P_closest_1 + N_side;
                    return;
                }
                if ((dN1N2 > 0.f && S_A2 >= S_A1 && S_A2 <= S_B1) ||
                    (dN1N2 < 0.f && S_A2 >= S_B1 && S_A2 <= S_A1) )
                {
                    P_closest_2 = P_axis_2;
                    P_closest_1 = P_closest_2 - N_side;
                    return;
                }
                if (S_A2 < S_A1)
                {
                    P_closest_1 = P_axis_1;
                    P_closest_2 = (dN1N2 > 0.f) ? P_axis_2 + NW_axis_2 : P_axis_2;
                }
                else
                {
                    P_closest_1 = P_axis_1 + NW_axis_1;
                    P_closest_2 = (dN1N2 < 0.f) ? P_axis_2 + NW_axis_2 : P_axis_2;
                }
                return;
            }
            // Based on http://local.wasp.uwa.edu.au/~pbourke/geometry/lineline3d/
            //dnmpo = dot(nm, po);
            //mua = ( dN1NW12 * dN1N2 - dN2NW12 * dN1N1 ) / ( dN2N2 * dN1N1 - dN1N2 * dN1N2 ) // dN2N2 = dN1N1 = 1
            //mub = ( dN1NW12 + mua * dN1N2 ) / dN1N1
            xVector3 NW_12 = P_axis_2 - P_axis_1;

            xFLOAT dN1N1   = xVector3::DotProduct(NW_axis_1, NW_axis_1);
            xFLOAT dN2N2   = xVector3::DotProduct(NW_axis_2, NW_axis_2);
            xFLOAT dN1NW12 = xVector3::DotProduct(NW_axis_1, NW_12);
            mub = (dN1NW12 * dN1N2 - xVector3::DotProduct(NW_axis_2, NW_12) * dN1N1) / (dN1N1*dN2N2 - dN1N2*dN1N2);
            mua = (dN1NW12 + mub * dN1N2) / dN1N1;

            // clamp
            if (mua <= 0.f) P_closest_1 = P_axis_1;
            else
            if (mua >= 1.f) P_closest_1 = P_axis_1 + NW_axis_1;
            else            P_closest_1 = P_axis_1 + mua * NW_axis_1;

            if (mub <= 0.f) P_closest_2 = P_axis_2;
            else
            if (mub >= 1.f) P_closest_2 = P_axis_2 + NW_axis_2;
            else            P_closest_2 = P_axis_2 + mub * NW_axis_2;
        }

        virtual void   P_MinMax_Get( xPoint3 &P_min, xPoint3 &P_max )
        {
            xPoint3 P_a = P_center + N_dir * S_dir;
            xPoint3 P_b = P_center - N_dir * S_dir;

            if (P_a.x >= P_b.x) { P_min.x = P_b.x; P_max.x = P_a.x; }
            else                { P_min.x = P_a.x; P_max.x = P_b.x; }
            if (P_a.y >= P_b.y) { P_min.y = P_b.y; P_max.y = P_a.y; }
            else                { P_min.y = P_a.y; P_max.y = P_b.y; }
            if (P_a.z >= P_b.z) { P_min.z = P_b.z; P_max.z = P_a.z; }
            else                { P_min.z = P_a.z; P_max.z = P_b.z; }
        }
        virtual xFLOAT S_Radius_Sqr_Get() { return S_dir * 2.f; }
        virtual xFLOAT W_Volume_Get()     { return 0.f; }
    };

} } // namespace Math.Figures

#endif
