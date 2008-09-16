#ifndef __incl_Math_xMesh_h
#define __incl_Math_xMesh_h

#include "xIFigure3d.h"
#include "xBoxA.h"

namespace Math { namespace Figures {
    using namespace ::Math::Figures;

    struct xMeshData
    {
        xBYTE    ID;

        xMatrix  MX_LocalToWorld;
        xMatrix  MX_MeshToLocal;
        xMatrix *MX_Bones;
        xBYTE    I_Bones;

        xMatrix  MX_MeshToWorld;
        xMatrix *MX_BonesToWorld;

        void CalculateLocalBones(const xMatrix &mx_LocalToWorld)
        {
            if (!MX_BonesToWorld) MX_BonesToWorld = new xMatrix[I_Bones];
            MX_LocalToWorld = mx_LocalToWorld;
            MX_MeshToWorld = MX_MeshToLocal * mx_LocalToWorld;
            if (MX_Bones)
                for (int i = 0; i < I_Bones; ++i)
                    MX_BonesToWorld[i] = xMatrix::Transpose(MX_Bones[i]) * MX_MeshToWorld;
        }

        xBYTE   *L_FaceData;
        xDWORD   I_FaceStride;
        xDWORD   I_FaceCount;

        xWORD3 *GetFacePtr(xDWORD I_FaceIndex)
        {
            return (xWORD3*) (L_FaceData + I_FaceIndex * I_FaceStride);
        }
        xWORD3 &GetFace(xDWORD I_FaceIndex)
        {
            return *(xWORD3*) (L_FaceData + I_FaceIndex * I_FaceStride);
        }

        xBYTE   *L_VertexData;
        xDWORD   I_VertexStride;
        xDWORD   I_VertexCount;

        xPoint3 &GetVertex(xDWORD I_VertexIndex)
        {
            return *(xPoint3*) (L_VertexData + I_VertexIndex * I_VertexStride);
        }

        xBYTE   *L_BoneData;
        xDWORD   I_BoneStride;

        xFLOAT4 &GetBone(xDWORD I_VertexIndex)
        {
            return *(xFLOAT4*) (L_BoneData + I_VertexIndex * I_BoneStride);
        }

        xPoint3 *L_VertexData_Transf;
        bool    *FL_VertexIsTransf;
        bool     FL_MeshIsTransf;

        xPoint3 &GetVertexTransf(xDWORD I_VertexIndex)
        {
            assert ( L_VertexData_Transf );
            assert ( FL_VertexIsTransf[I_VertexIndex] );
            return L_VertexData_Transf[I_VertexIndex];
        }

        xFLOAT   S_radius;
        xVector3 NW_dims;
        xPoint3  P_center;

        void CalculateProperties()
        {
            xBoxA box = BoundingBox();
            NW_dims = (box.P_max - box.P_min) * 0.5f;
            S_radius = NW_dims.length();
            P_center = box.P_min + NW_dims;
        }

        xBoxA BoundingBox() const;
        virtual void Transform(const xMatrix  &MX_LocalToWorld);

        xMeshData()
            : MX_Bones(NULL)
            , MX_BonesToWorld(NULL)
            , L_BoneData(NULL)
            , L_VertexData_Transf(NULL)
            , FL_VertexIsTransf(NULL)
            , FL_MeshIsTransf(false)
        { MX_LocalToWorld.x0 = xFLOAT_HUGE_NEGATIVE; }
    };

    struct xMesh : xIFigure3d
    {
        xMeshData *MeshData;

        xDWORD    *L_FaceIndices;
        xDWORD     I_FaceIndices;

        xDWORD    *L_VertexIndices;
        xDWORD     I_VertexIndices;

        xMesh() { Type = xIFigure3d::Mesh; MeshData = NULL; }

        virtual void free(bool transformationOnly = false)
        {
            MeshData->FL_MeshIsTransf = false;

            if (transformationOnly)
            {
                memset(MeshData->FL_VertexIsTransf, 0, sizeof(bool) * MeshData->I_VertexCount);
                return;
            }

            if ( MeshData->L_VertexData_Transf )
            {
                delete[] MeshData->L_VertexData_Transf; MeshData->L_VertexData_Transf = NULL;
                delete[] MeshData->FL_VertexIsTransf;   MeshData->FL_VertexIsTransf   = NULL;
            }

            delete[] L_FaceIndices;   L_FaceIndices   = NULL;
            delete[] L_VertexIndices; L_VertexIndices = NULL;
        }

        virtual void ComputeSpan(const xVector3 &N_axis, xFLOAT &P_min, xFLOAT &P_max, int axis = -1) const
        {
            assert ( MeshData->L_VertexData_Transf );

            xFLOAT P_tmp;
            xDWORD *L_VertexIndex_Itr = L_VertexIndices;

            P_min = P_max = xVector3::DotProduct(N_axis, MeshData->L_VertexData_Transf[*L_VertexIndex_Itr]);
            ++L_VertexIndex_Itr;

            for (int i = I_VertexIndices-1; i; --i, ++L_VertexIndex_Itr)
            {
                P_tmp = xVector3::DotProduct(N_axis, MeshData->L_VertexData_Transf[*L_VertexIndex_Itr]);
                if (P_tmp > P_max) P_max = P_tmp;
                else
                if (P_tmp < P_min) P_min = P_tmp;
            }
        }

        virtual xIFigure3d *Transform(const xMatrix  &MX_LocalToWorld);

        virtual void   P_MinMax_Get( xPoint3 &P_min, xPoint3 &P_max )
        {
            assert ( MeshData->L_VertexData_Transf );

            xDWORD *L_VertexIndex_Itr = L_VertexIndices;

            P_min = P_max = MeshData->L_VertexData_Transf[*L_VertexIndex_Itr];
            ++L_VertexIndex_Itr;

            for (int i = I_VertexIndices-1; i; --i, ++L_VertexIndex_Itr)
            {
                xPoint3 &P_a = MeshData->L_VertexData_Transf[*L_VertexIndex_Itr];
                if (P_a.x < P_min.x) { P_min.x = P_a.x; }
                else
                if (P_a.x > P_max.x) { P_max.x = P_a.x; }
                if (P_a.y < P_min.y) { P_min.y = P_a.y; }
                else
                if (P_a.y > P_max.y) { P_max.y = P_a.y; }
                if (P_a.z < P_min.z) { P_min.z = P_a.z; }
                else
                if (P_a.z > P_max.z) { P_max.z = P_a.z; }
            }
        }
        virtual xFLOAT S_Radius_Sqr_Get() { return MeshData->S_radius*MeshData->S_radius; }
        virtual xFLOAT W_Volume_Get()     { return MeshData->NW_dims.x * MeshData->NW_dims.y * MeshData->NW_dims.z * 8.f; }
    };

} } // namespace Math.Figures

#endif
