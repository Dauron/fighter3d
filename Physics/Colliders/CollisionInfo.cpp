#include "CollisionInfo.h"
#include "../../Math/Figures/xMesh.h"

using namespace Physics::Colliders;
using namespace Math::Figures;

void CollisionPoint :: SetBoneWeights()
{
    I_Bones = 0;
    if (Figure->Type != xIFigure3d::Mesh)
        return;

    xMesh &mesh = *(xMesh*) Figure;
    if (! mesh.MeshData->L_BoneData)
        return;

    xWORD3 &face = mesh.MeshData->GetFace(ID_subobj);
    xFLOAT3 W_vert;
    W_vert[0] = (mesh.MeshData->GetVertexTransf( face[0] ) - P_collision).lengthSqr();
    W_vert[1] = (mesh.MeshData->GetVertexTransf( face[1] ) - P_collision).lengthSqr();
    W_vert[2] = (mesh.MeshData->GetVertexTransf( face[2] ) - P_collision).lengthSqr();
    xFLOAT W_scale = 1.f / (W_vert[0] + W_vert[1] + W_vert[2]);
    W_vert[0] = 1.f - W_vert[0] * W_scale;
    W_vert[1] = 1.f - W_vert[1] * W_scale;
    W_vert[2] = 1.f - W_vert[2] * W_scale;

    for (int i = 0; i < 3; ++i)
    {
        xFLOAT4 &boneIdxWgh = mesh.MeshData->GetBone(face[i]);

        int   I_bone[4];
        I_bone[0] = (int) floor(boneIdxWgh[0]);
        I_bone[1] = (int) floor(boneIdxWgh[1]);
        I_bone[2] = (int) floor(boneIdxWgh[2]);
        I_bone[3] = (int) floor(boneIdxWgh[3]);
        float W_bone[4];
        W_bone[0] = (boneIdxWgh[0] - I_bone[0]);
        W_bone[1] = (boneIdxWgh[1] - I_bone[1]);
        W_bone[2] = (boneIdxWgh[2] - I_bone[2]);
        W_bone[3] = (boneIdxWgh[3] - I_bone[3]);

        for (int bi = 0; bi < 4; ++bi)
        {
            if (W_bone[bi] < 0.001) break;
            bool found = false;
            for (int ci = 0; ci < I_Bones && !found; ++ci)
                if (W_Bones[ci].I_bone == I_bone[bi])
                {
                    W_Bones[ci].W_bone += W_vert[i] * W_bone[bi]*10.f;
                    found = true;
                }
            if (! found)
            {
                W_Bones[I_Bones].W_bone = W_vert[i] * W_bone[bi]*10.f;
                W_Bones[I_Bones].I_bone = I_bone[bi];
                ++I_Bones;
            }
        }
    }
}