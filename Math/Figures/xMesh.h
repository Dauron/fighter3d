#ifndef __incl_Math_xMesh_h
#define __incl_Math_xMesh_h

#include "xIFigure3d.h"

namespace Math { namespace Figures {
    using namespace ::Math::Figures;

    struct xMeshData
    {
        xMatrix  MX_LocalToWorld;
        xMatrix  MX_MeshToLocal;

        xBYTE   *L_FaceData;
        xDWORD   I_FaceStride;
        xDWORD   I_FaceCount;

        xWORD3 &GetFace(xDWORD I_FaceIndex)
        {
            return *(xWORD3*) (L_FaceData + I_FaceIndex * I_FaceStride);
        }

        xBYTE   *L_VertexData;
        xDWORD   I_VertexStride;
        xDWORD   I_VertexCount;

        xPoint3 &GetVertex(xDWORD I_VertesIndex)
        {
            return *(xPoint3*) (L_VertexData + I_VertesIndex * I_VertexStride);
        }

        xPoint3 *L_VertexData_Transf;
        bool    *FL_VertexIsTransf;

        xPoint3 &GetVertexTransf(xDWORD I_VertesIndex)
        {
            assert ( L_VertexData_Transf );
            return L_VertexData_Transf[I_VertesIndex];
        }

        xMeshData() : L_VertexData_Transf(NULL), FL_VertexIsTransf(NULL) { MX_LocalToWorld.identity(); }
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
            if ( MeshData->L_VertexData_Transf )
            {
                delete[] MeshData->L_VertexData_Transf; MeshData->L_VertexData_Transf = NULL;
                delete[] MeshData->FL_VertexIsTransf;   MeshData->FL_VertexIsTransf   = NULL;
            }

            if (transformationOnly) return;

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

        virtual xIFigure3d *Transform(const xMatrix  &MX_LocalToWorld)
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
            for (int i = I_VertexIndices; i; --i, ++L_VertexIndex_Itr)
            {
                const xDWORD &index = *L_VertexIndex_Itr;

                if (MeshData->FL_VertexIsTransf[index]) continue;

                MeshData->L_VertexData_Transf[index] = MX_MeshToWorld.preTransformP(
                                                           MeshData->GetVertex(index) );
                MeshData->FL_VertexIsTransf[index] = true;
            }

            xMesh *res = new xMesh();
            *res = *this;
            return res;
        }
    };

} } // namespace Math.Figures

#endif
