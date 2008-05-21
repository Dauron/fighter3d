#include "xUtils.h"
#include "xShadowVolume.h"

void xShadows_ExtrudePoints (const xElement *elem, bool infiniteL, const xVector3 &lightPos,
                             xShadowData &shadowData)
{
    xVector4 *dest = shadowData.verticesP + elem->verticesC;

    if (infiniteL)
    {
        dest->init(-lightPos, 0.f);
        return;
    }

    xVector4 *src  = shadowData.verticesP;
    for (int i = elem->verticesC; i; --i, ++src, ++dest)
        dest->init(src->vector3 - lightPos, 0.f);
}

void xShadows_GetBackFaces (const xElement *elem, const xElementInstance &instance, bool infiniteL,
                            const xShadowData &shadowData, bool *&backFaces)
{
    if (backFaces == NULL)
        backFaces = new bool[elem->facesC];

    bool     *dest = backFaces;
    xFace    *face = elem->facesP;
    xVector4 *extrVerticesP = shadowData.verticesP + elem->verticesC;

    if (infiniteL)
    {
        xVector3 lightDir = extrVerticesP->vector3;
        if (elem->skeletized)
        {
            xVector3 faceNormal;
            for (int i = elem->facesC; i; --i, ++face, ++dest)
            {
                faceNormal = instance.normalsP[(*face)[0]] + instance.normalsP[(*face)[1]] + instance.normalsP[(*face)[2]];
                *dest = xVector3::DotProduct(lightDir, faceNormal) > 0;
            }
        }
        else
        {
            xVector3 *faceNormal = elem->renderData.faceNormalsP;
            for (int i = elem->facesC; i; --i, ++face, ++faceNormal, ++dest)
                *dest = xVector3::DotProduct(lightDir, *faceNormal) > 0;
        }
    }
    else
    {
        if (elem->skeletized)
        {
            xVector3 faceNormal;
            for (int i = elem->facesC; i; --i, ++face, ++dest)
            {
                faceNormal = instance.normalsP[(*face)[0]] + instance.normalsP[(*face)[1]] + instance.normalsP[(*face)[2]];
                *dest = xVector3::DotProduct((extrVerticesP + *face[0])->vector3, faceNormal) > 0;
            }
        }
        else
        {
            xVector3 *faceNormal = elem->renderData.faceNormalsP;
            for (int i = elem->facesC; i; --i, ++face, ++faceNormal, ++dest)
                *dest = xVector3::DotProduct((extrVerticesP + (*face)[0])->vector3, *faceNormal) > 0;
        }
    }
}

xBYTE *buffer = NULL;
xDWORD buffSize = 0;

void xShadows_GetSilhouette(const xElement *elem, bool infiniteL, bool optimizeBackCap,
                            const bool *facingFlag, xShadowData &shadowData)
{
    shadowData.sideC = 0;
    shadowData.frontC = 0;
    shadowData.backC = 0;
    if (shadowData.indexP) delete[] shadowData.indexP;

    xWORD3 *frontCapP = NULL;
    if (shadowData.zDataLevel == xShadowData::ZFAIL_PASS)
        frontCapP = new xWORD3[elem->facesC];
    xWORD3 *frontDest = frontCapP;

    xEdge  *edgeIter = elem->edgesP;
    xEdge  *edgeEnd  = edgeIter + elem->edgesC;

    if (!infiniteL)
    {
        xWORD  backFirst = elem->verticesC + edgeIter->vert1;

        if (buffSize < elem->edgesC*sizeof(xWORD4))
        {
            if (buffer) delete[] buffer;
            buffSize = elem->edgesC*sizeof(xWORD4);
            buffer = new xBYTE[buffSize];
        }
        xWORD4 *sideQadsP = (xWORD4*)buffer;
        xWORD4 *sideDest  = sideQadsP;
        xWORD3 *backCapP  = NULL;

        if (shadowData.zDataLevel == xShadowData::ZFAIL_PASS)
            if (optimizeBackCap)
                backCapP = new xWORD3[elem->edgesC];
            else
                backCapP = new xWORD3[elem->facesC];
        xWORD3 *backDest  = backCapP;

        for (; edgeIter != edgeEnd; ++edgeIter)
            if (facingFlag[edgeIter->face1] ^ (edgeIter->face2 != xWORD_MAX && facingFlag[edgeIter->face2]))
            {
                if (facingFlag[edgeIter->face1])
                {
                    (*sideDest)[0] = edgeIter->vert2;
                    (*sideDest)[1] = edgeIter->vert1;
                    (*sideDest)[2] = elem->verticesC + edgeIter->vert1;
                    (*sideDest)[3] = elem->verticesC + edgeIter->vert2;
                }
                else
                {
                    (*sideDest)[0] = edgeIter->vert1;
                    (*sideDest)[1] = edgeIter->vert2;
                    (*sideDest)[2] = elem->verticesC + edgeIter->vert2;
                    (*sideDest)[3] = elem->verticesC + edgeIter->vert1;
                }
                if (optimizeBackCap && shadowData.zDataLevel == xShadowData::ZFAIL_PASS)
                {
                    (*backDest)[0] = backFirst;
                    (*backDest)[1] = (*sideDest)[3];
                    (*backDest)[2] = (*sideDest)[2];
                    ++shadowData.backC;
                    ++backDest;
                }
                ++sideDest;
                ++shadowData.sideC;
            }

        if (shadowData.zDataLevel == xShadowData::ZFAIL_PASS)
        {
            xFaceList *faceList = elem->faceListP;
            for (int i=elem->faceListC; i; --i, ++faceList)
            {
                if (!faceList->materialP || faceList->materialP->transparency == 0.f)
                {
                    xWORD3     *face     = elem->facesP + faceList->indexOffset;
                    const bool *faceFlag = facingFlag + faceList->indexOffset;
                    for (int j = faceList->indexCount; j; --j, ++face, ++faceFlag)
                        if (*faceFlag)
                        {
                            (*frontDest)[0] = (*face)[0];
                            (*frontDest)[1] = (*face)[1];
                            (*frontDest)[2] = (*face)[2];
                            ++shadowData.frontC;
                            ++frontDest;
                        }
                        else
                        if(!optimizeBackCap)
                        {
                            (*backDest)[0] = elem->verticesC + (*face)[0];
                            (*backDest)[1] = elem->verticesC + (*face)[1];
                            (*backDest)[2] = elem->verticesC + (*face)[2];
                            ++shadowData.backC;
                            ++backDest;
                        }
                }
            }
            shadowData.indexP = new xWORD[4*shadowData.sideC + 3*shadowData.frontC + 3*shadowData.backC];
            memcpy(shadowData.indexP, sideQadsP, shadowData.sideC*sizeof(xWORD4));
            memcpy(shadowData.indexP + shadowData.sideC*4, frontCapP, shadowData.frontC*sizeof(xWORD3));
            delete[] frontCapP;
            memcpy(shadowData.indexP + shadowData.sideC*4 + shadowData.frontC*3,
                   backCapP, shadowData.backC*sizeof(xWORD3));
            delete[] backCapP;
        }
        else
        {
            shadowData.indexP = new xWORD[4*shadowData.sideC];
            memcpy(shadowData.indexP, sideQadsP, shadowData.sideC*sizeof(xWORD4));
        }
    }
    else
    {
        if (buffSize < elem->edgesC*sizeof(xWORD3))
        {
            if (buffer) delete[] buffer;
            buffSize = elem->edgesC*sizeof(xWORD3);
            buffer = new xBYTE[buffSize];
        }
        xWORD3 *sideTrisP = (xWORD3*) buffer;
        xWORD3 *sideDest = sideTrisP;

        for (; edgeIter != edgeEnd; ++edgeIter)
            if (facingFlag[edgeIter->face1] ^ (edgeIter->face2 != xWORD_MAX && facingFlag[edgeIter->face2]))
            {
                if (facingFlag[edgeIter->face1])
                {
                    (*sideDest)[0] = edgeIter->vert2;
                    (*sideDest)[1] = edgeIter->vert1;
                    (*sideDest)[2] = elem->verticesC;
                }
                else
                {
                    (*sideDest)[0] = edgeIter->vert1;
                    (*sideDest)[1] = edgeIter->vert2;
                    (*sideDest)[2] = elem->verticesC;
                }
                ++sideDest;
                ++shadowData.sideC;
            }

        if (shadowData.zDataLevel == xShadowData::ZFAIL_PASS)
        {
            xFaceList *faceList = elem->faceListP;
            for (int i=elem->faceListC; i; --i, ++faceList)
            {
                if (!faceList->materialP || faceList->materialP->transparency == 0.f)
                {
                    xWORD3     *face     = elem->facesP + faceList->indexOffset;
                    const bool *faceFlag = facingFlag + faceList->indexOffset;
                    for (int j = faceList->indexCount; j; --j, ++face, ++faceFlag)
                        if (*faceFlag)
                        {
                            (*frontDest)[0] = (*face)[0];
                            (*frontDest)[1] = (*face)[1];
                            (*frontDest)[2] = (*face)[2];
                            ++shadowData.frontC;
                            ++frontDest;
                        }
                }
            }
            shadowData.indexP = new xWORD[3*shadowData.sideC + 3*shadowData.frontC];
            memcpy(shadowData.indexP, sideTrisP, shadowData.sideC*sizeof(xWORD3));
            memcpy(shadowData.indexP + shadowData.sideC*3, frontCapP, shadowData.frontC*sizeof(xWORD3));
            delete[] frontCapP;
        }
        else
        {
            shadowData.indexP = new xWORD[3*shadowData.sideC];
            memcpy(shadowData.indexP, sideTrisP, shadowData.sideC*sizeof(xWORD3));
        }
    }
}

bool xShadows_ViewportMaybeShadowed (const xElement *elem, xElementInstance &instance,
                                     const xMatrix &location, const xFieldOfView &FOV, const xLight& light)
{
    xPlane occlusionPyramid[6];
    int numPlanes = 5;

    xMatrix  mtxWorldToObject;
    mtxWorldToObject = (elem->matrix * location).invert();

    // all operations are in object space
    xVector3 vieportCorners[4];
    vieportCorners[0] = mtxWorldToObject.preTransformP(FOV.Corners3D[0]);
    vieportCorners[1] = mtxWorldToObject.preTransformP(FOV.Corners3D[1]);
    vieportCorners[2] = mtxWorldToObject.preTransformP(FOV.Corners3D[2]);
    vieportCorners[3] = mtxWorldToObject.preTransformP(FOV.Corners3D[3]);
    xVector3 viewCenter = (vieportCorners[0] + vieportCorners[2]) * 0.5f;

    occlusionPyramid[0].planeFromPoints(vieportCorners[0], vieportCorners[2], vieportCorners[1]);

    xVector3 lightPos;
    xVector3 lightDirection;
    if (light.type == xLight_INFINITE)
    {
        lightPos = mtxWorldToObject.preTransformV(light.position);
        occlusionPyramid[1].planeFromPoints(vieportCorners[0] + lightPos, vieportCorners[0], vieportCorners[1] );
        occlusionPyramid[2].planeFromPoints(vieportCorners[1] + lightPos, vieportCorners[1], vieportCorners[2] );
        occlusionPyramid[3].planeFromPoints(vieportCorners[2] + lightPos, vieportCorners[2], vieportCorners[3] );
        occlusionPyramid[4].planeFromPoints(vieportCorners[3] + lightPos, vieportCorners[3], vieportCorners[0] );
        lightDirection = -lightPos;
    }
    else
    {
        lightPos = mtxWorldToObject.preTransformP(light.position);
        occlusionPyramid[1].planeFromPoints(lightPos, vieportCorners[0], vieportCorners[1] );
        occlusionPyramid[2].planeFromPoints(lightPos, vieportCorners[1], vieportCorners[2] );
        occlusionPyramid[3].planeFromPoints(lightPos, vieportCorners[2], vieportCorners[3] );
        occlusionPyramid[4].planeFromPoints(lightPos, vieportCorners[3], vieportCorners[0] );
        lightDirection = viewCenter - lightPos;
        numPlanes++;
        occlusionPyramid[5].init(lightDirection, -xVector3::DotProduct(lightDirection, lightPos) );
    }

    if (xVector3::DotProduct(lightDirection, occlusionPyramid[0].vector3) > 0.f) // objectViewVector
        // light is behind us, flip all occlusion planes
        for (int i = 0; i < 5; ++i)
            occlusionPyramid[i].invert();

    return !instance.bbBox.culledBy(occlusionPyramid, numPlanes);
}
