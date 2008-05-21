#include "xUtils.h"

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

void _xElement_SkinElementInstance(const xElement *elem, const xMatrix *bones, const bool *boneMods,
                                   xElementInstance &instance)
{
    xWORD     count  = instance.verticesC = elem->verticesC;
    xBYTE    *srcV   = (xBYTE *) elem->verticesP;
    xVector3 *srcN   = elem->renderData.normalP;
    xDWORD stride = elem->skeletized
        ? (elem->textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel))
        : (elem->textured ? sizeof(xVertexTex)     : sizeof(xVertex));

    if (elem->skeletized)
    {
        bool old_refill = instance.verticesP;
        if (!old_refill)
        {
            instance.verticesP = new xVector4[count];
            instance.normalsP  = new xVector3[count];
        }
        xVector4 *itrV = instance.verticesP;
        xVector3 *itrN = instance.normalsP;
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
        if (!instance.verticesP)
        {
            instance.verticesP = new xVector4[count];
            instance.normalsP  = NULL;
            xVector4 *itrV = instance.verticesP;
            for (int i = count; i > 0; --i, ++itrV, srcV += stride)
                itrV->init(*(xVector3 *)srcV, 1.f);
            //memcpy(dst.normalsP, elem->renderData.normalP, sizeof(xVector3)*count);
        }
    }
}

void _xModel_SkinElementInstance(const xElement *elem, xElementInstance *instanceP,
                                 const xMatrix *bones, const bool *boneMods)
{
    for (xElement *celem = elem->kidsP; celem; celem = celem->nextP)
        _xModel_SkinElementInstance(celem, instanceP, bones, boneMods);

    _xElement_SkinElementInstance(elem, bones, boneMods, instanceP[elem->id]);
}
void xModel_SkinElementInstance(const xModel *model, xModelInstance &instance)
{
    for (xElement *celem = model->kidsP; celem; celem = celem->nextP)
        _xModel_SkinElementInstance(celem, instance.elementInstanceP, instance.bonesM, instance.bonesMod);
}

xBox     _xBoundingBox(const xVector4 *vertexP, xWORD vertexC)
{
    xBox res;
    if (!vertexC)
    {
        res.min.zero();
        res.max.zero();
        return res;
    }

    res.min = vertexP->vector3;
    res.max = vertexP->vector3;
    --vertexC;

    for (; vertexC; --vertexC, ++vertexP)
    {
        if (vertexP->x > res.max.x) res.max.x = vertexP->x;
        else
        if (vertexP->x < res.min.x) res.min.x = vertexP->x;
        if (vertexP->y > res.max.y) res.max.y = vertexP->y;
        else
        if (vertexP->y < res.min.y) res.min.y = vertexP->y;
        if (vertexP->z > res.max.z) res.max.z = vertexP->z;
        else
        if (vertexP->z < res.min.z) res.min.z = vertexP->z;
    }

    return res;
}

void     _xElementInstance_GetBounds(xElementInstance &instance)
{
    if (!instance.verticesC)
    {
        instance.bbBox.min.zero();
        instance.bbBox.max.zero();
        instance.bsCenter.zero();
        instance.bsRadius = 0.f;
        return;
    }

    instance.bbBox    = _xBoundingBox(instance.verticesP, instance.verticesC);
    instance.bsCenter = (instance.bbBox.min + instance.bbBox.max) * 0.5f;

    xVector4 *iter = instance.verticesP;
    xFLOAT r = 0.f;
    for (xWORD i = instance.verticesC; i; --i, ++iter)
        r = max (r, (iter->vector3 - instance.bsCenter).lengthSqr());
    instance.bsRadius = sqrtf(r);
}

xVector3 xModel_GetBounds(xModelInstance &instance)
{
    xVector3 res; res.zero();
    xDWORD   vcnt = 0;

    xElementInstance *iter = instance.elementInstanceP;
    for (xBYTE i = instance.elementInstanceC; i; --i, ++iter)
    {
        _xElementInstance_GetBounds(*iter);
        vcnt += iter->verticesC;
        res  += iter->bsCenter*iter->verticesC;
    }

    if (vcnt) res /= (float)vcnt;
    return res;
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
                xWORD * iterV  = hierarchy->verticesP;
                xWORD   vCount = hierarchy->verticesC;

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
                    if (vert->x > tmp.max.x) tmp.max.x = vert->x;
                    if (vert->y < tmp.min.y) tmp.min.y = vert->y;
                    if (vert->y > tmp.max.y) tmp.max.y = vert->y;
                    if (vert->z < tmp.min.z) tmp.min.z = vert->z;
                    if (vert->z > tmp.max.z) tmp.max.z = vert->z;
                }
#else
                __asm // eax = tmp->?, ebx = iterV, ecx = vert, edx = vert->?
                {
                    // const xVector4 *vert = vertices + *iterV;
                    MOV ebx, iterV;
                    MOV ecx, 0
                    MOV cx, word ptr [ebx];
                    SHL ecx, 4; // =*16B
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
                        MOV ecx, 0
                        MOV cx, word ptr [ebx];
                        SHL ecx, 4; // =*16B
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
