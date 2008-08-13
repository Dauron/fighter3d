// Included in FigureCollider.cpp
// Depends on xSegment.h
// Depends on xCapsule.h

////////////////////////////// Segment - Capsule

bool SegmentCapsuleMinMax(const xPoint3 &P_A, const xPoint3 &P_B, const xCapsule &cp2,
                          xPoint3 &P_min, xPoint3 &P_max, bool fast = true)
{
    xVector3 NW_AB = P_B - P_A;

    xPoint3 P_close_sg1;
    xPoint3 P_close_cp2;
    xSegment::DistanceLineLine(NW_AB, P_A, cp2.N_top, cp2.P_center, P_close_sg1, P_close_cp2);
    xVector3 NW_close = P_close_sg1 - P_close_cp2;
    xFLOAT   S_distance = NW_close.lengthSqr();
    xFLOAT   S_r_sqr    = cp2.S_radius * cp2.S_radius;
    if (S_distance > S_r_sqr + 0.01f ) return false;
    xFLOAT   S_AB = NW_AB.length();
    xVector3 N_AB = NW_AB / S_AB;

    if (S_distance > S_r_sqr - EPSILON2)
        P_min = P_max = P_close_sg1;
    else
    {
        xVector3 N_fix;
        if (S_distance < EPSILON)
        {
            S_distance = cp2.S_radius;
            N_fix = xVector3::CrossProduct(xVector3::CrossProduct(NW_AB, cp2.N_top), cp2.N_top).normalize();
        }
        else
        {
            S_distance = sqrt (S_r_sqr - S_distance);
            N_fix = xVector3::CrossProduct(NW_close, cp2.N_top).normalize();
        }

        xFLOAT W_cosFixSG1 = fabs( xVector3::DotProduct(N_fix, N_AB) );
        S_distance = S_distance / W_cosFixSG1;

        N_fix = N_AB * S_distance;
        P_min = P_close_sg1 - N_fix;
        P_max = P_close_sg1 + N_fix;
    }

    xFLOAT P_min_cast = xVector3::DotProduct(P_min - cp2.P_center, cp2.N_top);
    xFLOAT P_max_cast = xVector3::DotProduct(P_max - cp2.P_center, cp2.N_top);

    if (P_min_cast >= -cp2.S_top && P_min_cast <= cp2.S_top &&
        P_max_cast >= -cp2.S_top && P_max_cast <= cp2.S_top)
    {
        P_min_cast = xVector3::DotProduct(P_min - P_A, N_AB);
        if (P_min_cast > S_AB) return false;
        P_max_cast = xVector3::DotProduct(P_max - P_A, N_AB);
        if (P_max_cast < 0.f)  return false;
        if (P_min_cast < 0.f)  P_min = P_A;
        if (P_max_cast > S_AB) P_max = P_B;
        if (P_max_cast + EPSILON2 < P_min_cast) return false;
        if (P_max_cast < P_min_cast)
            P_max = P_min = (P_max + P_min) * 0.5f;
        return true;
    }

    xVector3 P_cap;

    if (P_min_cast < -cp2.S_top || P_min_cast > cp2.S_top)
    {
        if (P_min_cast < -cp2.S_top)
            P_cap = cp2.P_center - cp2.S_top * cp2.N_top;
        else
            P_cap = cp2.P_center + cp2.S_top * cp2.N_top;

        xVector3 NW_a      = P_cap - P_min;
        xFLOAT   W_cos_a_n = xVector3::DotProduct(N_AB, NW_a);
        if (W_cos_a_n < 0) return false;

        xFLOAT   S_a_sqr = NW_a.lengthSqr();
        xFLOAT   S_a     = sqrt(S_a_sqr);
        W_cos_a_n /= S_a;

        // R*R = a*a + d*d - 2*a*d*cosB
        // d*d - 2*a*cosB*d + a*a - R*R = 0
        // d*d - v*d + u = 0
        // v = 2*a*cosB
        // u = a*a - R*R
        // delta = v*v - 4*u
        // d1 = (v - sqrt(delta)) * 0.5
        // d1 = (v + sqrt(delta)) * 0.5
        xFLOAT u = S_a_sqr - S_r_sqr;
        xFLOAT v = 2 * S_a * W_cos_a_n;
        xFLOAT delta = v*v - 4*u;
        if (delta < -0.01f) return false;
        if (delta <= 0.f)
            S_distance = v * 0.5f;
        else
            S_distance = (v - sqrt(delta)) * 0.5f;
        P_min += N_AB * S_distance;
    }

    if (P_max_cast < -cp2.S_top || P_max_cast > cp2.S_top)
    {
        if (P_max_cast < -cp2.S_top)
            P_cap = cp2.P_center - cp2.S_top * cp2.N_top;
        else
            P_cap = cp2.P_center + cp2.S_top * cp2.N_top;

        xVector3 NW_a      = P_max - P_cap;
        xFLOAT   W_cos_a_n = xVector3::DotProduct(N_AB, NW_a);
        if (W_cos_a_n < 0) return false;

        xFLOAT   S_a_sqr = NW_a.lengthSqr();
        xFLOAT   S_a     = sqrt(S_a_sqr);
        W_cos_a_n /= S_a;

        // R*R = a*a + d*d - 2*a*d*cosB
        // d*d - 2*a*cosB*d + a*a - R*R = 0
        // d*d - v*d + u = 0
        // v = 2*a*cosB
        // u = a*a - R*R
        // delta = v*v - 4*u
        // d1 = (v - sqrt(delta)) * 0.5
        // d1 = (v + sqrt(delta)) * 0.5
        xFLOAT u = S_a_sqr - S_r_sqr;
        xFLOAT v = 2 * S_a * W_cos_a_n;
        xFLOAT delta = v*v - 4*u;
        if (delta < -0.01f) return false;
        if (delta <= 0.f)
            S_distance = v * 0.5f;
        else
            S_distance = (v - sqrt(delta)) * 0.5f;
        P_max -= N_AB * S_distance;
    }

    P_min_cast = xVector3::DotProduct(P_min - P_A, N_AB);
    if (P_min_cast > S_AB) return false;
    P_max_cast = xVector3::DotProduct(P_max - P_A, N_AB);
    if (P_max_cast < 0.f)  return false;
    if (P_min_cast < 0.f)  P_min = P_A;
    if (P_max_cast > S_AB) P_max = P_B;
    if (P_max_cast + EPSILON2 < P_min_cast) return false;
    if (P_max_cast < P_min_cast)
        P_max = P_min = (P_max + P_min) * 0.5f;
    return true;
}

bool SegmentCapsuleMinMax(const xSegment &sg1, const xCapsule &cp2,
                       xPoint3 &P_min, xPoint3 &P_max, bool fast = true)
{
    xPoint3 P_close_sg1;
    xPoint3 P_close_cp2;
    xSegment::DistanceLineLine(sg1.N_dir, sg1.P_center, cp2.N_top, cp2.P_center, P_close_sg1, P_close_cp2);
    xVector3 NW_close = P_close_sg1 - P_close_cp2;
    xFLOAT   S_distance = NW_close.lengthSqr();
    xFLOAT   S_r_sqr    = cp2.S_radius * cp2.S_radius;
    if (S_distance > S_r_sqr + 0.01f ) return false;
    if (S_distance > S_r_sqr - EPSILON2)
    {
        xFLOAT P_cast = xVector3::DotProduct(P_close_sg1 - sg1.P_center, sg1.N_dir);
        if (P_cast >  sg1.S_dir) return false;
        if (P_cast < -sg1.S_dir) return false;
        if (P_cast < -sg1.S_dir) P_close_sg1 = sg1.P_center - sg1.N_dir*sg1.S_dir;
        if (P_cast >  sg1.S_dir) P_close_sg1 = sg1.P_center + sg1.N_dir*sg1.S_dir;
        P_min = P_max = P_close_sg1;
        return true;
    }

    xVector3 N_fix;
    if (S_distance < EPSILON)
    {
        S_distance = cp2.S_radius;
        N_fix = xVector3::CrossProduct(xVector3::CrossProduct(sg1.N_dir, cp2.N_top), cp2.N_top).normalize();
    }
    else
    {
        S_distance = sqrt (S_r_sqr - S_distance);
        N_fix = xVector3::CrossProduct(NW_close, cp2.N_top).normalize();
    }

    xFLOAT W_cosFixSG1 = fabs( xVector3::DotProduct(N_fix, sg1.N_dir) );
    S_distance = S_distance / W_cosFixSG1;

    N_fix = sg1.N_dir * S_distance;
    P_min = P_close_sg1 - N_fix;
    P_max = P_close_sg1 + N_fix;

    xFLOAT P_min_cast = xVector3::DotProduct(P_min - cp2.P_center, cp2.N_top);
    xFLOAT P_max_cast = xVector3::DotProduct(P_max - cp2.P_center, cp2.N_top);

    if (P_min_cast >= -cp2.S_top && P_min_cast <= cp2.S_top &&
        P_max_cast >= -cp2.S_top && P_max_cast <= cp2.S_top)
    {
        P_min_cast = xVector3::DotProduct(P_min - sg1.P_center, sg1.N_dir);
        if (P_min_cast >  sg1.S_dir) return false;
        P_max_cast = xVector3::DotProduct(P_max - sg1.P_center, sg1.N_dir);
        if (P_max_cast < -sg1.S_dir) return false;
        if (P_min_cast < -sg1.S_dir) P_min = sg1.P_center - sg1.N_dir*sg1.S_dir;
        if (P_max_cast >  sg1.S_dir) P_max = sg1.P_center + sg1.N_dir*sg1.S_dir;
        if (P_max_cast + EPSILON2 < P_min_cast) return false;
        if (P_max_cast < P_min_cast)
            P_max = P_min = (P_max + P_min) * 0.5f;
        return true;
    }

    xVector3 P_cap;

    if (P_min_cast < -cp2.S_top || P_min_cast > cp2.S_top)
    {
        if (P_min_cast < -cp2.S_top)
            P_cap = cp2.P_center - cp2.S_top * cp2.N_top;
        else
            P_cap = cp2.P_center + cp2.S_top * cp2.N_top;

        xVector3 NW_a      = P_cap - P_min;
        xFLOAT   W_cos_a_n = xVector3::DotProduct(sg1.N_dir, NW_a);
        if (W_cos_a_n < 0) return false;

        xFLOAT   S_a_sqr = NW_a.lengthSqr();
        xFLOAT   S_a     = sqrt(S_a_sqr);
        W_cos_a_n /= S_a;

        // c*c - v*c + u = 0
        xFLOAT u = S_a_sqr - S_r_sqr;
        xFLOAT v = 2 * S_a * W_cos_a_n;
        xFLOAT d = v*v - 4*u;
        if (d < 0) return false;

        S_distance = (v + sqrt(d)) * 0.5f;
        P_min += sg1.N_dir * S_distance;
    }

    if (P_max_cast < -cp2.S_top || P_max_cast > cp2.S_top)
    {
        if (P_max_cast < -cp2.S_top)
            P_cap = cp2.P_center - cp2.S_top * cp2.N_top;
        else
            P_cap = cp2.P_center + cp2.S_top * cp2.N_top;

        xVector3 NW_a      = P_max - P_cap;
        xFLOAT   W_cos_a_n = xVector3::DotProduct(sg1.N_dir, NW_a);
        if (W_cos_a_n < 0) return false;

        xFLOAT   S_a_sqr = NW_a.lengthSqr();
        xFLOAT   S_a     = sqrt(S_a_sqr);
        W_cos_a_n /= S_a;

        // c*c - v*c + u = 0
        xFLOAT u = S_a_sqr - S_r_sqr;
        xFLOAT v = 2 * S_a * W_cos_a_n;
        xFLOAT d = v*v - 4*u;
        if (d < 0) return false;

        S_distance = (v + sqrt(d)) * 0.5f;
        P_max -= sg1.N_dir * S_distance;
    }

    P_min_cast = xVector3::DotProduct(P_min - sg1.P_center, sg1.N_dir);
    if (P_min_cast >  sg1.S_dir) return false;
    P_max_cast = xVector3::DotProduct(P_max - sg1.P_center, sg1.N_dir);
    if (P_max_cast < -sg1.S_dir) return false;
    if (P_min_cast < -sg1.S_dir) P_min = sg1.P_center - sg1.N_dir*sg1.S_dir;
    if (P_max_cast >  sg1.S_dir) P_max = sg1.P_center + sg1.N_dir*sg1.S_dir;
    if (P_max_cast + EPSILON2 < P_min_cast) return false;
    if (P_max_cast < P_min_cast)
        P_max = P_min = (P_max + P_min) * 0.5f;
    return true;
}
