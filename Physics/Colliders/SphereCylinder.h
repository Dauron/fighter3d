// Included in FigureCollider.cpp
// Depends on xSphere.h
// Depends on xCylinder.h

////////////////////////////// Sphere - Cylinder

/*
{
    if (figure->Type == xIFigure3d::Cylinder)
    {
        const xCylinder &object = *((xCylinder*) figure);

        xVector3 NW_top = object.N_top * object.S_top;
        xPlane NW_wall; NW_wall.planeFromNormalAndPoint(object.N_top, object.P_center + NW_top);
        xFLOAT S_distFromTop = NW_wall.planeTest(P_center);
        if (S_distFromTop >= S_radius) return xVector3::Create(0,0,0);

        xVector3 NW_collision; NW_collision.zero();
        xFLOAT S_top_closer = 0.f;
        xBYTE  W_top_dir    = 0;

        if (S_distFromTop < 0.f)  // test bottom side, maybe it's fix is closer
        {
            NW_wall.planeFromNormalAndPoint(-object.N_top, object.P_center - NW_top);
            xFLOAT S_distFromBottom = NW_wall.planeTest(P_center);
            if (S_distFromBottom >= S_radius) return xVector3::Create(0,0,0);

            if (S_distFromBottom >= 0.f)
                NW_collision -= NW_top;
            else
            if (S_distFromTop >= S_distFromBottom)
            { S_top_closer = -S_distFromTop; W_top_dir = 1; }
            else
            { S_top_closer = -S_distFromBottom; W_top_dir = -1; }
        }
        else
            NW_collision += NW_top;

        xVector3 NW_centers = P_center - object.P_center;
        xFLOAT   S_centers  = NW_centers.length();
        xVector3 N_centers  = NW_centers / S_centers;
        // TODO: verify, that side is not negative
        xVector3 N_side     = xVector3::CrossProduct(object.N_top, xVector3::CrossProduct(object.N_top, N_centers)).normalize();
        xVector3 NW_side    = N_side * object.S_radius;
        NW_wall.planeFromNormalAndPoint(N_side, object.P_center + NW_side);
        xFLOAT S_distFromSide = NW_wall.planeTest(P_center);
        if (S_distFromSide >= S_radius) return xVector3::Create(0,0,0);

        bool FL_inside = false;

        if (S_distFromSide >= 0.f)
            NW_collision += NW_side;

        else if (W_top_dir != 0)
        {
            FL_inside = true;

            S_distFromSide = -S_distFromSide;

            xFLOAT S_diagonal = S_radius * 2;
            bool FL_whole_inside = true;

            if (S_diagonal > S_top_closer)
            {
                NW_collision += W_top_dir * NW_top;
                FL_whole_inside = false;
            }
            if (S_diagonal > S_distFromSide)
            {
                NW_collision += NW_side;
                FL_whole_inside = false;
            }

            if (FL_whole_inside)
                NW_collision = (S_top_closer < S_distFromSide) ? W_top_dir * NW_top : NW_side;
        }

        xPlane PN_collision; PN_collision.planeFromNormalAndPoint(xVector3::Normalize(NW_collision), object.P_center+NW_collision);
        xVector3 P_collision = PN_collision.planeCastPoint(P_center);
        xVector3 NW_center = P_center - P_collision;
        xFLOAT   S_center = NW_center.length();

        if (FL_inside)
            return -NW_center * (S_radius / S_center + 1.f);
        return NW_center * (S_radius / S_center - 1.f);
    }

    return xVector3::Create(0,0,0);
}
*/
