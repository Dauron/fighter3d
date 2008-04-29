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

            xVector4 vec; vec.Init(* (xVector3 *)vert->pos, 1.f);
            *itr  = bones[i1] * vec * w1;
            if (w2 > 0.01) *itr += bones[i2] * vec * w2;
            if (w3 > 0.01) *itr += bones[i3] * vec * w3;
            if (w4 > 0.01) *itr += bones[i4] * vec * w4;
        }
    else
        for (int i = count; i > 0; --i, ++itr, src += stride)
        {
            xVertex *vert = (xVertex *)src;
            itr->Init(* (xVector3 *)vert->pos, 1.f);
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

            xVector4 vec; vec.Init(* (xVector3 *)vert->pos, 1.f);
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
    xVector3 res; res.Init(0.f,0.f,0.f);
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

        for (int i=0; i < elem->renderData.verticesC; ++i, ++iter)
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
    xVector3 minv; minv.Init( 1000000000.f,  1000000000.f,  1000000000.f);
    xVector3 maxv; maxv.Init(-1000000000.f, -1000000000.f, -1000000000.f);
    _xCenterOfTheModel(elem, bones, minv, maxv, false);
    return (minv + maxv) / 2.f;
}

xVector3 xCenterOfTheModel(const xFile *file, const xMatrix *bones)
{
    xVector3 minv; minv.Init( 1000000000.f,  1000000000.f,  1000000000.f);
    xVector3 maxv; maxv.Init(-1000000000.f, -1000000000.f, -1000000000.f);

    for (xElement *elem = file->firstP; elem; elem = elem->nextP)
        _xCenterOfTheModel(elem, bones, minv, maxv);

    return (minv + maxv) / 2.f;
}

xVector3 xCenterOfTheMass(const xVector4* vertices, xDWORD count, bool scale)
{
    xVector3 res; res.Init(0.f,0.f,0.f);
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
    res.max.x = -1000000000.f; res.max.y = -1000000000.f; res.max.z = -1000000000.f;
    res.min.x =  1000000000.f; res.min.y =  1000000000.f; res.min.z =  1000000000.f;

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
    cFaces.resize(cBoundings.size());

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
            if (cHierarchy[i].facesC > 150)
            {
                float scale = cHierarchy[i].facesC / 150.f;
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
    if (cHierarchy.size() == 1)
    {
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
        elem->collisionData.hierarchyP = NULL;
        elem->collisionData.hierarchyC = 0;
        return;
    }
    if (elem->facesC < 100)
    {
        elem->collisionData.hierarchyP = new xCollisionHierarchy[1];
        elem->collisionData.hierarchyC = 1;
        elem->collisionData.hierarchyP->facesC = elem->facesC;
        elem->collisionData.hierarchyP->facesP = new xWORD3*[elem->facesC];
        elem->collisionData.hierarchyP->kidsC = 0;
        elem->collisionData.hierarchyP->kidsP = NULL;

        xWORD3 **iterHF = elem->collisionData.hierarchyP->facesP;
        xWORD3  *iterF  = elem->facesP;
        for (int i = elem->facesC; i; --i, ++iterHF, ++iterF)
            *iterHF = iterF;
        return;
    }

    std::vector<xCollisionHierarchy> cHierarchy;

    xBYTE *src    = (xBYTE *) elem->verticesP;
    xDWORD stride = elem->skeletized
        ? (elem->textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel))
        : (elem->textured ? sizeof(xVertexTex)     : sizeof(xVertex));

    if (!elem->skeletized)
    {
        xCollisionHierarchy hierarchy;
        hierarchy.kidsC = 0;
        hierarchy.kidsP = NULL;
        hierarchy.facesC = elem->facesC;
        xWORD3 **iterHF = hierarchy.facesP = new xWORD3*[elem->facesC];
        xWORD3  *iterF  = elem->facesP;

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

        xElement_SubdivideCollisionHierarchy(elem, 5.f, hierarchy, 1, bounding);
        cHierarchy.push_back(hierarchy);
    }
    else
    {
        elem->collisionData.hierarchyC = xBoneChildCount(file->spineP)+1;

        std::vector<xBox> cBoundings;
        cBoundings.resize(elem->collisionData.hierarchyC);

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
            if (cHierarchy[i].facesC > 150 && cHierarchy[i].kidsC == 0)
            {
                float scale = cHierarchy[i].facesC / 150.f;
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
    elem->collisionData.hierarchyC = cHierarchy.size();
    elem->collisionData.hierarchyP = new xCollisionHierarchy[cHierarchy.size()];
    for (size_t i = 0; i<cHierarchy.size(); ++i)
        elem->collisionData.hierarchyP[i] = cHierarchy[i];
}

void xElement_FreeCollisionHierarchy(xCollisionHierarchy *hierarchyP, xWORD hierarchyC)
{
    if (hierarchyP)
    {
        for (int i=hierarchyC-1; i >= 0; --i)
        {
            if (hierarchyP[i].facesP) delete[] hierarchyP[i].facesP;
            xElement_FreeCollisionHierarchy(hierarchyP[i].kidsP, hierarchyP[i].kidsC);
        }
        delete[] hierarchyP;
    }
}
void xElement_FreeCollisionHierarchyBounds(xCollisionHierarchy *hierarchyP, xWORD hierarchyC,
                                           xCollisionHierarchyBounds *hierarchyBP)
{
    if (hierarchyP && hierarchyBP)
    {
        for (int i=hierarchyC-1; i >= 0; --i)
        {
            xElement_FreeCollisionHierarchyBounds(hierarchyP[i].kidsP, hierarchyP[i].kidsC, hierarchyBP[i].kids);
            delete[] hierarchyBP[i].faceBoundings;
        }
        delete[] hierarchyBP;
    }
}

xBox xElement_CalcCollisionHierarchyBox(const xVector4* vertices,
                                        xCollisionHierarchy *hierarchyP, xWORD hierarchyC,
                                        xCollisionHierarchyBounds *&bounds)
{
    xBox res;
    res.max.x = -1000000000.f; res.max.y = -1000000000.f; res.max.z = -1000000000.f;
    res.min.x =  1000000000.f; res.min.y =  1000000000.f; res.min.z =  1000000000.f;

    if (hierarchyP)
    {
        bool init = !bounds;
        if (init)
            bounds = new xCollisionHierarchyBounds[hierarchyC];

        for (int i=0; i<hierarchyC; ++i)
        {
            if (init)
                bounds[i].kids = NULL;

            if (hierarchyP[i].kidsC)
            {
                bounds[i].faceBoundings = NULL;
                xBox tmp = xElement_CalcCollisionHierarchyBox(vertices, hierarchyP[i].kidsP, hierarchyP[i].kidsC, bounds[i].kids);

                bounds[i].bounding = tmp;
                if (tmp.min.x < res.min.x) res.min.x = tmp.min.x;
                if (tmp.min.y < res.min.y) res.min.y = tmp.min.y;
                if (tmp.min.z < res.min.z) res.min.z = tmp.min.z;
                if (tmp.max.x > res.max.x) res.max.x = tmp.max.x;
                if (tmp.max.y > res.max.y) res.max.y = tmp.max.y;
                if (tmp.max.z > res.max.z) res.max.z = tmp.max.z;
            }
            else
            if (hierarchyP[i].facesP)
            {
                if (init)
                    bounds[i].faceBoundings = new xBox[hierarchyP[i].facesC];

                xBox tmp;
                tmp.max.x = -1000000000.f; tmp.max.y = -1000000000.f; tmp.max.z = -1000000000.f;
                tmp.min.x =  1000000000.f; tmp.min.y =  1000000000.f; tmp.min.z =  1000000000.f;

                xWORD3 ** iterF = hierarchyP[i].facesP;
                xBox    * iterB = bounds[i].faceBoundings;
                for (int j=hierarchyP[i].facesC; j; --j, ++iterF, ++iterB)
                {
                    const xVector4 *v1 = vertices + (**iterF)[0];
                    const xVector4 *v2 = vertices + (**iterF)[1];
                    const xVector4 *v3 = vertices + (**iterF)[2];

                    iterB->min.x = min(v1->x, min(v2->x, v3->x));
                    iterB->min.y = min(v1->y, min(v2->y, v3->y));
                    iterB->min.z = min(v1->z, min(v2->z, v3->z));
                    iterB->max.x = max(v1->x, max(v2->x, v3->x));
                    iterB->max.y = max(v1->y, max(v2->y, v3->y));
                    iterB->max.z = max(v1->z, max(v2->z, v3->z));

                    if (iterB->min.x < tmp.min.x) tmp.min.x = iterB->min.x;
                    if (iterB->min.y < tmp.min.y) tmp.min.y = iterB->min.y;
                    if (iterB->min.z < tmp.min.z) tmp.min.z = iterB->min.z;
                    if (iterB->max.x > tmp.max.x) tmp.max.x = iterB->max.x;
                    if (iterB->max.y > tmp.max.y) tmp.max.y = iterB->max.y;
                    if (iterB->max.z > tmp.max.z) tmp.max.z = iterB->max.z;
                }

                bounds[i].bounding = tmp;
                if (tmp.min.x < res.min.x) res.min.x = tmp.min.x;
                if (tmp.min.y < res.min.y) res.min.y = tmp.min.y;
                if (tmp.min.z < res.min.z) res.min.z = tmp.min.z;
                if (tmp.max.x > res.max.x) res.max.x = tmp.max.x;
                if (tmp.max.y > res.max.y) res.max.y = tmp.max.y;
                if (tmp.max.z > res.max.z) res.max.z = tmp.max.z;
            }
            else
                bounds[i].faceBoundings = NULL;
        }
    }

    return res;
}
