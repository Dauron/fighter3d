// Included in FigureCollider.cpp
// Depends on xBoxO.h
// Depends on xMesh.h

////////////////////////////// BoxO - Mesh

bool   TestBoxOMesh (const xBoxO &bo1, const xMesh &ms2)
{
    xDWORD *L_FaceIndex_Itr = ms2.L_FaceIndices;
    if (ms2.MeshData->L_VertexData_Transf)
        for (int i = ms2.I_FaceIndices; i; --i, ++L_FaceIndex_Itr)
        {
            const xWORD3 &Face = ms2.MeshData->GetFace(*L_FaceIndex_Itr);
            if ( TestBoxOTriangle(bo1,
                                  ms2.MeshData->GetVertexTransf(Face[0]),
                                  ms2.MeshData->GetVertexTransf(Face[1]),
                                  ms2.MeshData->GetVertexTransf(Face[2])) )
                return true;
        }
    else
        for (int i = ms2.I_FaceIndices; i; --i, ++L_FaceIndex_Itr)
        {
            const xWORD3 &Face = ms2.MeshData->GetFace(*L_FaceIndex_Itr);
            if ( TestBoxOTriangle(bo1,
                                  ms2.MeshData->GetVertex(Face[0]),
                                  ms2.MeshData->GetVertex(Face[1]),
                                  ms2.MeshData->GetVertex(Face[2])) )
                return true;
        }
    return false;
}

xDWORD CollideBoxOMesh (const xBoxO &bo1, const xMesh &ms2,
                        IPhysicalBody *body1, IPhysicalBody *body2, CollisionSet &cset)
{
    xDWORD *L_FaceIndex_Itr = ms2.L_FaceIndices;
    xDWORD  I_cols = 0;
    if (ms2.MeshData->L_VertexData_Transf)
        for (int i = ms2.I_FaceIndices; i; --i, ++L_FaceIndex_Itr)
        {
            const xWORD3 &Face = ms2.MeshData->GetFace(*L_FaceIndex_Itr);
            I_cols += CollideBoxOTriangle(bo1,
                                          ms2.MeshData->GetVertexTransf(Face[0]),
                                          ms2.MeshData->GetVertexTransf(Face[1]),
                                          ms2.MeshData->GetVertexTransf(Face[2]),
                                          body1, body2,
                                          ms2, *L_FaceIndex_Itr, cset);
        }
    else
        for (int i = ms2.I_FaceIndices; i; --i, ++L_FaceIndex_Itr)
        {
            const xWORD3 &Face = ms2.MeshData->GetFace(*L_FaceIndex_Itr);
            I_cols += CollideBoxOTriangle(bo1,
                                          ms2.MeshData->GetVertex(Face[0]),
                                          ms2.MeshData->GetVertex(Face[1]),
                                          ms2.MeshData->GetVertex(Face[2]),
                                          body1, body2,
                                          ms2, *L_FaceIndex_Itr, cset);
        }
    return I_cols;
}
