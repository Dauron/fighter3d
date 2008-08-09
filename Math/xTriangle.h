// Included in xMath.h

struct xTriange {

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
};
