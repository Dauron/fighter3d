#ifndef __incl_Math_xTriangle_h
#define __incl_Math_xTriangle_h

#include "xIFigure3d.h"

namespace Math { namespace Figures {
    using namespace ::Math::Figures;

    struct xTriangle : xIFigure3d
    {
        xPoint3 P_A;
        xPoint3 P_B;
        xPoint3 P_C;

        xTriangle() { Type = xIFigure3d::Triangle; }
        xTriangle(const xPoint3 &P_a, const xPoint3 &P_b, const xPoint3 &P_c)
            : P_A(P_a), P_B(P_b), P_C(P_c)
        { Type = xIFigure3d::Triangle; }

        static void ComputeSpan(const xPoint3 &P_A, const xPoint3 &P_B, const xPoint3 &P_C,
                                const xVector3 &N_axis, xFLOAT &P_min, xFLOAT &P_max, int axis = -1)
        {
            if (axis == 0)
                P_min = P_max = xVector3::DotProduct(N_axis, P_A);
            else
            {
                xFLOAT P_a = xVector3::DotProduct(N_axis, P_A);
                xFLOAT P_b = xVector3::DotProduct(N_axis, P_B);
                xFLOAT P_c = xVector3::DotProduct(N_axis, P_C);

                if (P_a <= P_b)
                {
                    if (P_b <= P_c)
                    {
                        P_min = P_a;
                        P_max = P_c;
                    }
                    else
                    {
                        P_max = P_b;
                        if (P_a <= P_c)
                            P_min = P_a;
                        else
                            P_min = P_c;
                    }
                }
                else
                if (P_a <= P_c)
                {
                    P_min = P_b;
                    P_max = P_c;
                }
                else
                {
                    P_max = P_a;
                    if (P_b <= P_c)
                        P_min = P_b;
                    else
                        P_min = P_c;
                }
            }
        }


        virtual void ComputeSpan(const xVector3 &N_axis, xFLOAT &P_min, xFLOAT &P_max, int axis = -1) const
        {
            xTriangle::ComputeSpan(P_A, P_B, P_C, N_axis, P_min, P_max, axis);
        }

        virtual xIFigure3d * Transform(const xMatrix  &MX_LocalToWorld)
        {
            xTriangle *res  = new xTriangle();
            //res->P_center = MX_LocalToWorld.preTransformP(res->P_center);
            res->P_A = MX_LocalToWorld.preTransformP(res->P_A);
            res->P_B = MX_LocalToWorld.preTransformP(res->P_B);
            res->P_C = MX_LocalToWorld.preTransformP(res->P_C);
            return res;
        }

        virtual void   P_MinMax_Get( xPoint3 &P_min, xPoint3 &P_max )
        {
            GetMinMax3(P_A.x,P_B.x,P_C.x, P_min.x, P_max.x);
            GetMinMax3(P_A.y,P_B.y,P_C.y, P_min.y, P_max.y);
            GetMinMax3(P_A.z,P_B.z,P_C.z, P_min.z, P_max.z);
        }
        virtual xFLOAT S_Radius_Sqr_Get()
        {
            xFLOAT a2 = (P_B - P_C).lengthSqr();
            xFLOAT b2 = (P_A - P_C).lengthSqr();
            xFLOAT c2 = (P_B - P_A).lengthSqr();
            xFLOAT a2b2_c2 = (a2 + b2 - c2);
            return a2 * b2 * c2 / ( 4 * a2 * b2 - a2b2_c2*a2b2_c2 );
        }
        virtual xFLOAT W_Volume_Get()
        {
            xFLOAT a2 = (P_B - P_C).lengthSqr();
            xFLOAT b2 = (P_A - P_C).lengthSqr();
            xFLOAT c2 = (P_B - P_A).lengthSqr();
            xFLOAT a2b2_c2 = (a2 + b2 - c2);
            return 0.25f * sqrt( 4 * a2 * b2 - a2b2_c2*a2b2_c2 );
        }

        static PointPosition PointRelation( const xPoint3 &P_A, const xPoint3 &P_B, const xPoint3 &P_C,
                                            const xVector3 &P_test, xFLOAT S_max_Sqr,
                                            xVector3 &N_tri, xPoint3 &P_closest,
                                            xVector3 &NW_closest, xBYTE &ID_edge)
        {
            xVector3 NW_AB = P_B - P_A;
            xVector3 NW_AC = P_C - P_A;
            // TODO: check if must be normalized
            N_tri = xVector3::CrossProduct(NW_AB, NW_AC).normalize();

            xPlane PN_tri; PN_tri.init(N_tri, P_A);
            xFLOAT S_distance = PN_tri.distanceToPoint(P_test);
            if (S_distance*S_distance + EPSILON2 > S_max_Sqr) return POINT_FarAway;

            xPoint3       P_cast = P_test - N_tri * S_distance; // PN_tri.castPoint(sp1.P_center);
            xVector3      NW_BC;
            PointPosition Position = POINT_NearCorner;

            // Should be on the right of the AB vector to be INSIDE
            xVector3 NW_AP = P_cast - P_A;
            xVector3 NW_side = xVector3::CrossProduct(NW_AB, NW_AP);
            // Outside edge AB?
            if (xVector3::DotProduct(N_tri, NW_side) < 0)
            {
                xFLOAT W_ABP = xVector3::DotProduct(NW_AB, NW_AP);
                // Near corner A?
                if (W_ABP < 0)
                    P_closest = P_A;
                else
                // Near corner B?
                if (W_ABP > NW_AB.lengthSqr())
                    P_closest = P_B;
                // Near edge
                else
                {
                    P_closest = P_A + NW_AB * (W_ABP / NW_AB.lengthSqr());
                    Position  = POINT_NearEdge;
                    ID_edge   = 1;
                }
            }
            else
            {
                // Should be on the left of the AC vector
                xVector3 NW_side = xVector3::CrossProduct(NW_AC, NW_AP);
                // Outside edge AC?
                if (xVector3::DotProduct(N_tri, NW_side) > 0)
                {
                    xFLOAT W_ACP = xVector3::DotProduct(NW_AC, NW_AP);
                    // Near corner A? - tested for AB edge
                    if (W_ACP < 0)
                        P_closest = P_A;
                    else
                    // Near corner C?
                    if (W_ACP > NW_AC.lengthSqr())
                        P_closest = P_C;
                    // Near edge
                    else
                    {
                        P_closest = P_A + NW_AC * (W_ACP / NW_AC.lengthSqr());
                        Position  = POINT_NearEdge;
                        ID_edge   = 2;
                    }
                }
                else
                {
                    // Should be on the right of the BC vector
                    NW_BC = P_C - P_B;
                    xVector3 NW_BP = P_cast - P_B;
                    NW_side = xVector3::CrossProduct(NW_BC, NW_BP);
                    // Outside edge BC?
                    if (xVector3::DotProduct(N_tri, NW_side) < 0)
                    {
                        xFLOAT W_BCP = xVector3::DotProduct(NW_BC, NW_BP);
                        // Near corner B? - tested for AB edge
                        if (W_BCP < 0)
                            P_closest = P_B;
                        else
                        // Near corner B? - tested for AC edge
                        if (W_BCP * W_BCP > NW_AC.lengthSqr())
                            P_closest = P_C;
                        // Near edge
                        else
                        {
                            P_closest = P_B + NW_BC * (W_BCP / NW_BC.lengthSqr());
                            Position  = POINT_NearEdge;
                            ID_edge   = 3;
                        }
                    }
                    // Inside the triangle
                    else
                    {
                        P_closest = P_cast;
                        Position  = POINT_Inside;
                    }
                }
            }

            NW_closest = P_test - P_closest;
            S_distance = NW_closest.lengthSqr();
            if (S_distance + EPSILON2 > S_max_Sqr) return POINT_FarAway;

            return Position;
        }

        PointPosition PointRelation(const xVector3 &P_test, xFLOAT S_max_Sqr,
                                    xVector3 &N_tri, xPoint3 &P_closest,
                                    xVector3 &NW_closest, xBYTE &ID_edge) const
        {
            return PointRelation(P_A, P_B, P_C, P_test, S_max_Sqr, N_tri, P_closest, NW_closest, ID_edge);
        }

        static bool PointInsideTheTriangle(const xPoint3& P_test,
                                       const xPoint3& P_A,const xPoint3& P_B, const xPoint3& P_C)
        {
            xVector3 NW_AB = P_B - P_A;
            xVector3 NW_AC = P_C - P_A;
            xVector3 NW_tri_normal = xVector3::CrossProduct(NW_AB, NW_AC);

            // Should be on the right of the AB vector
            xVector3 NW_AP = P_test - P_A;
            xVector3 NW_side = xVector3::CrossProduct(NW_AB, NW_AP);
            if (xVector3::DotProduct(NW_tri_normal, NW_side) < 0) return false;

            // Should be on the left of the AC vector
            NW_side = xVector3::CrossProduct(NW_AC, NW_AP);
            if (xVector3::DotProduct(NW_tri_normal, NW_side) > 0) return false;

            // Should be on the right of the BC vector
            xVector3 NW_BC = P_C - P_B;
            xVector3 NW_BP = P_test - P_B;
            NW_side = xVector3::CrossProduct(NW_BC, NW_BP);
            if (xVector3::DotProduct(NW_tri_normal, NW_side) < 0) return false;

            return true;
        }


        // Based on http://www.peroxide.dk/papers/collision/collision.pdf
        // Some reference on http://www.blackpawn.com/texts/pointinpoly/default.html
        /*
        #define INSIDE(a) ((xDWORD&) a)
        static bool PointInsideTheTriangleEx(const xPoint3& P_test,
                                             const xPoint3& P_A,const xPoint3& P_B, const xPoint3& P_C)
        {
            xVector3 NW_C = P_B - P_A;
            xVector3 NW_B = P_C - P_A;
            xVector3 NW_P = P_test - P_A;

            xFLOAT W_C = xVector3::DotProduct(NW_C, NW_C);
            xFLOAT W_B = xVector3::DotProduct(NW_B, NW_B);
            xFLOAT W_A = xVector3::DotProduct(NW_B, NW_C);
            xFLOAT bc_aa = (W_B * W_C) - (W_A * W_A);

            xFLOAT W_D = xVector3::DotProduct(NW_C, NW_P);
            xFLOAT W_E = xVector3::DotProduct(NW_B, NW_P);

            xFLOAT x = (W_D * W_B) - (W_E * W_A);
            xFLOAT y = (W_E * W_C) - (W_D * W_A);
            xFLOAT z = x + y - bc_aa;

            return (( INSIDE(z) & ~(INSIDE(x) | INSIDE(y)) ) & 0x80000000);
        }
        */
    };

} } // namespace Math.Figures

#endif
