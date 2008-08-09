// Included in FigureCollider.cpp
// Depends on xCylinder.h

////////////////////////////// Cylinder - Cylinder

xFLOAT LineToLineDistance(xVector3 N_line1, xPoint3 P_line1, xVector3 N_line2, xPoint3 P_line2)
{
    xVector3 N_plane = xVector3::CrossProduct(N_line1, N_line2);
    return xPlane().init(N_plane, P_line1).distanceToPoint(P_line2);
}

/*
{
    if (figure->Type == xIFigure3d::Cylinder)
    {
        const xCylinder &object = *((xCylinder*) figure);

        xFLOAT   dN1N2      = xVector3::DotProduct(N_top, object.N_top);
        xVector3 NW_centers, N_side;
        bool     FL_parallel = false;
        if (dN1N2 >= 1.f - EPSILON || dN1N2 <= -1.f + EPSILON) // parallel
        {
            // TODO: Add case when axes of cyllinders overlap
            // TODO: verify, that side is not negative
            NW_centers = object.P_center - P_center;
            N_side = xVector3::CrossProduct(N_top, xVector3::CrossProduct(N_top, NW_centers)).normalize();
            FL_parallel = true;
        }
        else
            N_side = xVector3::CrossProduct(N_top, object.N_top).normalize();
        xPlane   PN_side; PN_side.planeFromNormalAndPoint(N_side, P_center);
        xFLOAT   S_distance = PN_side.planeTest(object.P_center);
        if (fabs(S_distance) > S_radius + object.S_radius)
            return xVector3::Create(0,0,0);

        if (FL_parallel)
        {
            xVector3 P_collision_1 = GetInnerRayCollisionPoint(P_center, NW_centers, N_side);
            xVector3 P_collision_2 = object.GetInnerRayCollisionPoint(object.P_center, -NW_centers, -N_side);
            return P_collision_2 - P_collision_1;
        }

        // Based on http://local.wasp.uwa.edu.au/~pbourke/geometry/lineline3d/
        //dnmpo = dot(nm, po);
        //mua = ( dN1NW12 * dN1N2 - dN2NW12 * dN1N1 ) / ( dN2N2 * dN1N1 - dN1N2 * dN1N2 ) // dN2N2 = dN1N1 = 1
        //mua = ( dN1NW12 * dN1N2 - dN2NW12 ) / ( 1 - dN1N2 * dN1N2 )
        //mub = ( dN1NW12 + mua * dN1N2 ) / dN1N1
        //mub = dN1NW12 + mua * dN1N2
        NW_centers = object.P_center - P_center;
        xFLOAT dN1NW12 = xVector3::DotProduct(N_top, NW_centers);
        xFLOAT mub = (dN1NW12 * dN1N2 - xVector3::DotProduct(object.N_top, NW_centers)) / (1.f - dN1N2*dN1N2);
        xFLOAT mua = dN1NW12 + mub * dN1N2;
        xFLOAT muaP = fabs(mua);
        xFLOAT mubP = fabs(mub);

        if (muaP > S_top + object.S_radius || mubP > object.S_top + S_radius)
            return xVector3::Create(0,0,0);

        xVector3 N_collision; N_collision.zero();

        if (muaP < S_top && mubP < object.S_top)
            return N_side * ( S_distance - S_radius - object.S_radius );

        xVector3 P_line_1 = P_center + mua * N_top;
        xVector3 P_line_2 = object.P_center + mub * object.N_top;

        xVector3 P_closest_1, P_closest_2;

        if (muaP > S_top && mubP <= object.S_top)
        {
            xVector3 P_capCenter;
            if (mua > 0)
            {
                P_capCenter = P_center + S_top * N_top;
                PN_side.planeFromNormalAndPoint(N_top, P_capCenter);
            }
            else
            {
                P_capCenter = P_center - S_top * N_top;
                PN_side.planeFromNormalAndPoint(-N_top, P_capCenter);
            }

            P_closest_1 = PN_side.planeCastPoint(P_line_2);
            if ((P_closest_1 - P_capCenter).lengthSqr() > S_radius * S_radius)
                P_closest_1 = P_capCenter + S_radius * N_side;

            P_closest_2 = P_line_2 - object.S_radius * N_side;
        }
        else
        if (mubP > object.S_top && muaP <= S_top)
        {
            xVector3 P_capCenter;
            if (mua > 0)
            {
                P_capCenter = object.P_center + object.S_top * object.N_top;
                PN_side.planeFromNormalAndPoint(object.N_top, P_capCenter);
            }
            else
            {
                P_capCenter = object.P_center - object.S_top * object.N_top;
                PN_side.planeFromNormalAndPoint(-object.N_top, P_capCenter);
            }

            P_closest_2 = PN_side.planeCastPoint(P_line_1);
            if ((P_closest_2 - P_capCenter).lengthSqr() > object.S_radius * object.S_radius)
                P_closest_2 = P_capCenter - object.S_radius * N_side;

            P_closest_1 = P_line_1 + S_radius * N_side;
        }
        else
        {
            P_closest_2 = P_line_2 - object.S_radius * N_side;
        }


        return xVector3::Create(0,0,0);
    }

    return xVector3::Create(0,0,0);
}
*/
