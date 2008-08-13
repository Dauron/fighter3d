// Included in FigureCollider.cpp
// Depends on xTriangle.h

////////////////////////////// Triangle - Triangle

#define det3(x1, y1, z1, x2, y2, z2, x3, y3, z3)    (x1)*((y2)*(z3)-(z2)*(y3))+(y1)*((z2)*(x3)-(x2)*(z3))+(z1)*((x2)*(y3)-(y2)*(x3));
#define determinant(vA, vB, vC)                     (vA->x-vC->x)*(vB->y-vC->y)-(vA->y-vC->y)*(vB->x-vC->x)
#define sgn(a)                                      ((a) > 0 ? 1 : ( (a) < 0 ? -1 : 0 ));

int  CheckPlanes(const xPoint3 &a1, const xPoint3 &a2, const xPoint3 &a3,
                 const xPoint3 &b1, const xPoint3 &b2, const xPoint3 &b3)
{
    // Calculate plane (with CrossProduct)
    float p1x = a2.x - a1.x, p1y = a2.y - a1.y, p1z = a2.z - a1.z;
    float p2x = a3.x - a1.x, p2y = a3.y - a1.y, p2z = a3.z - a1.z;
    float X = p1y*p2z - p1z*p2y, Y = p1z*p2x - p1x*p2z, Z = p1x*p2y - p1y*p2x;
    float W = -(a1.x*X + a1.y*Y + a1.z*Z);
    // Check positions (with DotProducts+W)
    float P1 = b1.x * X + b1.y * Y + b1.z * Z + W;
    float P2 = b2.x * X + b2.y * Y + b2.z * Z + W;
    float P3 = b3.x * X + b3.y * Y + b3.z * Z + W;
/*
    xPlane plane; plane.init(xVector3::CrossProduct(a2-a1,a3-a1), a1);
    float P1 = plane.distanceToPoint(b1);
    float P2 = plane.distanceToPoint(b2);
    float P3 = plane.distanceToPoint(b3);
*/
    if ((P1 > 0.f && P2 > 0.f && P3 > 0.f) || (P1 < 0.f && P2 < 0.f && P3 < 0.f)) // no intersection
        return 0;
    else
    if (P1 == 0.f && P2 == 0.f && P3 == 0.f)                                      // coplanar
        return -1;

    // possible intersection
    if (P2*P3 > 0.f || (P2 == 0.f && P3 == 0.f))
        return 1;
    if (P1*P3 > 0.f || (P1 == 0.f && P3 == 0.f))
        return 2;
    return 3;
}

bool TestTriangleTriangle(const xPoint3 &P_a1, const xPoint3 &P_a2, const xPoint3 &P_a3,
                          const xPoint3 &P_b1, const xPoint3 &P_b2, const xPoint3 &P_b3)
{
    int r = CheckPlanes(P_a1, P_a2, P_a3, P_b1, P_b2, P_b3);
    if (!r) return false;

    const xPoint3 *a1 = &P_a1, *a2 = &P_a2, *a3 = &P_a3;
    const xPoint3 *b1 = &P_b1, *b2 = &P_b2, *b3 = &P_b3;

    const xPoint3 *swp;
    if (r > 0)
    {
        if (r == 2) { swp = b1; b1 = b2; b2 = b3; b3 = swp; }
        else
        if (r == 3) { swp = b1; b1 = b3; b3 = b2; b2 = swp; }

        r = CheckPlanes(*b1, *b2, *b3, *a1, *a2, *a3);
        if (!r) return false;
    }

    if (r == -1) // coplanar
    {
        return false; // we don't treat 2d collisions as collisions, they just touch, but do not collide

        if (determinant(a1,a2,a3) < 0.f) { swp = a2; a2 = a3; a3 = swp; }
        if (determinant(b1,b2,b3) < 0.f) { swp = b2; b2 = b3; b3 = swp; }

        float P1 = determinant(a1, b1, b2);
        float P2 = determinant(a1, b2, b3);
        float P3 = determinant(a1, b3, b1);

        if (P1 >= 0.f && P2 >= 0.f && P3 >= 0.f) // inside, on the vertex, on the edge or "empty triangle"
            return true;

        while (determinant(a1, b1, b2) < 0 || determinant(a1, b1, b3) < 0)
        { swp = b1; b1 = b2; b2 = b3; b3 = swp; }

        if (determinant(a1, b2, b3) > 0) // Region R1
        {
            if (determinant(b3, b1, a2) >= 0)    // I
            {
                if (determinant(b3, a1, a2) < 0)  // IIa
                    return false;
                else
                {
                    if (determinant(a1, b1, a2) >= 0) // IIIa
                        return true;
                    else
                    {
                        if (determinant(a1, b1, a3) < 0) // IV a
                            return false;
                        else
                        {
                            if (determinant(a2, a3, b1) >= 0) // V
                                return true;
                            else
                                return false;
                        }
                    }
                }
            }
            else
            {
                if (determinant(b3, b1, a3) < 0) // IIb
                    return false;
                else
                {
                    if (determinant(a2, a3, b3) > 0) // IIIb
                        return false;
                    else
                    {
                        if (determinant(a1, b1, a3) > 0) // IVb
                            return false;
                        else
                            return true;
                    }
                }
            }
        }
        else                             // Region R2
        {
            if (determinant(b3, b1, a2) >= 0)        // I
            {
                if (determinant(b2, b3, a2) >= 0)    // IIa
                {
                    if (determinant(a1, b1, a2) >= 0) // IIIa
                    {
                        if (determinant(a1, b2, a2) > 0) // IVa
                            return false;
                        else
                            return true;
                    }
                    else
                    {
                        if (determinant(a1, b1, a3) > 0) // IVb
                            return false;
                        else
                        {
                            if (determinant(b3, b1, a3) > 0) // Va
                                return false;
                            else
                                return true;
                        }
                    }
                }
                else
                {
                    if (determinant(a1, b2, a2) > 0) // IIIb
                        return false;
                    else
                    {
                        if (determinant(b2, b3, a3) > 0) // IVc
                            return false;
                        else
                        {
                            if (determinant(a2, a3, b2) > 0) // Vb
                                return false;
                            else
                                return true;
                        }
                    }
                }
            }
            else
            {
                if (determinant(b3, b1, a3) > 0) // IIb
                    return false;
                else
                {
                    if (determinant(a2, a3, b3) >= 0) // IIIc
                    {
                        if (determinant(a3, a1, b1) > 0) // IVd
                            return false;
                        else
                            return true;
                    }
                    else
                    {
                        if (determinant(a2, a3, b2) > 0) // VIe
                            return false;
                        else
                        {
                            if (determinant(b2, b3, a3) > 0) // Vc
                                return false;
                            else
                                return true;
                        }
                    }
                }
            }
        }
    }
    else
    {
        if (r == 2) { swp = a1; a1 = a2; a2 = a3; a3 = swp; }
        else
        if (r == 3) { swp = a1; a1 = a3; a3 = a2; a2 = swp; }

        // Calculate plane (with CrossProduct)
        float p1x = a2->x - a1->x, p1y = a2->y - a1->y, p1z = a2->z - a1->z;
        float p2x = b1->x - a1->x, p2y = b1->y - a1->y, p2z = b1->z - a1->z;
        float X1 = p1y*p2z - p1z*p2y, Y1 = p1z*p2x - p1x*p2z, Z1 = p1x*p2y - p1y*p2x;
        float W1 = -(a1->x*X1 + a1->y*Y1 + a1->z*Z1);
        // Check positions (with DotProduct+W)
        float P1a = b2->x * X1 + b2->y * Y1 + b2->z * Z1 + W1;
        float P1b = b3->x * X1 + b3->y * Y1 + b3->z * Z1 + W1;
        // Calculate plane (with CrossProduct)
        p1x = a3->x - a1->x; p1y = a3->y - a1->y; p1z = a3->z - a1->z;
        float X2 = p1y*p2z - p1z*p2y, Y2 = p1z*p2x - p1x*p2z, Z2 = p1x*p2y - p1y*p2x;
        float W2 = -(a1->x*X2 + a1->y*Y2 + a1->z*Z2);
        // Check positions (with DotProduct+W)
        float P2a = b3->x * X2 + b3->y * Y2 + b3->z * Z2 + W2;
        float P2b = b2->x * X2 + b2->y * Y2 + b2->z * Z2 + W2;
/*
        xPlane plane; plane.init(xVector3::CrossProduct(*a2-*a1,*b1-*a1), *a1);
        float P1a = plane.distanceToPoint(*b2);
        float P1b = plane.distanceToPoint(*b3);
        plane.init(xVector3::CrossProduct(*a3-*a1,*b1-*a1), *a1);
        float P2a = plane.distanceToPoint(*b3);
        float P2b = plane.distanceToPoint(*b2);
*/
        int p1a = sgn(P1a); int p1b = sgn(P1b);
        int p2a = sgn(P2a); int p2b = sgn(P2b);
        return p1a*p2a < 0.f || p1b*p2b < 0.f;
    }
}

inline bool TestTriangleTriangle(const xTriangle &tr1, const xTriangle &tr2)
{
    return TestTriangleTriangle(tr1.P_A, tr1.P_B, tr1.P_C, tr2.P_A, tr2.P_B, tr2.P_C);
}

inline bool SegmentSegmentFixDistance(const xVector3 &N_fix, const xPoint3 &P_fix,
                                      const xPoint3 &P_a, const xPoint3 &P_b,
                                      xFLOAT &S_a_min, xFLOAT &S_b_min,
                                      xFLOAT &S_minmax, xVector3 &N_fix_min)
{
    xPlane PN_fix; PN_fix.init(N_fix, P_fix);
    xFLOAT S_a = PN_fix.distanceToPoint(P_a);
    xFLOAT S_b = PN_fix.distanceToPoint(P_b);
    xFLOAT S_max = max(S_a,S_b);

    if (S_max < S_minmax)
    {
        S_minmax = S_max;
        S_a_min = S_a;
        S_b_min = S_b;
        N_fix_min = N_fix;
        return true;
    }
    return false;
}

inline bool SegmentSegmentFixDistanceAbs(const xVector3 &N_fix, const xPoint3 &P_fix,
                                      const xPoint3 &P_a, const xPoint3 &P_b,
                                      xFLOAT &S_a_min, xFLOAT &S_b_min,
                                      xFLOAT &S_minmax, xVector3 &N_fix_min)
{
    xPlane PN_fix; PN_fix.init(N_fix, P_fix);
    xFLOAT S_a = fabs ( PN_fix.distanceToPoint(P_a) );
    xFLOAT S_b = fabs ( PN_fix.distanceToPoint(P_b) );
    xFLOAT S_max = max(S_a,S_b);

    if (S_max < S_minmax)
    {
        S_minmax = S_max;
        S_a_min = S_a;
        S_b_min = S_b;
        N_fix_min = N_fix;
        return true;
    }
    return false;
}


xDWORD CollideTriangleTriangle(const xPoint3 &P_a1, const xPoint3 &P_a2, const xPoint3 &P_a3,
                               const xPoint3 &P_b1, const xPoint3 &P_b2, const xPoint3 &P_b3,
                               const xIFigure3d &figure1, xDWORD ID_tri_1,
                               const xIFigure3d &figure2, xDWORD ID_tri_2,
                               CollisionSet &cset)
{
    int r = CheckPlanes(P_a1, P_a2, P_a3, P_b1, P_b2, P_b3);
    if (!r) return false;

    const xPoint3 *a1 = &P_a1, *a2 = &P_a2, *a3 = &P_a3;
    const xPoint3 *b1 = &P_b1, *b2 = &P_b2, *b3 = &P_b3;

    const xPoint3 *swp;
    if (r > 0)
    {
        if (r == 2) { swp = b1; b1 = b2; b2 = b3; b3 = swp; }
        else
        if (r == 3) { swp = b1; b1 = b3; b3 = b2; b2 = swp; }

        r = CheckPlanes(*b1, *b2, *b3, *a1, *a2, *a3);
        if (!r) return false;
    }

    if (r == -1) // coplanar
    {
        return false; // we don't treat 2d collisions as collisions, they just touch, but do not collide

        if (determinant(a1,a2,a3) < 0.f) { swp = a2; a2 = a3; a3 = swp; }
        if (determinant(b1,b2,b3) < 0.f) { swp = b2; b2 = b3; b3 = swp; }

        float P1 = determinant(a1, b1, b2);
        float P2 = determinant(a1, b2, b3);
        float P3 = determinant(a1, b3, b1);

        if (P1 >= 0.f && P2 >= 0.f && P3 >= 0.f) // inside, on the vertex, on the edge or "empty triangle"
            return true;

        while (determinant(a1, b1, b2) < 0 || determinant(a1, b1, b3) < 0)
        { swp = b1; b1 = b2; b2 = b3; b3 = swp; }

        if (determinant(a1, b2, b3) > 0) // Region R1
        {
            if (determinant(b3, b1, a2) >= 0)    // I
            {
                if (determinant(b3, a1, a2) < 0)  // IIa
                    return false;
                else
                {
                    if (determinant(a1, b1, a2) >= 0) // IIIa
                        return true;
                    else
                    {
                        if (determinant(a1, b1, a3) < 0) // IV a
                            return false;
                        else
                        {
                            if (determinant(a2, a3, b1) >= 0) // V
                                return true;
                            else
                                return false;
                        }
                    }
                }
            }
            else
            {
                if (determinant(b3, b1, a3) < 0) // IIb
                    return false;
                else
                {
                    if (determinant(a2, a3, b3) > 0) // IIIb
                        return false;
                    else
                    {
                        if (determinant(a1, b1, a3) > 0) // IVb
                            return false;
                        else
                            return true;
                    }
                }
            }
        }
        else                             // Region R2
        {
            if (determinant(b3, b1, a2) >= 0)        // I
            {
                if (determinant(b2, b3, a2) >= 0)    // IIa
                {
                    if (determinant(a1, b1, a2) >= 0) // IIIa
                    {
                        if (determinant(a1, b2, a2) > 0) // IVa
                            return false;
                        else
                            return true;
                    }
                    else
                    {
                        if (determinant(a1, b1, a3) > 0) // IVb
                            return false;
                        else
                        {
                            if (determinant(b3, b1, a3) > 0) // Va
                                return false;
                            else
                                return true;
                        }
                    }
                }
                else
                {
                    if (determinant(a1, b2, a2) > 0) // IIIb
                        return false;
                    else
                    {
                        if (determinant(b2, b3, a3) > 0) // IVc
                            return false;
                        else
                        {
                            if (determinant(a2, a3, b2) > 0) // Vb
                                return false;
                            else
                                return true;
                        }
                    }
                }
            }
            else
            {
                if (determinant(b3, b1, a3) > 0) // IIb
                    return false;
                else
                {
                    if (determinant(a2, a3, b3) >= 0) // IIIc
                    {
                        if (determinant(a3, a1, b1) > 0) // IVd
                            return false;
                        else
                            return true;
                    }
                    else
                    {
                        if (determinant(a2, a3, b2) > 0) // VIe
                            return false;
                        else
                        {
                            if (determinant(b2, b3, a3) > 0) // Vc
                                return false;
                            else
                                return true;
                        }
                    }
                }
            }
        }
    }

    if (r == 2) { swp = a1; a1 = a2; a2 = a3; a3 = swp; }
    else
    if (r == 3) { swp = a1; a1 = a3; a3 = a2; a2 = swp; }

    // Calculate plane (with CrossProduct)
    float p1x = a2->x - a1->x, p1y = a2->y - a1->y, p1z = a2->z - a1->z;
    float p2x = b1->x - a1->x, p2y = b1->y - a1->y, p2z = b1->z - a1->z;
    float X1 = p1y*p2z - p1z*p2y, Y1 = p1z*p2x - p1x*p2z, Z1 = p1x*p2y - p1y*p2x;
    float W1 = -(a1->x*X1 + a1->y*Y1 + a1->z*Z1);
    // Check positions (with DotProduct+W)
    float P1a = b2->x * X1 + b2->y * Y1 + b2->z * Z1 + W1;
    float P1b = b3->x * X1 + b3->y * Y1 + b3->z * Z1 + W1;
    // Calculate plane (with CrossProduct)
    p1x = a3->x - a1->x; p1y = a3->y - a1->y; p1z = a3->z - a1->z;
    float X2 = p1y*p2z - p1z*p2y, Y2 = p1z*p2x - p1x*p2z, Z2 = p1x*p2y - p1y*p2x;
    float W2 = -(a1->x*X2 + a1->y*Y2 + a1->z*Z2);
    // Check positions (with DotProduct+W)
    float P2a = b3->x * X2 + b3->y * Y2 + b3->z * Z2 + W2;
    float P2b = b2->x * X2 + b2->y * Y2 + b2->z * Z2 + W2;

    /*
    xPlane plane; plane.init(xVector3::CrossProduct(*a2-*a1,*b1-*a1), *a1);
    float P1a = plane.distanceToPoint(*b2);
    float P1b = plane.distanceToPoint(*b3);
    plane.init(xVector3::CrossProduct(*a3-*a1,*b1-*a1), *a1);
    float P2a = plane.distanceToPoint(*b3);
    float P2b = plane.distanceToPoint(*b2);
    */

    int p1a = sgn(P1a); int p1b = sgn(P1b);
    int p2a = sgn(P2a); int p2b = sgn(P2b);
    bool res = p1a*p2a < 0.f || p1b*p2b < 0.f;

    if (!res) return false;

    if (!p1a) p1a = -1;
    if (!p2a) p2a = -1;
    if (!p1b) p1b = -1;
    if (!p2b) p2b = -1;
    xPoint3 p, p1, p2;
    bool    FL_p1_used = false;

    if (p1a*p2b < 0.f || p2a*p1b < 0.f)
    {
        p2x = a2->x - a1->x, p2y = a2->y - a1->y, p2z = a2->z - a1->z;
        X2 = p1y*p2z - p1z*p2y, Y2 = p1z*p2x - p1x*p2z, Z2 = p1x*p2y - p1y*p2x;
        W2 = -(a1->x*X2 + a1->y*Y2 + a1->z*Z2);
        W2 = b1->x * X2 + b1->y * Y2 + b1->z * Z2 + W2;

        if (p1a*p2b < 0.f)
        {
            X1 = b2->x - b1->x; Y1 = b2->y - b1->y; Z1 = b2->z - b1->z;
            W1 = W2 / (X1 * X2 + Y1 * Y2 + Z1 * Z2);
            p1.init(b1->x - X1*W1, b1->y - Y1*W1, b1->z - Z1*W1);
            FL_p1_used = true;
        }
        if (p2a*p1b < 0.f)
        {
            X1 = b3->x - b1->x; Y1 = b3->y - b1->y; Z1 = b3->z - b1->z;
            W1 = W2 / (X1 * X2 + Y1 * Y2 + Z1 * Z2);
            if (FL_p1_used)
                p2.init(b1->x - X1*W1, b1->y - Y1*W1, b1->z - Z1*W1);
            else
                p1.init(b1->x - X1*W1, b1->y - Y1*W1, b1->z - Z1*W1);
            FL_p1_used = true;
        }
    }
    if (p1a*p1b < 0.f || p2a*p2b < 0.f)
    {
        p1x = b3->x - b1->x; p1y = b3->y - b1->y; p1z = b3->z - b1->z;
        p2x = b2->x - b1->x, p2y = b2->y - b1->y, p2z = b2->z - b1->z;
        X2 = p1y*p2z - p1z*p2y, Y2 = p1z*p2x - p1x*p2z, Z2 = p1x*p2y - p1y*p2x;
        W2 = -(b1->x*X2 + b1->y*Y2 + b1->z*Z2);
        W2 = a1->x * X2 + a1->y * Y2 + a1->z * Z2 + W2;

        if (p1a*p1b < 0.f)
        {
            X1 = a2->x - a1->x; Y1 = a2->y - a1->y; Z1 = a2->z - a1->z;
            W1 = W2 / (X1 * X2 + Y1 * Y2 + Z1 * Z2);
            if (FL_p1_used)
                p2.init(a1->x - X1*W1, a1->y - Y1*W1, a1->z - Z1*W1);
            else
                p1.init(a1->x - X1*W1, a1->y - Y1*W1, a1->z - Z1*W1);
            FL_p1_used = true;
        }
        if (p2a*p2b < 0.f)
        {
            X1 = a3->x - a1->x; Y1 = a3->y - a1->y; Z1 = a3->z - a1->z;
            W1 = W2 / (X1 * X2 + Y1 * Y2 + Z1 * Z2);
            p2.init(a1->x - X1*W1, a1->y - Y1*W1, a1->z - Z1*W1);
        }
    }

    xVector3 N_fix_min;
    xFLOAT   S_minmax = xFLOAT_HUGE_POSITIVE;
    xFLOAT   S_a_min, S_b_min;

    xVector3 NW_12   = P_a2 - P_a1;
    xVector3 NW_23   = P_a3 - P_a2;
    xVector3 NW_31   = P_a1 - P_a3;
    xVector3 NW_tri = xVector3::CrossProduct(NW_12, NW_23);

    if (SegmentSegmentFixDistance(xVector3::CrossProduct(NW_tri, NW_12).normalize(), P_a1,
                              p1, p2, S_a_min, S_b_min, S_minmax, N_fix_min))
        N_fix_min.invert();
    if (SegmentSegmentFixDistance(xVector3::CrossProduct(NW_tri, NW_23).normalize(), P_a2,
                              p1, p2, S_a_min, S_b_min, S_minmax, N_fix_min))
        N_fix_min.invert();
    if (SegmentSegmentFixDistance(xVector3::CrossProduct(NW_tri, NW_31).normalize(), P_a3,
                              p1, p2, S_a_min, S_b_min, S_minmax, N_fix_min))
        N_fix_min.invert();
    SegmentSegmentFixDistanceAbs(NW_tri.normalize(), P_b1,
                                 p1, p2, S_a_min, S_b_min, S_minmax, N_fix_min);

    NW_12   = P_b2 - P_b1;
    NW_23   = P_b3 - P_b2;
    NW_31   = P_b1 - P_b3;
    NW_tri = xVector3::CrossProduct(NW_12, NW_23);

    SegmentSegmentFixDistance(xVector3::CrossProduct(NW_tri, NW_12).normalize(), P_b1,
                              p1, p2, S_a_min, S_b_min, S_minmax, N_fix_min);
    SegmentSegmentFixDistance(xVector3::CrossProduct(NW_tri, NW_23).normalize(), P_b2,
                              p1, p2, S_a_min, S_b_min, S_minmax, N_fix_min);
    SegmentSegmentFixDistance(xVector3::CrossProduct(NW_tri, NW_31).normalize(), P_b3,
                              p1, p2, S_a_min, S_b_min, S_minmax, N_fix_min);
    if (SegmentSegmentFixDistanceAbs(NW_tri.normalize(), P_a1,
                                 p1, p2, S_a_min, S_b_min, S_minmax, N_fix_min))
        N_fix_min.invert();

    N_fix_min *= S_minmax;

    if (S_a_min > S_b_min)
        return cset.Add(CollisionInfo(figure1, figure2, ID_tri_1, ID_tri_2,
                                      N_fix_min, p1, p1 + N_fix_min)), 1;
    if (S_a_min < S_b_min || p1 == p2)
        return cset.Add(CollisionInfo(figure1, figure2, ID_tri_1, ID_tri_2,
                                      N_fix_min, p2, p2 + N_fix_min)), 1;

    cset.Add(CollisionInfo(figure1, figure2, ID_tri_1, ID_tri_2,
                                      N_fix_min, p1, p1 + N_fix_min));
    cset.Add(CollisionInfo(figure1, figure2, ID_tri_1, ID_tri_2,
                                      N_fix_min, p2, p2 + N_fix_min));
    return 2;
}


inline xDWORD CollideTriangleTriangle(const xTriangle &tr1, const xTriangle &tr2, CollisionSet &cset)
{
    return CollideTriangleTriangle(tr1.P_A, tr1.P_B, tr1.P_C, tr2.P_A, tr2.P_B, tr2.P_C, tr1, 0, tr2, 0, cset);
}