#include "xUtils.h"

const xBYTE MAX_HIERARCHY_DEPTH = 10;

xVector4 * xElement_GetSkinnedVertices(const xElement *elem, const xMatrix *bones, bool fromRenderData)
{
    xWORD  count  = fromRenderData ? elem->renderData.verticesC : elem->verticesC;
    xBYTE *src    = (xBYTE *) (fromRenderData ? elem->renderData.verticesP : elem->verticesP);
    xDWORD stride = elem->skeletized
        ? (elem->textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel))
        : (elem->textured ? sizeof(xVertexTex)     : sizeof(xVertex));

    xVector4 *dst = new xVector4[count];
    xVector4 *itr = dst;

    if (elem->skeletized)
        for (int i = count; i > 0; --i, ++itr, src += stride)
        {
            xVertexSkel *vert = (xVertexSkel *)src;

            int   i1 = (int) floor(vert->b0);
            float w1 = (vert->b0 - i1)*10;
            int   i2 = (int) floor(vert->b1);
            float w2 = (vert->b1 - i2)*10;
            int   i3 = (int) floor(vert->b2);
            float w3 = (vert->b2 - i3)*10;
            int   i4 = (int) floor(vert->b3);
            float w4 = (vert->b3 - i4)*10;

            xVector4 vec; vec.init(* (xVector3 *)vert->pos, 1.f);
            *itr  = bones[i1] * vec * w1;
            if (w2 > 0.01) *itr += bones[i2] * vec * w2;
            if (w3 > 0.01) *itr += bones[i3] * vec * w3;
            if (w4 > 0.01) *itr += bones[i4] * vec * w4;
        }
    else
        for (int i = count; i > 0; --i, ++itr, src += stride)
        {
            xVertex *vert = (xVertex *)src;
            itr->init(* (xVector3 *)vert->pos, 1.f);
        }
    return dst;
}

xSkinnedData xElement_GetSkinnedElement(const xElement *elem, const xMatrix *bones)
{
    xWORD     count  = elem->renderData.verticesC;
    xBYTE    *srcV   = (xBYTE *) elem->renderData.verticesP;
    xVector3 *srcN   = elem->renderData.normalP;
    xDWORD stride = elem->skeletized
        ? (elem->textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel))
        : (elem->textured ? sizeof(xVertexTex)     : sizeof(xVertex));

    xSkinnedData dst;
    dst.verticesP = new xVector3[count];
    dst.normalsP  = new xVector3[count];
    xVector3 *itrV = dst.verticesP;
    xVector3 *itrN = dst.normalsP;

    if (elem->skeletized)
        for (int i = count; i > 0; --i, ++itrV, ++itrN, srcV += stride, ++srcN)
        {
            xVertexSkel *vert = (xVertexSkel *)srcV;

            xVector4 vec;  vec.init(* (xVector3 *)vert->pos, 1.f);
            xVector4 nor;  nor.init(* srcN, 0.f);
            xMatrix  bone;

            itrV->init(0.f, 0.f, 0.f);
            itrN->init(0.f, 0.f, 0.f);
            for (int b=0; b<4; ++b)
            {
                int   i = (int) floor(vert->bone[b]);
                float w = (vert->bone[b] - i)*10;
                bone = bones[i] * w;
                *itrV  += (bone * vec).vector3;
                *itrN  += (bone * nor).vector3;
            }
        }
    else
    {
        for (int i = count; i > 0; --i, ++itrV, srcV += stride)
            *itrV = *(xVector3 *)srcV;
        memcpy(dst.normalsP, elem->renderData.normalP, sizeof(xVector3)*count);
    }
    return dst;
}

xVector4 * xElement_GetSkinnedVertices(const xElement *elem, const xMatrix *bones, xMatrix transformation,
                                       xVector4 *&dst, bool fromRenderData)
{
    xWORD  count  = fromRenderData ? elem->renderData.verticesC : elem->verticesC;
    xBYTE *src    = (xBYTE *) (fromRenderData ? elem->renderData.verticesP : elem->verticesP);
    xDWORD stride = elem->skeletized
        ? (elem->textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel))
        : (elem->textured ? sizeof(xVertexTex)     : sizeof(xVertex));

    if (!dst) dst = new xVector4[count];
    xVector4 *itr = dst;

    transformation *= elem->matrix;

    if (elem->skeletized && bones)
        for (int i = count; i > 0; --i, ++itr, src += stride)
        {
            xVertexSkel *vert = (xVertexSkel *)src;

            int   i1 = (int) floor(vert->b0);
            float w1 = (vert->b0 - i1)*10;
            int   i2 = (int) floor(vert->b1);
            float w2 = (vert->b1 - i2)*10;
            int   i3 = (int) floor(vert->b2);
            float w3 = (vert->b2 - i3)*10;
            int   i4 = (int) floor(vert->b3);
            float w4 = (vert->b3 - i4)*10;

            xVector4 vec; vec.init(* (xVector3 *)vert->pos, 1.f);
            *itr  = bones[i1] * vec * w1;
            if (w2 > 0.01) *itr += bones[i2] * vec * w2;
            if (w3 > 0.01) *itr += bones[i3] * vec * w3;
            if (w4 > 0.01) *itr += bones[i4] * vec * w4;
            *itr = *itr * transformation;
        }
    else
        for (int i = count; i > 0; --i, ++itr, src += stride)
        {
            xVertex *vert = (xVertex *)src;
            *itr = xVector4::Create(* (xVector3 *)vert->pos, 1.f) * transformation;
        }
    return dst;
}

xVector3 xCenterOfTheMass(const xElement *elem, const xMatrix *bones)
{
    xVector3 res; res.zero();
    if (!elem->renderData.verticesC) return res;

    if (elem->skeletized && bones)
    {
        xVector4 *verts = xElement_GetSkinnedVertices(elem, bones, false);
        xVector4 *iter = verts;

        for (int i=0; i < elem->renderData.verticesC; ++i, ++iter)
            res += iter->vector3;

        delete[] verts;
    }
    else
    {
        int stride = 0;
        if (!elem->skeletized)
            stride = (elem->textured) ? sizeof(xVertexTex) : sizeof(xVertex);
        else
            stride = (elem->textured) ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel);

        xBYTE *ptr = (xBYTE *)elem->renderData.verticesP;
        for (int i=0; i<elem->renderData.verticesC; ++i, ptr += stride)
            res += *((xVector3*)ptr);
    }

    res /= elem->renderData.verticesC;
    return res;
}

void _xCenterOfTheModelMass(const xElement *elem, const xMatrix *bones, xVector3 &res, xDWORD &vcnt)
{
    for (xElement *celem = elem->kidsP; celem; celem = celem->nextP)
        _xCenterOfTheModelMass(celem, bones, res, vcnt);
    if (!elem->verticesC) return;

    vcnt += elem->verticesC;
    if (elem->skeletized && bones)
    {
        xVector4 *verts = xElement_GetSkinnedVertices(elem, bones, false);
        xVector4 *iter = verts;

        for (int i=0; i < elem->renderData.verticesC; ++i, ++iter)
            res += iter->vector3;

        delete[] verts;
    }
    else
    {
        int stride = 0;
        if (!elem->skeletized)
            stride = (elem->textured) ? sizeof(xVertexTex) : sizeof(xVertex);
        else
            stride = (elem->textured) ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel);

        xBYTE *ptr = (xBYTE *)elem->verticesP;
        for (int i=0; i<elem->verticesC; ++i, ptr += stride)
            res += *((xVector3*)ptr);
    }
}

xVector3 xCenterOfTheModelMass(const xFile *file, const xMatrix *bones)
{
    xVector3 res; res.zero();
    xDWORD   vcnt = 0;
    for (xElement *elem = file->firstP; elem; elem = elem->nextP)
        _xCenterOfTheModelMass(elem, bones, res, vcnt);
    if (vcnt)
        res /= (float)vcnt;
    return res;
}

void   _xCenterOfTheModel(const xElement *elem, const xMatrix *bones,
                            xVector3 &min, xVector3 &max, bool recursive = true)
{
    if (recursive)
        for (xElement *celem = elem->kidsP; celem; celem = celem->nextP)
            _xCenterOfTheModel(celem, bones, min, max);

    if (!elem->renderData.verticesC) return;

    if (elem->skeletized && bones)
    {
        xVector4 *verts = xElement_GetSkinnedVertices(elem, bones, false);
        xVector4 *iter = verts;

        for (int i=0; i < elem->verticesC; ++i, ++iter)
        {
            if (iter->x > max.x) max.x = iter->x;
            if (iter->x < min.x) min.x = iter->x;
            if (iter->y > max.y) max.y = iter->y;
            if (iter->y < min.y) min.y = iter->y;
            if (iter->z > max.z) max.z = iter->z;
            if (iter->z < min.z) min.z = iter->z;
        }

        delete[] verts;
    }
    else
    {
        int stride = 0;
        if (!elem->skeletized)
            stride = (elem->textured) ? sizeof(xVertexTex) : sizeof(xVertex);
        else
            stride = (elem->textured) ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel);

        xBYTE *ptr = (xBYTE *)elem->renderData.verticesP;
        for (int i=0; i<elem->renderData.verticesC; ++i, ptr += stride)
        {
            xVector3 *iter = (xVector3*)ptr;
            if (iter->x > max.x) max.x = iter->x;
            if (iter->x < min.x) min.x = iter->x;
            if (iter->y > max.y) max.y = iter->y;
            if (iter->y < min.y) min.y = iter->y;
            if (iter->z > max.z) max.z = iter->z;
            if (iter->z < min.z) min.z = iter->z;
        }
    }
}

xVector3 xCenterOfTheElement(const xElement *elem, const xMatrix *bones)
{
    xVector3 minv; minv.init( 1000000000.f,  1000000000.f,  1000000000.f);
    xVector3 maxv; maxv.init(-1000000000.f, -1000000000.f, -1000000000.f);
    _xCenterOfTheModel(elem, bones, minv, maxv, false);
    return (minv + maxv) / 2.f;
}

xVector3 xCenterOfTheModel(const xFile *file, const xMatrix *bones)
{
    xVector3 minv; minv.init( 1000000000.f,  1000000000.f,  1000000000.f);
    xVector3 maxv; maxv.init(-1000000000.f, -1000000000.f, -1000000000.f);

    for (xElement *elem = file->firstP; elem; elem = elem->nextP)
        _xCenterOfTheModel(elem, bones, minv, maxv);

    return (minv + maxv) / 2.f;
}

xVector3 xCenterOfTheMass(const xVector4* vertices, xDWORD count, bool scale)
{
    xVector3 res; res.init(0.f,0.f,0.f);
    if (!count) return res;

    const xVector4 *iter = vertices;
    for (int i=count; i; --i, ++iter)
        res += iter->vector3;

    if (scale)
        return res / (float)count;
    return res;
}

xBox xBoundingBox(const xVector4* vertices, xDWORD count)
{
    xBox res;
    res.max.init(-1000000000.f, -1000000000.f, -1000000000.f);
    res.min.init( 1000000000.f,  1000000000.f,  1000000000.f);

    for (; count; --count, ++vertices)
    {
        if (vertices->x > res.max.x) res.max.x = vertices->x;
        if (vertices->y > res.max.y) res.max.y = vertices->y;
        if (vertices->z > res.max.z) res.max.z = vertices->z;
        if (vertices->x < res.min.x) res.min.x = vertices->x;
        if (vertices->y < res.min.y) res.min.y = vertices->y;
        if (vertices->z < res.min.z) res.min.z = vertices->z;
    }

    return res;
}


#include "xSkeleton.h"
#include <vector>
#include <algorithm>

void xElement_CreateHierarchyFromVertices(const xElement                   *elem,
                                          xCollisionHierarchy              *baseHierarchy,
                                          std::vector<xBox>                &cBoundings,
                                          std::vector<xCollisionHierarchy> &cHierarchy)
{
    std::vector<std::vector<xWORD3*> > cFaces;
    std::vector<std::vector<xDWORD> >  cVerts;
    std::vector<xDWORD>::iterator      found;
    cFaces.resize(cBoundings.size());
    cVerts.resize(cBoundings.size());

    xBYTE *src    = (xBYTE *) elem->verticesP;
    xDWORD stride = elem->skeletized
        ? (elem->textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel))
        : (elem->textured ? sizeof(xVertexTex)     : sizeof(xVertex));

    if (baseHierarchy)
    {
        xWORD3 **iterF = baseHierarchy->facesP;
        for (int i = 0; i < baseHierarchy->facesC; ++i, ++iterF)
            for (int j = cBoundings.size()-1; j >= 0 ; --j)
            {
                const xBox &bounding = cBoundings[j];
                const xVector3 *vert1 = (xVector3*) &((xVertex*) (src + stride*(**iterF)[0]))->pos;
                const xVector3 *vert2 = (xVector3*) &((xVertex*) (src + stride*(**iterF)[1]))->pos;
                const xVector3 *vert3 = (xVector3*) &((xVertex*) (src + stride*(**iterF)[2]))->pos;

                if (bounding.Contains(*vert1) ||
                    bounding.Contains(*vert2) ||
                    bounding.Contains(*vert3))
                {
                    cFaces[j].push_back(*iterF);

                    found = std::find<std::vector<xDWORD>::iterator, xDWORD>
                                        (cVerts[j].begin(), cVerts[j].end(), (**iterF)[0]);
                    if (found == cVerts[j].end()) cVerts[j].push_back((**iterF)[0]);
                    found = std::find<std::vector<xDWORD>::iterator, xDWORD>
                                        (cVerts[j].begin(), cVerts[j].end(), (**iterF)[0]);
                    if (found == cVerts[j].end()) cVerts[j].push_back((**iterF)[0]);
                    found = std::find<std::vector<xDWORD>::iterator, xDWORD>
                                        (cVerts[j].begin(), cVerts[j].end(), (**iterF)[0]);
                    if (found == cVerts[j].end()) cVerts[j].push_back((**iterF)[0]);
                    break;
                }
            }
    }
    else
    {
        xWORD3 *iterF = elem->facesP;
        for (int i = 0; i < elem->facesC; ++i, ++iterF)
            for (int j = cBoundings.size()-1; j >= 0 ; --j)
            {
                const xBox &bounding = cBoundings[j];
                const xVector3 *vert1 = (xVector3*) &((xVertex*) (src + stride*(*iterF)[0]))->pos;
                const xVector3 *vert2 = (xVector3*) &((xVertex*) (src + stride*(*iterF)[1]))->pos;
                const xVector3 *vert3 = (xVector3*) &((xVertex*) (src + stride*(*iterF)[2]))->pos;

                if (bounding.Contains(*vert1) ||
                    bounding.Contains(*vert2) ||
                    bounding.Contains(*vert3))
                {
                    cFaces[j].push_back(iterF);

                    found = std::find<std::vector<xDWORD>::iterator, xDWORD>
                                        (cVerts[j].begin(), cVerts[j].end(), (*iterF)[0]);
                    if (found == cVerts[j].end()) cVerts[j].push_back((*iterF)[0]);
                    found = std::find<std::vector<xDWORD>::iterator, xDWORD>
                                        (cVerts[j].begin(), cVerts[j].end(), (*iterF)[0]);
                    if (found == cVerts[j].end()) cVerts[j].push_back((*iterF)[0]);
                    found = std::find<std::vector<xDWORD>::iterator, xDWORD>
                                        (cVerts[j].begin(), cVerts[j].end(), (*iterF)[0]);
                    if (found == cVerts[j].end()) cVerts[j].push_back((*iterF)[0]);
                    break;
                }
            }
    }

    size_t size = cFaces.size();
    for (size_t j = 0; j < size; ++j)
    {
        xCollisionHierarchy hierarchy;
        hierarchy.kidsC = 0;
        hierarchy.kidsP = NULL;
        hierarchy.facesC = cFaces[j].size();
        hierarchy.verticesC = cVerts[j].size();

        if (hierarchy.verticesC)
        {
            xDWORD *iterHV = hierarchy.verticesP = new xDWORD[hierarchy.verticesC];
            std::vector<xDWORD>::iterator endBV = cVerts[j].end();
            for (std::vector<xDWORD>::iterator iterBV = cVerts[j].begin();
                 iterBV != endBV; ++iterBV, ++iterHV)
                *iterHV = *iterBV;
        }
        else
            hierarchy.verticesP = NULL;

        if (hierarchy.facesC)
        {
            xWORD3 **iterHF = hierarchy.facesP = new xWORD3*[hierarchy.facesC];
            std::vector<xWORD3*>::iterator endBF = cFaces[j].end();
            for (std::vector<xWORD3*>::iterator iterBF = cFaces[j].begin();
                 iterBF != endBF; ++iterBF, ++iterHF)
                *iterHF = *iterBF;
        }
        else
            hierarchy.facesP = NULL;

        cHierarchy.push_back(hierarchy);
    }
}

void xElement_SubdivideCollisionHierarchy(const xElement *elem, float scale,
                                          xCollisionHierarchy &hierarchy, int depth,
                                          const xBox &bounding)
{
    float dx = bounding.max.x - bounding.min.x;
    float dy = bounding.max.y - bounding.min.y;
    float dz = bounding.max.z - bounding.min.z;
    float d  = max (dx, max(dy, dz));
    float w  = d / scale;
    float wx = min (w, dx);
    float wy = min (w, dy);
    float wz = min (w, dz);
    int   cx = (int)ceil(dx / wx);
    int   cy = (int)ceil(dy / wy);
    int   cz = (int)ceil(dz / wz);

    std::vector<xBox> cBoundings;
    cBoundings.resize(cx*cy*cz);

    for (int ix = 0; ix < cx; ++ix)
        for (int iy = 0; iy < cy; ++iy)
            for (int iz = 0; iz < cz; ++iz)
            {
                int hrch = ix + cx * (iy + cy * iz);
                xBox &cBounding = cBoundings[hrch];
                cBounding.min.x = ix * wx + bounding.min.x;
                cBounding.min.y = iy * wy + bounding.min.y;
                cBounding.min.z = iz * wz + bounding.min.z;
                cBounding.max.x = (ix+1 != cx) ? (ix+1) * wx + bounding.min.x : bounding.max.x;
                cBounding.max.y = (iy+1 != cy) ? (iy+1) * wy + bounding.min.y : bounding.max.y;
                cBounding.max.z = (iz+1 != cz) ? (iz+1) * wz + bounding.min.z : bounding.max.z;
            }

    std::vector<xCollisionHierarchy> cHierarchy;
    xElement_CreateHierarchyFromVertices(elem, &hierarchy, cBoundings, cHierarchy);

    if (depth < MAX_HIERARCHY_DEPTH)
        for (size_t i = 0; i<cHierarchy.size(); ++i)
            if (cHierarchy[i].facesC > 20)
            {
                float scale = cHierarchy[i].facesC / 20.f;
                if (scale < 2.f)  scale = 2.f;
                if (scale > 10.f) scale = 10.f;
                xElement_SubdivideCollisionHierarchy(elem, scale, cHierarchy[i], depth+1, cBoundings[i]);
            }

    for (size_t i = 0; i<cHierarchy.size(); ++i)
        if (cHierarchy[i].facesC == 0)
        {
            cHierarchy[i--] = *cHierarchy.rbegin();
            cHierarchy.resize(cHierarchy.size()-1);
        }

    if (cHierarchy.empty())
        return;

    delete[] hierarchy.facesP;
    hierarchy.facesP = NULL;
    delete[] hierarchy.verticesP;
    hierarchy.verticesP = NULL;
    if (cHierarchy.size() == 1)
    {
        hierarchy.verticesP = cHierarchy[0].verticesP;
        hierarchy.facesP = cHierarchy[0].facesP;
        hierarchy.kidsC  = cHierarchy[0].kidsC;
        hierarchy.kidsP  = cHierarchy[0].kidsP;
    }
    else
    {
        hierarchy.kidsC = cHierarchy.size();
        hierarchy.kidsP = new xCollisionHierarchy[cHierarchy.size()];
        for (size_t i = 0; i<cHierarchy.size(); ++i)
            hierarchy.kidsP[i] = cHierarchy[i];
    }
}

void xElement_GetCollisionHierarchy(const xFile *file, xElement *elem)
{
    if (!elem->verticesC)
    {
        elem->collisionData.kidsP = NULL;
        elem->collisionData.kidsC = 0;
        return;
    }
    if (elem->facesC < 20)
    {
        elem->collisionData.kidsP = new xCollisionHierarchy[1];
        elem->collisionData.kidsC = 1;
        elem->collisionData.kidsP->facesC = elem->facesC;
        elem->collisionData.kidsP->facesP = new xWORD3*[elem->facesC];
        elem->collisionData.kidsP->verticesC = elem->verticesC;
        elem->collisionData.kidsP->verticesP = new xDWORD[elem->verticesC];
        elem->collisionData.kidsP->kidsC = 0;
        elem->collisionData.kidsP->kidsP = NULL;

        xWORD3 **iterHF = elem->collisionData.kidsP->facesP;
        xWORD3  *iterF  = elem->facesP;
        for (int i = elem->facesC; i; --i, ++iterHF, ++iterF)
            *iterHF = iterF;

        xDWORD   *iterHV = elem->collisionData.kidsP->verticesP;
        for (xDWORD i = 0; i < elem->verticesC; ++i, ++iterHV)
            *iterHV = i;
        return;
    }

    xBYTE *src    = (xBYTE *) elem->verticesP;
    xDWORD stride = elem->skeletized
        ? (elem->textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel))
        : (elem->textured ? sizeof(xVertexTex)     : sizeof(xVertex));
    std::vector<xCollisionHierarchy> cHierarchy;

    if (!elem->skeletized)
    {
        xCollisionHierarchy hierarchy;
        hierarchy.kidsC = 0;
        hierarchy.kidsP = NULL;
        hierarchy.facesC    = elem->facesC;
        hierarchy.verticesC = elem->verticesC;
        xWORD3  **iterHF = hierarchy.facesP    = new xWORD3*[elem->facesC];
        xDWORD   *iterHV = hierarchy.verticesP = new xDWORD[elem->verticesC];
        xWORD3   *iterF  = elem->facesP;

        xBox bounding;
        bounding.max.x = -1000000000.f; bounding.max.y = -1000000000.f; bounding.max.z = -1000000000.f;
        bounding.min.x =  1000000000.f; bounding.min.y =  1000000000.f; bounding.min.z =  1000000000.f;

        for (int i = elem->facesC; i; --i, ++iterHF, ++iterF)
        {
            *iterHF = iterF;

            const xVector3 *v1 = (xVector3*) &((xVertex*) (src + stride*(*iterF)[0]))->pos;
            const xVector3 *v2 = (xVector3*) &((xVertex*) (src + stride*(*iterF)[1]))->pos;
            const xVector3 *v3 = (xVector3*) &((xVertex*) (src + stride*(*iterF)[2]))->pos;

            float minx = min(v1->x, min(v2->x, v3->x));
            float miny = min(v1->y, min(v2->y, v3->y));
            float minz = min(v1->z, min(v2->z, v3->z));
            float maxx = max(v1->x, max(v2->x, v3->x));
            float maxy = max(v1->y, max(v2->y, v3->y));
            float maxz = max(v1->z, max(v2->z, v3->z));

            if (minx < bounding.min.x) bounding.min.x = minx;
            if (miny < bounding.min.y) bounding.min.y = miny;
            if (minz < bounding.min.z) bounding.min.z = minz;
            if (maxx > bounding.max.x) bounding.max.x = maxx;
            if (maxy > bounding.max.y) bounding.max.y = maxy;
            if (maxz > bounding.max.z) bounding.max.z = maxz;
        }

        for (xDWORD i = 0; i < elem->verticesC; ++i, ++iterHV)
            *iterHV = i;

        xElement_SubdivideCollisionHierarchy(elem, 5.f, hierarchy, 1, bounding);
        cHierarchy.push_back(hierarchy);
    }
    else
    {
        elem->collisionData.kidsC = xBoneChildCount(file->spineP)+1;

        std::vector<xBox> cBoundings;
        cBoundings.resize(elem->collisionData.kidsC);

        xBYTE *iterV = src;
        for (xDWORD i = elem->verticesC; i; --i, iterV += stride)
        {
            xVertexSkel *vert = (xVertexSkel *) iterV;

            int   bone   = (int) floor(vert->b0);
            float weight = (vert->b0 - bone)*10;

            int   bi = (int) floor(vert->b1);
            float bw = (vert->b1 - bi)*10;
            if (bw > weight) { bone = bi; weight = bw; }

            bi = (int) floor(vert->b2);
            bw = (vert->b2 - bi)*10;
            if (bw > weight) { bone = bi; weight = bw; }

            bi = (int) floor(vert->b3);
            bw = (vert->b3 - bi)*10;
            if (bw > weight) { bone = bi; weight = bw; }

            xBox &bounding = cBoundings[bone];
            if (vert->x < bounding.min.x) bounding.min.x = vert->x;
            if (vert->y < bounding.min.y) bounding.min.y = vert->y;
            if (vert->z < bounding.min.z) bounding.min.z = vert->z;
            if (vert->x > bounding.max.x) bounding.max.x = vert->x;
            if (vert->y > bounding.max.y) bounding.max.y = vert->y;
            if (vert->z > bounding.max.z) bounding.max.z = vert->z;
        }

        xElement_CreateHierarchyFromVertices(elem, NULL, cBoundings, cHierarchy);

        for (size_t i = 0; i<cHierarchy.size(); ++i)
            if (cHierarchy[i].facesC > 20 && cHierarchy[i].kidsC == 0)
            {
                float scale = cHierarchy[i].facesC / 20.f;
                if (scale < 2.0f) scale = 2.0f;
                if (scale > 10.f) scale = 10.f;
                xElement_SubdivideCollisionHierarchy(elem, scale, cHierarchy[i], 1, cBoundings[i]);
            }
    }

    for (size_t i = 0; i<cHierarchy.size(); ++i)
        if (cHierarchy[i].facesC == 0)
        {
            cHierarchy[i--] = *cHierarchy.rbegin();
            cHierarchy.resize(cHierarchy.size()-1);
        }
    elem->collisionData.kidsC = cHierarchy.size();
    elem->collisionData.kidsP = new xCollisionHierarchy[cHierarchy.size()];
    for (size_t i = 0; i<cHierarchy.size(); ++i)
        elem->collisionData.kidsP[i] = cHierarchy[i];
}

void xElement_FreeCollisionHierarchy(xCollisionData *pcData)
{
    if (pcData->kidsP)
    {
        for (int i=pcData->kidsC-1; i >= 0; --i)
        {
            xCollisionHierarchy *ch = pcData->kidsP + i;
            if (ch->facesP)    delete[] ch->facesP;
            if (ch->verticesP) delete[] ch->verticesP;
            xElement_FreeCollisionHierarchy(ch);
        }
        delete[] pcData->kidsP;
    }
}
void xElement_FreeCollisionHierarchyBounds(xCollisionData *pcData, xCollisionHierarchyBounds *hierarchyBP)
{
    if (pcData->kidsP && hierarchyBP)
    {
        xCollisionHierarchy       *hierarchy  = pcData->kidsP;
        xCollisionHierarchyBounds *hierarchyB = hierarchyBP;
        for (int i=pcData->kidsC; i; --i, ++hierarchy, ++hierarchyB)
            xElement_FreeCollisionHierarchyBounds(hierarchy, hierarchyB->kids);
        delete[] hierarchyBP;
    }
}

void xElement_CalcCollisionHierarchyBox(const xVector4* vertices,
                                        xCollisionData *pcData, xCollisionHierarchyBounds *pBound)
{
    xBox res;
    res.max.x = -1000000000.f; res.max.y = -1000000000.f; res.max.z = -1000000000.f;
    res.min.x =  1000000000.f; res.min.y =  1000000000.f; res.min.z =  1000000000.f;

    if (pcData->kidsP)
    {
        bool init = !pBound->kids;
        if (init)
            pBound->kids = new xCollisionHierarchyBounds[pcData->kidsC];

        xCollisionHierarchyBounds *bound     = pBound->kids;
        xCollisionHierarchy       *hierarchy = pcData->kidsP;

        for (int i=pcData->kidsC; i; --i, ++bound, ++hierarchy)
        {
            if (init)
                bound->kids = NULL;
            bound->sorted = false;

            if (hierarchy->kidsC)
              {
                xElement_CalcCollisionHierarchyBox(vertices, hierarchy, bound);
                if (bound->bounding.min.x < res.min.x) res.min.x = bound->bounding.min.x;
                if (bound->bounding.min.y < res.min.y) res.min.y = bound->bounding.min.y;
                if (bound->bounding.min.z < res.min.z) res.min.z = bound->bounding.min.z;
                if (bound->bounding.max.x > res.max.x) res.max.x = bound->bounding.max.x;
                if (bound->bounding.max.y > res.max.y) res.max.y = bound->bounding.max.y;
                if (bound->bounding.max.z > res.max.z) res.max.z = bound->bounding.max.z;
            }
            else
            if (hierarchy->verticesP)
            {
                xDWORD * iterV  = hierarchy->verticesP;
                xDWORD   vCount = hierarchy->verticesC;

                xBox tmp;
#ifndef WIN32
                const xVector4 *vert = vertices + *iterV;
                ++iterV; --vCount;
                tmp.max.x = vert->x; tmp.max.y = vert->y; tmp.max.z = vert->z;
                tmp.min.x = vert->x; tmp.min.y = vert->y; tmp.min.z = vert->z;
                for (int j=vCount; j; --j, ++iterV)
                {
                    vert = vertices + *iterV;
                    if (vert->x < tmp.min.x) tmp.min.x = vert->x;
                    else
                    if (vert->x > tmp.max.x) tmp.max.x = vert->x;
                    if (vert->y < tmp.min.y) tmp.min.y = vert->y;
                    else
                    if (vert->y > tmp.max.y) tmp.max.y = vert->y;
                    if (vert->z < tmp.min.z) tmp.min.z = vert->z;
                    else
                    if (vert->z > tmp.max.z) tmp.max.z = vert->z;
                }
#else
                __asm // eax = tmp->?, ebx = iterV, ecx = vert, edx = vert->?
                {
                    // const xVector4 *vert = vertices + *iterV;
                    MOV ebx, iterV;
                    MOV ecx, dword ptr [ebx];
                    SHL ecx, 4; // sizeof (xVector4)
                    ADD ecx, vertices;
                    // tmp.max.x = vert->x; tmp.max.y = vert->y; tmp.max.z = vert->z;
                    // tmp.min.x = vert->x; tmp.min.y = vert->y; tmp.min.z = vert->z;
                    MOV edx, dword ptr [ecx];
                    MOV tmp.max.x, edx;
                    MOV tmp.min.x, edx;
                    ADD ecx, 4;
                    MOV edx, dword ptr [ecx];
                    MOV tmp.max.y, edx;
                    MOV tmp.min.y, edx;
                    ADD ecx, 4;
                    MOV edx, dword ptr [ecx];
                    MOV tmp.max.z, edx;
                    MOV tmp.min.z, edx;

                LOOP_COPY:
                    DEC vCount;
                    CMP vCount, 0;
                    JZ  FIN_FIN;

                        // vert = vertices + *iterV;
                        ADD ebx, 4;
                        MOV ecx, dword ptr [ebx];
                        SHL ecx, 4; // sizeof (xVector4)
                        ADD ecx, vertices;

                        // if (vert->x > tmp.max.x) tmp.max.x = vert->x;
                        MOV    edx, dword ptr [ecx];

                        FLD    tmp.max.x;
                        FCOMP  dword ptr [ecx];
                        FNSTSW ax;
                        TEST   ah,5;

                        MOV    eax, tmp.max.x;
                        CMOVNP eax, edx;
                        MOV    tmp.max.x, eax;

                        // if (vert->x < tmp.min.x) tmp.min.x = vert->x;
                        FLD    tmp.min.x;
                        FCOMP  dword ptr [ecx];
                        FNSTSW ax;
                        TEST   ah,41h;

                        MOV    eax, tmp.min.x;
                        CMOVE  eax, edx;
                        MOV    tmp.min.x, eax;

                        // if (vert->y > tmp.max.y) tmp.max.y = vert->y;
                        ADD    ecx, 4;
                        MOV    edx, dword ptr [ecx];

                        FLD    tmp.max.y;
                        FCOMP  dword ptr [ecx];
                        FNSTSW ax;
                        TEST   ah,5;

                        MOV    eax, tmp.max.y;
                        CMOVNP eax, edx;
                        MOV    tmp.max.y, eax;

                        // if (vert->y < tmp.min.y) tmp.min.y = vert->y;
                        FLD    tmp.min.y;
                        FCOMP  dword ptr [ecx];
                        FNSTSW ax;
                        TEST   ah,41h;

                        MOV    eax, tmp.min.y;
                        CMOVE  eax, edx;
                        MOV    tmp.min.y, eax;

                        // if (vert->z > tmp.max.z) tmp.max.z = vert->z;
                        ADD    ecx, 4;
                        MOV    edx, dword ptr [ecx];

                        FLD    tmp.max.z;
                        FCOMP  dword ptr [ecx];
                        FNSTSW ax;
                        TEST   ah,5;

                        MOV    eax, tmp.max.z;
                        CMOVNP eax, edx;
                        MOV    tmp.max.z, eax;

                        // if (vert->z > tmp.max.z) tmp.max.z = vert->z;
                        FLD    tmp.min.z;
                        FCOMP  dword ptr [ecx];
                        FNSTSW ax;
                        TEST   ah,41h;

                        MOV    eax, tmp.min.z;
                        CMOVE  eax, edx;
                        MOV    tmp.min.z, eax;

                        // LOOP_FINITO
                        JMP    LOOP_COPY;
                    FIN_FIN:
                }
#endif
                bound->bounding = tmp;
                bound->center   = (tmp.min + tmp.max) * 0.5f;
                bound->radius   = (tmp.min - bound->center).length();
                if (tmp.min.x < res.min.x) res.min.x = tmp.min.x;
                if (tmp.min.y < res.min.y) res.min.y = tmp.min.y;
                if (tmp.min.z < res.min.z) res.min.z = tmp.min.z;
                if (tmp.max.x > res.max.x) res.max.x = tmp.max.x;
                if (tmp.max.y > res.max.y) res.max.y = tmp.max.y;
                if (tmp.max.z > res.max.z) res.max.z = tmp.max.z;
            }
            else
            if (hierarchy->facesP)
            {
                xBox tmp;
                tmp.max.x = -1000000000.f; tmp.max.y = -1000000000.f; tmp.max.z = -1000000000.f;
                tmp.min.x =  1000000000.f; tmp.min.y =  1000000000.f; tmp.min.z =  1000000000.f;

                xWORD3 ** iterF = hierarchy->facesP;
                xBox      iterB;
                for (int j=hierarchy->facesC; j; --j, ++iterF)
                {
                    const xVector4 *v1 = vertices + (**iterF)[0];
                    const xVector4 *v2 = vertices + (**iterF)[1];
                    const xVector4 *v3 = vertices + (**iterF)[2];

                    iterB.min.x = min(v1->x, min(v2->x, v3->x));
                    iterB.min.y = min(v1->y, min(v2->y, v3->y));
                    iterB.min.z = min(v1->z, min(v2->z, v3->z));
                    iterB.max.x = max(v1->x, max(v2->x, v3->x));
                    iterB.max.y = max(v1->y, max(v2->y, v3->y));
                    iterB.max.z = max(v1->z, max(v2->z, v3->z));

                    if (iterB.min.x < tmp.min.x) tmp.min.x = iterB.min.x;
                    if (iterB.min.y < tmp.min.y) tmp.min.y = iterB.min.y;
                    if (iterB.min.z < tmp.min.z) tmp.min.z = iterB.min.z;
                    if (iterB.max.x > tmp.max.x) tmp.max.x = iterB.max.x;
                    if (iterB.max.y > tmp.max.y) tmp.max.y = iterB.max.y;
                    if (iterB.max.z > tmp.max.z) tmp.max.z = iterB.max.z;
                }

                bound->bounding = tmp;
                bound->center   = (tmp.min + tmp.max) * 0.5f;
                bound->radius   = (tmp.min - bound->center).length();
                if (tmp.min.x < res.min.x) res.min.x = tmp.min.x;
                if (tmp.min.y < res.min.y) res.min.y = tmp.min.y;
                if (tmp.min.z < res.min.z) res.min.z = tmp.min.z;
                if (tmp.max.x > res.max.x) res.max.x = tmp.max.x;
                if (tmp.max.y > res.max.y) res.max.y = tmp.max.y;
                if (tmp.max.z > res.max.z) res.max.z = tmp.max.z;
            }
        }
    }

    pBound->bounding = res;
    pBound->center   = (res.min + res.max) * 0.5f;
    pBound->radius   = (res.min - pBound->center).length();
}
