#include "ShadowVolume.h"

namespace ShadowVolume
{

    void ExtrudePoints (const xElement *elem, bool infiniteL, const xVector3 &lightPos,
                        xShadowData &shadowData)
    {
        xVector4 *dest = shadowData.L_vertices + elem->I_vertices;

        if (infiniteL)
        {
            dest->init(-lightPos, 0.f);
            return;
        }

        xVector4 *src  = shadowData.L_vertices,
                 *last = dest;
        for (; src != last; ++src, ++dest)
            dest->init(src->vector3 - lightPos, 0.f);
    }

    void GetBackFaces (const xElement *elem, const xElementInstance &instance, bool infiniteL,
                       const xShadowData &shadowData, bool *&backFaces)
    {
        if (backFaces == NULL)
            backFaces = new bool[elem->I_faces];

        bool      *dest          = backFaces;
        xFace     *face          = elem->L_faces;
        xVector4  *extrVerticesP = shadowData.L_vertices + elem->I_vertices;
        xFaceList *iterL         = elem->L_faceLists;
        xWORD      maxOffset     = iterL->I_offset + iterL->I_count - 1;

        if (infiniteL)
        {
            xVector3 lightDir = extrVerticesP->vector3;
            if (elem->FL_skeletized)
            {
                xVector3 faceNormal;
                for (int i = 0; i < elem->I_faces; ++i, ++face, ++dest)
                {
                    if (i > maxOffset) { ++iterL; maxOffset = iterL->I_offset+iterL->I_count-1; }
                    if (iterL->Material && iterL->Material->two_sided)
                        *dest = true;
                    else
                    {
                        faceNormal = instance.L_normals[(*face)[0]] + instance.L_normals[(*face)[1]] + instance.L_normals[(*face)[2]];
                        *dest = xVector3::DotProduct(lightDir, faceNormal) > 0;
                    }
                }
            }
            else
            {
                xVector3 *faceNormal = elem->renderData.L_face_normals;
                for (int i = 0; i < elem->I_faces; ++i, ++face, ++faceNormal, ++dest)
                {
                    if (i > maxOffset) { ++iterL; maxOffset = iterL->I_offset+iterL->I_count-1; }
                    if (iterL->Material && iterL->Material->two_sided)
                        *dest = true;
                    else
                        *dest = xVector3::DotProduct(lightDir, *faceNormal) > 0;
                }
            }
        }
        else
        {
            if (elem->FL_skeletized && instance.L_normals)
            {
                xVector3 faceNormal;
                for (int i = 0; i < elem->I_faces; ++i, ++face, ++dest)
                {
                    if (i > maxOffset) { ++iterL; maxOffset = iterL->I_offset+iterL->I_count-1; }
                    if (iterL->Material && iterL->Material->two_sided)
                        *dest = true;
                    else
                    {
                        faceNormal = instance.L_normals[(*face)[0]] + instance.L_normals[(*face)[1]] + instance.L_normals[(*face)[2]];
                        *dest = xVector3::DotProduct((extrVerticesP + *face[0])->vector3, faceNormal) > 0;
                    }
                }
            }
            else
            {
                xVector3 *faceNormal = elem->renderData.L_face_normals;
                for (int i = 0; i < elem->I_faces; ++i, ++face, ++faceNormal, ++dest)
                {
                    if (i > maxOffset) { ++iterL; maxOffset = iterL->I_offset+iterL->I_count-1; }
                    if (iterL->Material && iterL->Material->two_sided)
                        *dest = true;
                    else
                        *dest = xVector3::DotProduct((extrVerticesP + (*face)[0])->vector3, *faceNormal) > 0;
                }
            }
        }
    }

    xBYTE *buffer = NULL;
    xDWORD buffSize = 0;

    void GetSilhouette(const xElement *elem, bool infiniteL, bool optimizeBackCap,
                       const bool *facingFlag, xShadowData &shadowData)
    {
        shadowData.I_sides = 0;
        shadowData.I_fronts = 0;
        shadowData.I_backs = 0;
        if (shadowData.L_indices) delete[] shadowData.L_indices;

        xWORD3 *frontCapP = NULL;
        if (shadowData.zDataLevel == xShadowData::ZFAIL_PASS)
            frontCapP = new xWORD3[elem->I_faces];
        xWORD3 *frontDest = frontCapP;

        xEdge  *edgeIter = elem->L_edges;
        xEdge  *edgeEnd  = edgeIter + elem->I_edges;

        if (!infiniteL)
        {
            xWORD  backFirst = elem->I_vertices + edgeIter->ID_vert_1;

            if (buffSize < elem->I_edges*sizeof(xWORD4))
            {
                if (buffer) delete[] buffer;
                buffSize = elem->I_edges*sizeof(xWORD4);
                buffer = new xBYTE[buffSize];
            }
            xWORD4 *sideQadsP = (xWORD4*)buffer;
            xWORD4 *sideDest  = sideQadsP;
            xWORD3 *backCapP  = NULL;

            if (shadowData.zDataLevel == xShadowData::ZFAIL_PASS)
                if (optimizeBackCap)
                    backCapP = new xWORD3[elem->I_edges];
                else
                    backCapP = new xWORD3[elem->I_faces];
            xWORD3 *backDest  = backCapP;

            for (; edgeIter != edgeEnd; ++edgeIter)
                if (facingFlag[edgeIter->ID_face_1] ^ (edgeIter->ID_face_2 != xWORD_MAX && facingFlag[edgeIter->ID_face_2]))
                {
                    if (facingFlag[edgeIter->ID_face_1])
                    {
                        (*sideDest)[0] = edgeIter->ID_vert_2;
                        (*sideDest)[1] = edgeIter->ID_vert_1;
                        (*sideDest)[2] = elem->I_vertices + edgeIter->ID_vert_1;
                        (*sideDest)[3] = elem->I_vertices + edgeIter->ID_vert_2;
                    }
                    else
                    {
                        (*sideDest)[0] = edgeIter->ID_vert_1;
                        (*sideDest)[1] = edgeIter->ID_vert_2;
                        (*sideDest)[2] = elem->I_vertices + edgeIter->ID_vert_2;
                        (*sideDest)[3] = elem->I_vertices + edgeIter->ID_vert_1;
                    }
                    if (optimizeBackCap && shadowData.zDataLevel == xShadowData::ZFAIL_PASS)
                    {
                        (*backDest)[0] = backFirst;
                        (*backDest)[1] = (*sideDest)[3];
                        (*backDest)[2] = (*sideDest)[2];
                        ++shadowData.I_backs;
                        ++backDest;
                    }
                    ++sideDest;
                    ++shadowData.I_sides;
                }

            if (shadowData.zDataLevel == xShadowData::ZFAIL_PASS)
            {
                xFaceList *faceList = elem->L_faceLists;
                for (int i=elem->I_faceLists; i; --i, ++faceList)
                {
                    if (!faceList->Material || faceList->Material->transparency == 0.f)
                    {
                        xWORD3     *face     = elem->L_faces + faceList->I_offset;
                        const bool *faceFlag = facingFlag + faceList->I_offset;
                        for (int j = faceList->I_count; j; --j, ++face, ++faceFlag)
                            if (*faceFlag)
                            {
                                (*frontDest)[0] = (*face)[0];
                                (*frontDest)[1] = (*face)[1];
                                (*frontDest)[2] = (*face)[2];
                                ++shadowData.I_fronts;
                                ++frontDest;
                            }
                            else
                            if(!optimizeBackCap)
                            {
                                (*backDest)[0] = elem->I_vertices + (*face)[0];
                                (*backDest)[1] = elem->I_vertices + (*face)[1];
                                (*backDest)[2] = elem->I_vertices + (*face)[2];
                                ++shadowData.I_backs;
                                ++backDest;
                            }
                    }
                }
                shadowData.I_indices = 4*shadowData.I_sides + 3*shadowData.I_fronts + 3*shadowData.I_backs;
                shadowData.L_indices = new xWORD[shadowData.I_indices];
                memcpy(shadowData.L_indices, sideQadsP, shadowData.I_sides*sizeof(xWORD4));
                memcpy(shadowData.L_indices + shadowData.I_sides*4, frontCapP, shadowData.I_fronts*sizeof(xWORD3));
                delete[] frontCapP;
                memcpy(shadowData.L_indices + shadowData.I_sides*4 + shadowData.I_fronts*3,
                       backCapP, shadowData.I_backs*sizeof(xWORD3));
                delete[] backCapP;
            }
            else
            {
                shadowData.I_indices = 4*shadowData.I_sides;
                shadowData.L_indices = new xWORD[shadowData.I_indices];
                memcpy(shadowData.L_indices, sideQadsP, shadowData.I_sides*sizeof(xWORD4));
            }
        }
        else
        {
            if (buffSize < elem->I_edges*sizeof(xWORD3))
            {
                if (buffer) delete[] buffer;
                buffSize = elem->I_edges*sizeof(xWORD3);
                buffer = new xBYTE[buffSize];
            }
            xWORD3 *sideTrisP = (xWORD3*) buffer;
            xWORD3 *sideDest = sideTrisP;

            for (; edgeIter != edgeEnd; ++edgeIter)
                if (facingFlag[edgeIter->ID_face_1] ^ (edgeIter->ID_face_2 != xWORD_MAX && facingFlag[edgeIter->ID_face_2]))
                {
                    if (facingFlag[edgeIter->ID_face_1])
                    {
                        (*sideDest)[0] = edgeIter->ID_vert_2;
                        (*sideDest)[1] = edgeIter->ID_vert_1;
                        (*sideDest)[2] = elem->I_vertices;
                    }
                    else
                    {
                        (*sideDest)[0] = edgeIter->ID_vert_1;
                        (*sideDest)[1] = edgeIter->ID_vert_2;
                        (*sideDest)[2] = elem->I_vertices;
                    }
                    ++sideDest;
                    ++shadowData.I_sides;
                }

            if (shadowData.zDataLevel == xShadowData::ZFAIL_PASS)
            {
                xFaceList *faceList = elem->L_faceLists;
                for (int i=elem->I_faceLists; i; --i, ++faceList)
                {
                    if (!faceList->Material || faceList->Material->transparency == 0.f)
                    {
                        xWORD3     *face     = elem->L_faces + faceList->I_offset;
                        const bool *faceFlag = facingFlag + faceList->I_offset;
                        for (int j = faceList->I_count; j; --j, ++face, ++faceFlag)
                            if (*faceFlag)
                            {
                                (*frontDest)[0] = (*face)[0];
                                (*frontDest)[1] = (*face)[1];
                                (*frontDest)[2] = (*face)[2];
                                ++shadowData.I_fronts;
                                ++frontDest;
                            }
                    }
                }
                shadowData.I_indices = 3*shadowData.I_sides + 3*shadowData.I_fronts;
                shadowData.L_indices = new xWORD[shadowData.I_indices];
                memcpy(shadowData.L_indices, sideTrisP, shadowData.I_sides*sizeof(xWORD3));
                memcpy(shadowData.L_indices + shadowData.I_sides*3, frontCapP, shadowData.I_fronts*sizeof(xWORD3));
                delete[] frontCapP;
            }
            else
            {
                shadowData.I_indices = 3*shadowData.I_sides;
                shadowData.L_indices = new xWORD[shadowData.I_indices];
                memcpy(shadowData.L_indices, sideTrisP, shadowData.I_sides*sizeof(xWORD3));
            }
        }
    }

    bool ViewportMaybeShadowed (xElementInstance &instance, const Math::Cameras::FieldOfView &FOV, const xLight& light)
    {
        xPlane occlusionPyramid[6];
        int numPlanes = 5;

        occlusionPyramid[0].init(FOV.Corners3D[0], FOV.Corners3D[2], FOV.Corners3D[1]);

        xVector3 lightDirection;
        if (light.type == xLight_INFINITE)
        {
            occlusionPyramid[1].init(FOV.Corners3D[0] + light.position, FOV.Corners3D[0], FOV.Corners3D[1] );
            occlusionPyramid[2].init(FOV.Corners3D[1] + light.position, FOV.Corners3D[1], FOV.Corners3D[2] );
            occlusionPyramid[3].init(FOV.Corners3D[2] + light.position, FOV.Corners3D[2], FOV.Corners3D[3] );
            occlusionPyramid[4].init(FOV.Corners3D[3] + light.position, FOV.Corners3D[3], FOV.Corners3D[0] );
            lightDirection = -light.position;
        }
        else
        {
            occlusionPyramid[1].init(light.position, FOV.Corners3D[0], FOV.Corners3D[1] );
            occlusionPyramid[2].init(light.position, FOV.Corners3D[1], FOV.Corners3D[2] );
            occlusionPyramid[3].init(light.position, FOV.Corners3D[2], FOV.Corners3D[3] );
            occlusionPyramid[4].init(light.position, FOV.Corners3D[3], FOV.Corners3D[0] );
            xPoint3 viewCenter = (FOV.Corners3D[0] + FOV.Corners3D[2]) * 0.5f;
            lightDirection = (viewCenter - light.position).normalize();
            numPlanes++;
            occlusionPyramid[5].init(-lightDirection, light.position);
        }

        xFLOAT cosa = xVector3::DotProduct(lightDirection, occlusionPyramid[0].vector3);
        if (cosa < 0.f) // objectViewVector
            // light is behind us, flip all occlusion planes
            for (int i = 0; i < 5; ++i)
                occlusionPyramid[i].invert();

        return !instance.bBox_T->CulledBy(occlusionPyramid, numPlanes);
    }

}
