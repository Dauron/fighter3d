#ifndef __incl_Math_xMesh_h
#define __incl_Math_xMesh_h

#include "xIFigure3d.h"
#include "xBoxA.h"

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

        xPoint3 &GetVertex(xDWORD I_VertexIndex)
        {
            return *(xPoint3*) (L_VertexData + I_VertexIndex * I_VertexStride);
        }

        xPoint3 *L_VertexData_Transf;
        bool    *FL_VertexIsTransf;

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

        xBoxA BoundingBox()
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
            //res->P_center = MX_LocalToWorld.preTransformP(res->P_center);
            *res = *this;
            return res;
        }

        virtual xFLOAT S_Radius_Sqr_Get() { return MeshData->S_radius*MeshData->S_radius; }
        virtual xFLOAT W_Volume_Get()     { return MeshData->NW_dims.x * MeshData->NW_dims.y * MeshData->NW_dims.z * 8.f; }
    };

} } // namespace Math.Figures

#endif
