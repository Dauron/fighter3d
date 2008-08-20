// Included in FigureCollider.cpp
// Depends on xMesh.h
// Depends on xTriangle.h

////////////////////////////// Triangle - Mesh

bool   TestTriangleMesh (const xPoint3 &P_A, const xPoint3 &P_B, const xPoint3 &P_C, const xMesh &ms2)
{
    xDWORD *L_FaceIndex_Itr = ms2.L_FaceIndices;
    if (ms2.MeshData->L_VertexData_Transf)
        for (int i = ms2.I_FaceIndices; i; --i, ++L_FaceIndex_Itr)
        {
            const xWORD3 &Face = ms2.MeshData->GetFace(*L_FaceIndex_Itr);
            if ( TestTriangleTriangle(P_A, P_B, P_C,
                                      ms2.MeshData->GetVertexTransf(Face[0]),
                                      ms2.MeshData->GetVertexTransf(Face[1]),
                                      ms2.MeshData->GetVertexTransf(Face[2])) )
                return true;
        }
    else
        for (int i = ms2.I_FaceIndices; i; --i, ++L_FaceIndex_Itr)
        {
            const xWORD3 &Face = ms2.MeshData->GetFace(*L_FaceIndex_Itr);
            if ( TestTriangleTriangle(P_A, P_B, P_C,
                                      ms2.MeshData->GetVertex(Face[0]),
                                      ms2.MeshData->GetVertex(Face[1]),
                                      ms2.MeshData->GetVertex(Face[2])) )
                return true;
        }
    return false;
}

inline bool   TestTriangleMesh (const xTriangle &tr1, const xMesh &ms2)
{
    return TestTriangleMesh(tr1.P_A, tr1.P_B, tr1.P_C, ms2);
}

xDWORD CollideTriangleMesh (const xPoint3 &P_A, const xPoint3 &P_B, const xPoint3 &P_C, const xMesh &ms2,
                            IPhysicalBody *body1, IPhysicalBody *body2,
                            const xIFigure3d &figure1, xDWORD ID_tri_1, CollisionSet &cset)
{
    xDWORD *L_FaceIndex_Itr = ms2.L_FaceIndices;
    xDWORD  I_cols = 0;
    if (ms2.MeshData->L_VertexData_Transf)
        for (int i = ms2.I_FaceIndices; i; --i, ++L_FaceIndex_Itr)
        {
            const xWORD3 &Face = ms2.MeshData->GetFace(*L_FaceIndex_Itr);
            I_cols += CollideTriangleTriangle(P_A, P_B, P_C,
                                              ms2.MeshData->GetVertexTransf(Face[0]),
                                              ms2.MeshData->GetVertexTransf(Face[1]),
                                              ms2.MeshData->GetVertexTransf(Face[2]),
                                              body1, body2,
                                              figure1, ID_tri_1, ms2, *L_FaceIndex_Itr, cset);
        }
    else
        for (int i = ms2.I_FaceIndices; i; --i, ++L_FaceIndex_Itr)
        {
            const xWORD3 &Face = ms2.MeshData->GetFace(*L_FaceIndex_Itr);
            I_cols += CollideTriangleTriangle(P_A, P_B, P_C,
                                              ms2.MeshData->GetVertex(Face[0]),
                                              ms2.MeshData->GetVertex(Face[1]),
                                              ms2.MeshData->GetVertex(Face[2]),
                                              body1, body2,
                                              figure1, ID_tri_1, ms2, *L_FaceIndex_Itr, cset);
        }
    return I_cols;
}

inline xDWORD CollideTriangleMesh (const xTriangle &tr1, const xMesh &ms2,
                                   IPhysicalBody *body1, IPhysicalBody *body2, CollisionSet &cset)
{
    return CollideTriangleMesh(tr1.P_A, tr1.P_B, tr1.P_C, ms2, body1, body2, tr1, 0, cset);
}