#include "xMesh.h"

using namespace Math::Figures;

xBoxA xMeshData :: BoundingBox() const
{
    xBoxA res;
    if (!I_VertexCount)
    {
        res.P_min.zero();
        res.P_max.zero();
        return res;
    }

    res.P_max = res.P_min = *(xPoint3*) L_VertexData;
    xDWORD vertexC = I_VertexCount - 1;
    xBYTE *vertexP = L_VertexData + I_VertexStride;

    for (; vertexC; --vertexC, vertexP += I_VertexStride)
    {
        xPoint3 &P = *(xPoint3*) vertexP;
        if (P.x > res.P_max.x) res.P_max.x = P.x;
        else
        if (P.x < res.P_min.x) res.P_min.x = P.x;
        if (P.y > res.P_max.y) res.P_max.y = P.y;
        else
        if (P.y < res.P_min.y) res.P_min.y = P.y;
        if (P.z > res.P_max.z) res.P_max.z = P.z;
        else
        if (P.z < res.P_min.z) res.P_min.z = P.z;
    }

    return res;
}

void  xMeshData :: Transform(const xMatrix  &MX_LocalToWorld)
{
    if ( ! L_VertexData_Transf )
    {
        L_VertexData_Transf = new xPoint3[I_VertexCount];
        FL_VertexIsTransf   = new bool[I_VertexCount];
        memset(FL_VertexIsTransf, 0, sizeof(bool) * I_VertexCount);
    }

    if (MX_LocalToWorld != this->MX_LocalToWorld)
    {
        memset(FL_VertexIsTransf, 0, sizeof(bool) * I_VertexCount);
        this->MX_LocalToWorld = MX_LocalToWorld;
    }

    xMatrix  MX_MeshToWorld = MX_MeshToLocal * MX_LocalToWorld;

    xBYTE   *L_VertexSource_Itr = L_VertexData;
    xPoint3 *L_VertexDest_Itr   = L_VertexData_Transf;
    bool    *L_FLTransf_Itr     = FL_VertexIsTransf;

    if (!MX_Bones || !L_BoneData)
        for (int i = I_VertexCount; i; --i, ++L_VertexDest_Itr, L_VertexSource_Itr += I_VertexStride, ++L_FLTransf_Itr)
        {
            const xPoint3 &point = *(xPoint3*)L_VertexSource_Itr;
            *L_VertexDest_Itr = MX_MeshToWorld.preTransformP(point);
            *L_FLTransf_Itr = true;
        }
    else
    {
        xBYTE *L_Bone_Itr = L_BoneData;

        for (int i = I_VertexCount; i; --i, ++L_VertexDest_Itr, L_VertexSource_Itr += I_VertexStride,
                                           L_Bone_Itr += I_BoneStride, ++L_FLTransf_Itr)
        {
            const xPoint3 &point = *(xPoint3*)L_VertexSource_Itr;

            xFLOAT4 &boneIdxWgh = *(xFLOAT4*)L_Bone_Itr;

            int   idx[4];
            idx[0] = (int) floor(boneIdxWgh[0]);
            idx[1] = (int) floor(boneIdxWgh[1]);
            idx[2] = (int) floor(boneIdxWgh[2]);
            idx[3] = (int) floor(boneIdxWgh[3]);
            float wght[4];
            wght[0] = (boneIdxWgh[0] - idx[0]);
            wght[1] = (boneIdxWgh[1] - idx[1]);
            wght[2] = (boneIdxWgh[2] - idx[2]);
            wght[3] = (boneIdxWgh[3] - idx[3]);

            xPoint3 &P_vert = *L_VertexDest_Itr;
            xPoint3 &P_src  = *(xPoint3*)L_VertexSource_Itr;

            P_vert.zero();
            for (int b=0; b<4; ++b)
            {
                if (wght[b] < 0.001) break;
                P_vert += MX_Bones[idx[b]].postTransformP(P_src) * (wght[b]*10.f);
            }
            P_vert = MX_MeshToWorld.preTransformP( P_vert );

            *L_FLTransf_Itr = true;
        }
    }
}

xIFigure3d * xMesh :: Transform(const xMatrix  &MX_LocalToWorld)
{
    if ( ! MeshData->L_VertexData_Transf )
    {
        MeshData->L_VertexData_Transf = new xPoint3[MeshData->I_VertexCount];
        MeshData->FL_VertexIsTransf   = new bool[MeshData->I_VertexCount];
        memset(MeshData->FL_VertexIsTransf, 0, sizeof(bool) * MeshData->I_VertexCount);
    }

    if (MX_LocalToWorld != MeshData->MX_LocalToWorld)
    {
        memset(MeshData->FL_VertexIsTransf, 0, sizeof(bool) * MeshData->I_VertexCount);
        MeshData->MX_LocalToWorld = MX_LocalToWorld;
    }

    xMatrix  MX_MeshToWorld = MeshData->MX_MeshToLocal * MX_LocalToWorld;

    xDWORD  *L_VertexIndex_Itr     = L_VertexIndices;
    if (!MeshData->MX_Bones || !MeshData->L_BoneData)
        for (int i = I_VertexIndices; i; --i, ++L_VertexIndex_Itr)
        {
            const xDWORD &index = *L_VertexIndex_Itr;

            if (MeshData->FL_VertexIsTransf[index]) continue;

            MeshData->L_VertexData_Transf[index] = MX_MeshToWorld.preTransformP(
                                                       MeshData->GetVertex(index) );
            MeshData->FL_VertexIsTransf[index] = true;
        }
    else
        for (int i = I_VertexIndices; i; --i, ++L_VertexIndex_Itr)
        {
            const xDWORD &index = *L_VertexIndex_Itr;

            if (MeshData->FL_VertexIsTransf[index]) continue;

            xFLOAT4 &boneIdxWgh = MeshData->GetBone(index);

            int   idx[4];
            idx[0] = (int) floor(boneIdxWgh[0]);
            idx[1] = (int) floor(boneIdxWgh[1]);
            idx[2] = (int) floor(boneIdxWgh[2]);
            idx[3] = (int) floor(boneIdxWgh[3]);
            float wght[4];
            wght[0] = (boneIdxWgh[0] - idx[0]);
            wght[1] = (boneIdxWgh[1] - idx[1]);
            wght[2] = (boneIdxWgh[2] - idx[2]);
            wght[3] = (boneIdxWgh[3] - idx[3]);

            xPoint3 &P_vert = MeshData->L_VertexData_Transf[index];
            xPoint3 &P_src  = MeshData->GetVertex(index);

            P_vert.zero();
            for (int b=0; b<4; ++b)
            {
                if (wght[b] < 0.001) break;
                P_vert += MeshData->MX_Bones[idx[b]].postTransformP(P_src) * (wght[b]*10.f);
            }
            P_vert = MX_MeshToWorld.preTransformP( P_vert );

            MeshData->FL_VertexIsTransf[index] = true;
        }

    xMesh *res = new xMesh();
    //res->P_center = MX_LocalToWorld.preTransformP(res->P_center);
    *res = *this;
    return res;
}