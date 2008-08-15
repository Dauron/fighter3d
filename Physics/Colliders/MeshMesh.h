// Included in FigureCollider.cpp
// Depends on xMesh.h

////////////////////////////// Mesh - Mesh

bool   TestMeshMesh (const xMesh &ms1, const xMesh &ms2)
{
    xDWORD *L_FaceIndex_Itr = ms1.L_FaceIndices;
    if (ms1.MeshData->L_VertexData_Transf)
        for (int i = ms1.I_FaceIndices; i; --i, ++L_FaceIndex_Itr)
        {
            const xWORD3 &Face = ms1.MeshData->GetFace(*L_FaceIndex_Itr);
            if ( TestTriangleMesh(ms1.MeshData->GetVertexTransf(Face[0]),
                                  ms1.MeshData->GetVertexTransf(Face[1]),
                                  ms1.MeshData->GetVertexTransf(Face[2]),
                                  ms2) )
                return true;
        }
    else
        for (int i = ms2.I_FaceIndices; i; --i, ++L_FaceIndex_Itr)
        {
            const xWORD3 &Face = ms2.MeshData->GetFace(*L_FaceIndex_Itr);
            if ( TestTriangleMesh(ms1.MeshData->GetVertex(Face[0]),
                                  ms1.MeshData->GetVertex(Face[1]),
                                  ms1.MeshData->GetVertex(Face[2]),
                                  ms2) )
                return true;
        }
    return false;
}

xDWORD CollideMeshMesh (const xMesh &ms1, const xMesh &ms2, CollisionSet &cset)
{
    xDWORD *L_FaceIndex_Itr = ms1.L_FaceIndices;
    xDWORD  I_cols = 0;
    if (ms1.MeshData->L_VertexData_Transf)
        for (int i = ms1.I_FaceIndices; i ; --i, ++L_FaceIndex_Itr)
        {
            const xWORD3 &Face = ms1.MeshData->GetFace(*L_FaceIndex_Itr);
            I_cols += CollideTriangleMesh(ms1.MeshData->GetVertexTransf(Face[0]),
                                          ms1.MeshData->GetVertexTransf(Face[1]),
                                          ms1.MeshData->GetVertexTransf(Face[2]),
                                          ms2,
                                          ms1, *L_FaceIndex_Itr, cset);
        }
    else
        for (int i = ms1.I_FaceIndices; i ; --i, ++L_FaceIndex_Itr)
        {
            const xWORD3 &Face = ms2.MeshData->GetFace(*L_FaceIndex_Itr);
            I_cols += CollideTriangleMesh(ms1.MeshData->GetVertex(Face[0]),
                                          ms1.MeshData->GetVertex(Face[1]),
                                          ms1.MeshData->GetVertex(Face[2]),
                                          ms2,
                                          ms1, *L_FaceIndex_Itr, cset);
        }
    return I_cols;
}