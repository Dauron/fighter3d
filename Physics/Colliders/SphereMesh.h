// Included in FigureCollider.cpp
// Depends on xSphere.h
// Depends on xMesh.h

////////////////////////////// Sphere - Mesh

inline bool   TestSphereTriangle (const xSphere &sp1, const xPoint3 &P_A,
                                  const xPoint3 &P_B, const xPoint3 &P_C)
{
    xVector3 NW_AB = P_B - P_A;
    xVector3 NW_AC = P_C - P_A;
    xVector3 N_tri = xVector3::CrossProduct(NW_AB, NW_AC).normalize();

    xPlane PN_tri; PN_tri.init(N_tri, P_A);
    xFLOAT S_distance = PN_tri.distanceToPoint(sp1.P_center);
    if (S_distance*S_distance + EPSILON2 > sp1.S_radius*sp1.S_radius) return false;

    xPoint3       P_cast = sp1.P_center - N_tri * S_distance; // PN_tri.castPoint(sp1.P_center);
    xPoint3       P_closest;

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
            P_closest = P_A + NW_AB * (W_ABP / NW_AB.lengthSqr());
    }
    else
    {
        // Should be on the left of the AC vector
        xVector3 NW_side = xVector3::CrossProduct(NW_AC, NW_AP);
        // Outside edge AC?
        if (xVector3::DotProduct(N_tri, NW_side) > 0)
        {
            xFLOAT W_ACP = xVector3::DotProduct(NW_AC, NW_AP);
            // Near corner A?
            if (W_ACP < 0)
                P_closest = P_A;
            else
            // Near corner C?
            if (W_ACP > NW_AC.lengthSqr())
                P_closest = P_C;
            // Near edge
            else
                P_closest = P_A + NW_AC * (W_ACP / NW_AC.lengthSqr());
        }
        else
        {
            // Should be on the right of the BC vector
            xVector3 NW_BC = P_C - P_B;
            xVector3 NW_BP = P_cast - P_B;
            NW_side = xVector3::CrossProduct(NW_BC, NW_BP);
            // Outside edge BC?
            if (xVector3::DotProduct(N_tri, NW_side) < 0)
            {
                xFLOAT W_BCP = xVector3::DotProduct(NW_BC, NW_BP);
                // Near corner B? - tested for AB edge
                if (W_BCP < 0)
                    P_closest = P_A;
                else
                // Near corner B? - tested for AC edge
                if (W_BCP > NW_AC.lengthSqr())
                    P_closest = P_C;
                // Near edge
                else
                  P_closest = P_B + NW_BC * (W_BCP / NW_BC.lengthSqr());
            }
            // Inside the triangle
            else
                P_closest = P_cast;
        }
    }

    xVector3 NW_fix = sp1.P_center - P_closest;
    S_distance      = NW_fix.lengthSqr();
    if (S_distance + EPSILON2 > sp1.S_radius*sp1.S_radius) return false;
    return true;
}

bool          TestSphereMesh (const xSphere &sp1, const xMesh &ms2)
{
    xDWORD *L_FaceIndex_Itr = ms2.L_FaceIndices;
    if (ms2.MeshData->L_VertexData_Transf)
        for (int i = ms2.I_FaceIndices; i; --i, ++L_FaceIndex_Itr)
        {
            const xWORD3 &Face = ms2.MeshData->GetFace(*L_FaceIndex_Itr);
            if ( TestSphereTriangle(sp1,
                                    ms2.MeshData->GetVertexTransf(Face[0]),
                                    ms2.MeshData->GetVertexTransf(Face[1]),
                                    ms2.MeshData->GetVertexTransf(Face[2])) )
                return true;
        }
    else
        for (int i = ms2.I_FaceIndices; i; --i, ++L_FaceIndex_Itr)
        {
            const xWORD3 &Face = ms2.MeshData->GetFace(*L_FaceIndex_Itr);
            if ( TestSphereTriangle(sp1,
                                    ms2.MeshData->GetVertex(Face[0]),
                                    ms2.MeshData->GetVertex(Face[1]),
                                    ms2.MeshData->GetVertex(Face[2])) )
                return true;
        }
    return false;
}

inline CollisionInfo CollideSphereTriangle (const xSphere &sp1, const xPoint3 &P_A,
                                            const xPoint3 &P_B, const xPoint3 &P_C)
{
    xVector3 NW_AB = P_B - P_A;
    xVector3 NW_AC = P_C - P_A;
    xVector3 N_tri = xVector3::CrossProduct(NW_AB, NW_AC).normalize();

    xPlane PN_tri; PN_tri.init(N_tri, P_A);
    xFLOAT S_distance = PN_tri.distanceToPoint(sp1.P_center);
    if (S_distance*S_distance + EPSILON2 > sp1.S_radius*sp1.S_radius) return CollisionInfo(false);

    xPoint3       P_cast = sp1.P_center - N_tri * S_distance; // PN_tri.castPoint(sp1.P_center);
    xPoint3       P_closest;
    xVector3      NW_BC;
    PointPosition Position = POINT_NearCorner;
    xBYTE         edge_idx;

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
            edge_idx  = 1;
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
                edge_idx  = 2;
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
                    P_closest = P_A;
                else
                // Near corner B? - tested for AC edge
                if (W_BCP * W_BCP > NW_AC.lengthSqr())
                    P_closest = P_C;
                // Near edge
                else
                {
                    P_closest = P_B + NW_BC * (W_BCP / NW_BC.lengthSqr());
                    Position  = POINT_NearEdge;
                    edge_idx  = 3;
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

    xVector3 NW_fix = sp1.P_center - P_closest;
    S_distance      = NW_fix.lengthSqr();
    if (S_distance + EPSILON2 > sp1.S_radius*sp1.S_radius) return CollisionInfo(false);

    if (S_distance > EPSILON)
    {
        S_distance = sqrt(S_distance);
        NW_fix /= S_distance;
        return CollisionInfo(NW_fix * (sp1.S_radius - S_distance),
                             sp1.P_center - NW_fix * sp1.S_radius,
                             P_closest);
    }

    if (Position == POINT_Inside)
        return CollisionInfo(N_tri * sp1.S_radius,
                             sp1.P_center - N_tri * sp1.S_radius,
                             P_closest);

    if (Position == POINT_NearCorner)
    {
        xPoint3 P_tri_center = (P_A + P_B + P_C) * 0.3333333333333333f;
        NW_fix = (P_closest - P_tri_center).normalize();
        return CollisionInfo(NW_fix * sp1.S_radius,
                             sp1.P_center - NW_fix * sp1.S_radius,
                             P_closest);
    }

    //if (Position == POINT_NearEdge)
    {
        if (edge_idx == 1) NW_fix = xVector3::CrossProduct(NW_AB, N_tri).normalize();  // normalized at previous stage
        else
        if (edge_idx == 2) NW_fix = -xVector3::CrossProduct(NW_AC, N_tri).normalize(); // normalized at previous stage
        else
        if (edge_idx == 3) NW_fix = xVector3::CrossProduct(NW_BC, N_tri).normalize();  // normalized at previous stage
        return CollisionInfo(NW_fix * sp1.S_radius,
                             sp1.P_center - NW_fix * sp1.S_radius,
                             P_closest);
    }
}

CollisionInfo CollideSphereMesh (const xSphere &sp1, const xMesh &ms2)
{
    xVector3 NW_Max; NW_Max.init(EPSILON,EPSILON,EPSILON);
    xDWORD   I_MaxX = 0;
    xDWORD   I_MaxY = 0;
    xDWORD   I_MaxZ = 0;
    xVector3 NW_Min; NW_Min.init(-EPSILON,-EPSILON,-EPSILON);
    xDWORD   I_MinX = 0;
    xDWORD   I_MinY = 0;
    xDWORD   I_MinZ = 0;
    xDWORD   I_cols = 0;
    CollisionInfo cinfo(false);

    xDWORD *L_FaceIndex_Itr = ms2.L_FaceIndices;
    if (ms2.MeshData->L_VertexData_Transf)
        for (int i = ms2.I_FaceIndices; i; --i, ++L_FaceIndex_Itr)
        {
            const xWORD3 &Face = ms2.MeshData->GetFace(*L_FaceIndex_Itr);
            CollisionInfo ci = CollideSphereTriangle(sp1,
                                                     ms2.MeshData->GetVertexTransf(Face[0]),
                                                     ms2.MeshData->GetVertexTransf(Face[1]),
                                                     ms2.MeshData->GetVertexTransf(Face[2]));

            if (ci.FL_collided)
            {
                ++I_cols;
                if (I_cols == 1) cinfo = ci;

                if (ci.NW_fix_1.x > NW_Max.x) { NW_Max.x = ci.NW_fix_1.x; ++I_MaxX; }
                else
                if (ci.NW_fix_1.x < NW_Min.x) { NW_Min.x = ci.NW_fix_1.x; ++I_MinX; }

                if (ci.NW_fix_1.y > NW_Max.y) { NW_Max.y = ci.NW_fix_1.y; ++I_MaxY; }
                else
                if (ci.NW_fix_1.y < NW_Min.y) { NW_Min.y = ci.NW_fix_1.y; ++I_MinY; }

                if (ci.NW_fix_1.z > NW_Max.z) { NW_Max.z = ci.NW_fix_1.z; ++I_MaxZ; }
                else
                if (ci.NW_fix_1.z < NW_Min.z) { NW_Min.z = ci.NW_fix_1.z; ++I_MinZ; }
            }
        }
    else
        for (int i = ms2.I_FaceIndices; i; --i, ++L_FaceIndex_Itr)
        {
            const xWORD3 &Face = ms2.MeshData->GetFace(*L_FaceIndex_Itr);
            CollisionInfo ci = CollideSphereTriangle(sp1,
                                                     ms2.MeshData->GetVertex(Face[0]),
                                                     ms2.MeshData->GetVertex(Face[1]),
                                                     ms2.MeshData->GetVertex(Face[2]));

            if (ci.FL_collided)
            {
                ++I_cols;
                if (I_cols == 1) cinfo = ci;

                if (ci.NW_fix_1.x > NW_Max.x) { NW_Max.x = ci.NW_fix_1.x; ++I_MaxX; }
                else
                if (ci.NW_fix_1.x < NW_Min.x) { NW_Min.x = ci.NW_fix_1.x; ++I_MinX; }

                if (ci.NW_fix_1.y > NW_Max.y) { NW_Max.y = ci.NW_fix_1.y; ++I_MaxY; }
                else
                if (ci.NW_fix_1.y < NW_Min.y) { NW_Min.y = ci.NW_fix_1.y; ++I_MinY; }

                if (ci.NW_fix_1.z > NW_Max.z) { NW_Max.z = ci.NW_fix_1.z; ++I_MaxZ; }
                else
                if (ci.NW_fix_1.z < NW_Min.z) { NW_Min.z = ci.NW_fix_1.z; ++I_MinZ; }
            }
        }

    if (I_cols <= 1) return cinfo;

    xFLOAT W_max = I_MaxX / ((xFLOAT) I_MaxX + I_MinX);
    cinfo.NW_fix_1.x = NW_Max.x * W_max + NW_Min.x * (1.f - W_max);
    W_max = I_MaxY / ((xFLOAT) I_MaxY + I_MinY);
    cinfo.NW_fix_1.y = NW_Max.y * W_max + NW_Min.y * (1.f - W_max);
    W_max = I_MaxZ / ((xFLOAT) I_MaxZ + I_MinZ);
    cinfo.NW_fix_1.z = NW_Max.z * W_max + NW_Min.z * (1.f - W_max);

    xVector3 N_fix = xVector3::Normalize(cinfo.NW_fix_1);
    cinfo.P_collision_1 = sp1.P_center - N_fix * sp1.S_radius;
    cinfo.P_collision_2 = cinfo.P_collision_1 + cinfo.NW_fix_1;
    return cinfo;
}
