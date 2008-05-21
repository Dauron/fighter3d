#include "xShadowVolume.h"

void xShadows_ExtrudePoints (const xElement *elem, bool infiniteL, const xVector3 &lightPos, xVector4 *&verticesP)
{
    if (verticesP == NULL)
        verticesP = new xVector4[elem->verticesC << 1];

    size_t stride = elem->skeletized
        ? (elem->textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel))
        : (elem->textured ? sizeof(xVertexTex)     : sizeof(xVertex));
    xVector4 *dest = verticesP;

    xBYTE *src = (xBYTE *) elem->verticesP;
    for (int i = elem->verticesC; i; --i, src += stride, ++dest)
        dest->init(*(xVector3 *)src, 1.f);

    src = (xBYTE *) elem->verticesP;
    for (int i = elem->verticesC; i; --i, src += stride, ++dest)
    {
        if (infiniteL)
            dest->init(-lightPos, 0.f);
        else
            dest->init(*(xVector3 *)src - lightPos, 0.f);
    }
}

void xShadows_GetBackFaces (const xElement *elem, const xVector4 *extrVerticesP, bool *&backFaces)
{
    if (backFaces == NULL)
        backFaces = new bool[elem->facesC];

    bool     *dest = backFaces;
    xWORD3   *face = elem->facesP;
    xVector3 *faceNormal = elem->renderData.faceNormalsP;

    extrVerticesP += elem->verticesC;
    for (int i = elem->facesC; i; --i, ++face, ++faceNormal, ++dest)
        *dest = xVector3::DotProduct((extrVerticesP + *face[0])->vector3, *faceNormal) > 0;
}

void xShadows_GetSilhouette(const xElement *elem, const bool *facingFlag, xWORD4 *&sideQadsP, xWORD3 *&backCapP, xWORD &edgesC)
{
    if (sideQadsP) delete[] sideQadsP;
    sideQadsP = new xWORD4[elem->edgesC];
    if (backCapP) delete[] backCapP;
    backCapP = new xWORD3[elem->edgesC];
    edgesC = 0;

    xEdge  *edgeIter = elem->edgesP;
    xEdge  *edgeEnd  = edgeIter + elem->edgesC;
    xWORD4 *sideDest = sideQadsP;
    xWORD3 *backDest = backCapP;

    xWORD  backFirst = elem->verticesC + edgeIter->vert1;

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
            (*backDest)[0] = backFirst;
            (*backDest)[1] = (*sideDest)[2];
            (*backDest)[2] = (*sideDest)[3];
            ++sideDest;
            ++backDest;
            ++edgesC;
        }

    if (edgesC < elem->edgesC)
    {
        sideDest = sideQadsP;
        sideQadsP = new xWORD4[edgesC];
        memcpy(sideQadsP, sideDest, edgesC*sizeof(xWORD4));
        delete[] sideDest;

        backDest = backCapP;
        backCapP = new xWORD3[edgesC];
        memcpy(backCapP, backDest, edgesC*sizeof(xWORD3));
        delete[] backDest;
    }
}

bool xShadows_ViewportMaybeShadowed (const xElement *elem, xElementInstance *instance,
                                     const xMatrix &location, const xFieldOfView *FOV, const xLight& light)
{
    xPlane occlusionPyramid[6];
    int numPlanes = 5;

    xMatrix  mtxWorldToObject;
    mtxWorldToObject = (elem->matrix * location).invert();

    // all operations are in object space
    xVector3 vieportCorners[4];
    vieportCorners[0] = mtxWorldToObject.preTransformP(FOV->Corners3D[0]);
    vieportCorners[1] = mtxWorldToObject.preTransformP(FOV->Corners3D[1]);
    vieportCorners[2] = mtxWorldToObject.preTransformP(FOV->Corners3D[2]);
    vieportCorners[3] = mtxWorldToObject.preTransformP(FOV->Corners3D[3]);
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

    return !instance->bbBox.culledBy(occlusionPyramid, numPlanes);
}