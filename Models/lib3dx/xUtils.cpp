#include "xUtils.h"

xVector3 * xElement_GetSkinnedVertices(const xElement *elem, const xMatrix *bones, bool fromRenderData)
{
    xWORD  count  = fromRenderData ? elem->renderData.verticesC : elem->verticesC;
    xBYTE *src    = (xBYTE *) (fromRenderData ? elem->renderData.verticesP : elem->verticesP);
    xDWORD stride = elem->skeletized
        ? (elem->textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel))
        : (elem->textured ? sizeof(xVertexTex)     : sizeof(xVertex));

    xVector3 *dst = new xVector3[count];
    xVector3 *itr = dst;

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

            *itr  = bones[i1].postTransformP(*(xVector3 *)vert->pos) * w1;
            if (w2 > 0.01) *itr += bones[i2].postTransformP(*(xVector3 *)vert->pos) * w2;
            if (w3 > 0.01) *itr += bones[i3].postTransformP(*(xVector3 *)vert->pos) * w3;
            if (w4 > 0.01) *itr += bones[i4].postTransformP(*(xVector3 *)vert->pos) * w4;
        }
    else
        for (int i = count; i > 0; --i, ++itr, src += stride)
        {
            xVertex *vert = (xVertex *)src;
            *itr = *(xVector3 *)vert->pos;
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

void xElement_GetSkinnedElementForShadow(const xElement *elem, const xMatrix *bones, const bool *boneMods,
                                         bool infiniteL, xShadowData &shadowData)
{
    xWORD     count  = elem->verticesC;
    xBYTE    *srcV   = (xBYTE *) elem->verticesP;
    xVector3 *srcN   = elem->renderData.normalP;
    xDWORD stride = elem->skeletized
        ? (elem->textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel))
        : (elem->textured ? sizeof(xVertexTex)     : sizeof(xVertex));

    if (elem->skeletized)
    {
        bool old_refill = shadowData.verticesP;
        if (!old_refill)
        {
            shadowData.verticesP = (infiniteL) ? new xVector4[count + 1] : new xVector4[count << 1];
            shadowData.normalsP  = new xVector3[count];
        }
        xVector4 *itrV = shadowData.verticesP;
        xVector3 *itrN = shadowData.normalsP;
        for (int i = count; i > 0; --i, ++itrV, ++itrN, srcV += stride, ++srcN)
        {
            xVertexSkel *vert = (xVertexSkel *)srcV;

            xVector4 vec;  vec.init(* (xVector3 *)vert->pos, 1.f);
            xMatrix  bone;

            int   idx[4];
            idx[0] = (int) floor(vert->bone[0]);
            idx[1] = (int) floor(vert->bone[1]);
            idx[2] = (int) floor(vert->bone[2]);
            idx[3] = (int) floor(vert->bone[3]);
            float wght[4];
            wght[0] = (vert->bone[0] - idx[0]);
            wght[1] = (vert->bone[1] - idx[1]);
            wght[2] = (vert->bone[2] - idx[2]);
            wght[3] = (vert->bone[3] - idx[3]);

            if (old_refill)
                if (!boneMods[idx[0]] &&
                    (!boneMods[idx[1]] || wght[1] < 0.001f) &&
                    (!boneMods[idx[2]] || wght[2] < 0.001f) &&
                    (!boneMods[idx[3]] || wght[3] < 0.001f))
                    continue; // vertex has not changed

            itrV->init(0.f, 0.f, 0.f, 0.f);
            itrN->init(0.f, 0.f, 0.f);
            for (int b=0; b<4; ++b)
            {
                bone = bones[idx[b]] * wght[b]*10;
                *itrV  += bone * vec;
                *itrN  += bone.postTransformV(*srcN);
            }
        }
    }
    else
    {
        if (!shadowData.verticesP)
        {
            shadowData.verticesP = (infiniteL) ? new xVector4[count + 1] : new xVector4[count << 1];
            shadowData.normalsP  = NULL;
            xVector4 *itrV = shadowData.verticesP;
            for (int i = count; i > 0; --i, ++itrV, srcV += stride)
                itrV->init(*(xVector3 *)srcV, 1.f);
            //memcpy(dst.normalsP, elem->renderData.normalP, sizeof(xVector3)*count);
        }
    }
}

xVector3 * xElement_GetSkinnedVertices(const xElement *elem, const xMatrix *bones, xMatrix transformation,
                                       xVector3 *&dst, bool fromRenderData)
{
    xWORD  count  = fromRenderData ? elem->renderData.verticesC : elem->verticesC;
    xBYTE *src    = (xBYTE *) (fromRenderData ? elem->renderData.verticesP : elem->verticesP);
    xDWORD stride = elem->skeletized
        ? (elem->textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel))
        : (elem->textured ? sizeof(xVertexTex)     : sizeof(xVertex));

    if (!dst) dst = new xVector3[count];
    xVector3 *itr = dst;

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

            *itr  = bones[i1].postTransformP(*(xVector3 *)vert->pos) * w1;
            if (w2 > 0.01) *itr += bones[i2].postTransformP(*(xVector3 *)vert->pos) * w2;
            if (w3 > 0.01) *itr += bones[i3].postTransformP(*(xVector3 *)vert->pos) * w3;
            if (w4 > 0.01) *itr += bones[i4].postTransformP(*(xVector3 *)vert->pos) * w4;
            *itr = transformation.preTransformP(*itr);
        }
    else
        for (int i = count; i > 0; --i, ++itr, src += stride)
        {
            xVertex *vert = (xVertex *)src;
            *itr = transformation.preTransformP(*(xVector3 *)vert->pos);
        }
    return dst;
}

void _xCenterOfTheModelMass(const xElement *elem, const xMatrix *bones, xVector3 &res, xDWORD &vcnt)
{
    for (xElement *celem = elem->kidsP; celem; celem = celem->nextP)
        _xCenterOfTheModelMass(celem, bones, res, vcnt);
    if (!elem->verticesC) return;

    vcnt += elem->verticesC;
    if (elem->skeletized && bones)
    {
        xVector3 *verts = xElement_GetSkinnedVertices(elem, bones, false);
        xVector3 *iter = verts;

        for (int i=0; i < elem->verticesC; ++i, ++iter)
            res += *iter;

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

xVector3 xCenterOfTheModelMass(const xModel *file, const xMatrix *bones)
{
    xVector3 res; res.zero();
    xDWORD   vcnt = 0;
    for (xElement *elem = file->kidsP; elem; elem = elem->nextP)
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
        xVector3 *verts = xElement_GetSkinnedVertices(elem, bones, false);
        xVector3 *iter = verts;

        for (int i=0; i < elem->verticesC; ++i, ++iter)
        {
            if (iter->x > max.x) max.x = iter->x;
            else
            if (iter->x < min.x) min.x = iter->x;
            if (iter->y > max.y) max.y = iter->y;
            else
            if (iter->y < min.y) min.y = iter->y;
            if (iter->z > max.z) max.z = iter->z;
            else
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

xVector3 xCenterOfTheModel(const xModel *file, const xMatrix *bones)
{
    xVector3 minv; minv.init( 1000000000.f,  1000000000.f,  1000000000.f);
    xVector3 maxv; maxv.init(-1000000000.f, -1000000000.f, -1000000000.f);

    for (xElement *elem = file->kidsP; elem; elem = elem->nextP)
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

xVector3 xMeshMassCenter(const xBYTE *vertexP, const size_t stride, xDWORD vertexC)
{
    xVector3 res; res.zero();
    if (!vertexC) return res;

    for (; vertexC; --vertexC, vertexP += stride)
        res += *(xVector3*)vertexP;
    return res / (xFLOAT)vertexC;
}

xBox     xBoundingBox(const xBYTE *vertexP, const size_t stride, xDWORD vertexC)
{
    xBox res;
    if (!vertexC)
    {
        res.min.zero();
        res.max.zero();
        return res;
    }

    res.min = *(xVector3*)vertexP;
    res.max = *(xVector3*)vertexP;
    --vertexC;

    for (; vertexC; --vertexC, vertexP += stride)
    {
        if (((xVector3*)vertexP)->x > res.max.x) res.max.x = ((xVector3*)vertexP)->x;
        else
        if (((xVector3*)vertexP)->x < res.min.x) res.min.x = ((xVector3*)vertexP)->x;
        if (((xVector3*)vertexP)->y > res.max.y) res.max.y = ((xVector3*)vertexP)->y;
        else
        if (((xVector3*)vertexP)->y < res.min.y) res.min.y = ((xVector3*)vertexP)->y;
        if (((xVector3*)vertexP)->z > res.max.z) res.max.z = ((xVector3*)vertexP)->z;
        else
        if (((xVector3*)vertexP)->z < res.min.z) res.min.z = ((xVector3*)vertexP)->z;
    }

    return res;
}

xFLOAT   xBoundingSphereRadius(const xBYTE *vertexP, const size_t stride, xDWORD vertexC, const xVector3 &sCenter)
{
    if (!vertexC) return 0.f;

    xFLOAT r = 0.f;
    for (; vertexC; --vertexC, vertexP += stride)
        r = max (r, (*(xVector3*)vertexP - sCenter).lengthSqr());
    return sqrtf(r);
}

void     xElement_GetBounds(const xElement &elem, const xMatrix *boneP, xElementInstance *instanceP, bool recursive = false)
{
    if (recursive)
        for (xElement *celem = elem.kidsP; celem; celem = celem->nextP)
            xElement_GetBounds(*celem, boneP, instanceP, true);

    xElementInstance &instance = instanceP[elem.id];

    if (!elem.verticesC)
    {
        instance.bbBox.min.zero();
        instance.bbBox.max.zero();
        instance.bsCenter.zero();
        instance.bsRadius = 0.f;
        return;
    }

    if (elem.skeletized && boneP)
    {
        xVector3 *verts   = xElement_GetSkinnedVertices(&elem, boneP, false);
        instance.bbBox    = xBoundingBox((xBYTE*)verts, sizeof(xVector3), elem.verticesC);
        instance.bsCenter = (instance.bbBox.min + instance.bbBox.max) * 0.5f;
        instance.bsRadius = xBoundingSphereRadius((xBYTE*)verts, sizeof(xVector3), elem.verticesC, instance.bsCenter);
        delete[] verts;
    }
    else
    {
        int stride;
        if (!elem.skeletized)
            stride = (elem.textured) ? sizeof(xVertexTex) : sizeof(xVertex);
        else
            stride = (elem.textured) ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel);
        instance.bbBox = xBoundingBox((xBYTE*)elem.verticesP, stride, elem.verticesC);
        instance.bsCenter = (instance.bbBox.min + instance.bbBox.max) * 0.5f;
        instance.bsRadius = xBoundingSphereRadius((xBYTE*)elem.verticesP, stride, elem.verticesC, instance.bsCenter);
    }
}

void     xModel_GetBounds(const xModel *model, const xMatrix *boneP, xElementInstance *instanceP)
{
    for (xElement *celem = model->kidsP; celem; celem = celem->nextP)
        xElement_GetBounds(*celem, boneP, instanceP, true);
}

#include "xBone.h"
#include <vector>
#include <algorithm>

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

void xElement_CalcCollisionHierarchyBox(const xVector3* vertices,
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
                xWORD * iterV  = hierarchy->verticesP;
                xWORD   vCount = hierarchy->verticesC;

                xBox tmp;
#ifndef WIN32
                const xVector3 *vert = vertices + *iterV;
                ++iterV; --vCount;
                tmp.max.x = vert->x; tmp.max.y = vert->y; tmp.max.z = vert->z;
                tmp.min.x = vert->x; tmp.min.y = vert->y; tmp.min.z = vert->z;
                for (int j=vCount; j; --j, ++iterV)
                {
                    vert = vertices + *iterV;
                    if (vert->x < tmp.min.x) tmp.min.x = vert->x;
                    if (vert->x > tmp.max.x) tmp.max.x = vert->x;
                    if (vert->y < tmp.min.y) tmp.min.y = vert->y;
                    if (vert->y > tmp.max.y) tmp.max.y = vert->y;
                    if (vert->z < tmp.min.z) tmp.min.z = vert->z;
                    if (vert->z > tmp.max.z) tmp.max.z = vert->z;
                }
#else
                __asm // eax = tmp->?, ebx = iterV, ecx = vert, edx = vert->?
                {
                    // const xVector3 *vert = vertices + *iterV;
                    MOV ebx, iterV;
                    MOV eax, 0
                    MOV ax, word ptr [ebx];
                    SHL eax, 2; // =*4B
                    MOV ecx, eax;
                    SHL eax, 1; // =*8B
                    ADD ecx, eax; // sizeof (xVector3) = 8B+4B=12B
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
                        ADD ebx, 2;
                        MOV eax, 0
                        MOV ax, word ptr [ebx];
                        SHL eax, 2; // =*4B
                        MOV ecx, eax;
                        SHL eax, 1; // =*8B
                        ADD ecx, eax; // sizeof (xVector3) = 8B+4B=12B
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

                xFace ** iterF = hierarchy->facesP;
                xBox      iterB;
                for (int j=hierarchy->facesC; j; --j, ++iterF)
                {
                    const xVector3 *v1 = vertices + (**iterF)[0];
                    const xVector3 *v2 = vertices + (**iterF)[1];
                    const xVector3 *v3 = vertices + (**iterF)[2];

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
