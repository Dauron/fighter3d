#include "xUtils.h"

void _xElement_SkinElementInstance(const xElement *elem, const xMatrix *bones, const bool *boneMods,
                                   xElementInstance &instance)
{
    xWORD     count  = instance.I_vertices = elem->I_vertices;
    xBYTE    *srcV   = (xBYTE *) elem->L_vertices;
    xVector3 *srcN   = elem->renderData.L_normals;
    xDWORD    stride = elem->GetVertexStride();

    if (elem->FL_skeletized)
    {
        bool old_refill = instance.L_vertices;
        if (!old_refill)
        {
            instance.L_vertices = new xVector4[count];
            instance.L_normals  = new xVector3[count];
        }
        xVector4 *itrV = instance.L_vertices;
        xVector3 *itrN = instance.L_normals;
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
        if (!instance.L_vertices)
        {
            instance.L_vertices = new xVector4[count];
            instance.L_normals  = NULL;
            xVector4 *itrV = instance.L_vertices;
            for (int i = count; i > 0; --i, ++itrV, srcV += stride)
                itrV->init(*(xVector3 *)srcV, 1.f);
            //memcpy(dst.L_normals, elem->renderData.L_normals, sizeof(xVector3)*count);
        }
    }
}

void _xModel_SkinElementInstance(const xElement *elem, xElementInstance *instanceP,
                                 const xMatrix *bones, const bool *boneMods)
{
    for (xElement *celem = elem->L_kids; celem; celem = celem->Next)
        _xModel_SkinElementInstance(celem, instanceP, bones, boneMods);

    _xElement_SkinElementInstance(elem, bones, boneMods, instanceP[elem->ID]);
}
void xModel_SkinElementInstance(const xModel *model, xModelInstance &instance)
{
    for (xElement *celem = model->L_kids; celem; celem = celem->Next)
        _xModel_SkinElementInstance(celem, instance.L_elements, instance.MX_bones, instance.FL_modified);
}

xBoxA    _xBoundingBox(const xVector4 *vertexP, xWORD vertexC)
{
    xBoxA res;
    if (!vertexC)
    {
        res.P_min.zero();
        res.P_max.zero();
        return res;
    }

    res.P_max = res.P_min = vertexP->vector3;
    --vertexC;
    ++vertexP;

    for (; vertexC; --vertexC, ++vertexP)
    {
        if (vertexP->x > res.P_max.x) res.P_max.x = vertexP->x;
        else
        if (vertexP->x < res.P_min.x) res.P_min.x = vertexP->x;
        if (vertexP->y > res.P_max.y) res.P_max.y = vertexP->y;
        else
        if (vertexP->y < res.P_min.y) res.P_min.y = vertexP->y;
        if (vertexP->z > res.P_max.z) res.P_max.z = vertexP->z;
        else
        if (vertexP->z < res.P_min.z) res.P_min.z = vertexP->z;
    }

    return res;
}

xBoxA    _xBoundingBox(const xElement &elem)
{
    xBoxA res;
    if (!elem.I_vertices)
    {
        res.P_min.zero();
        res.P_max.zero();
        return res;
    }

    res.P_max = res.P_min = *(xPoint3*)elem.L_vertices;

    size_t stride = elem.GetVertexStride();
    xBYTE *V_curr = (xBYTE*) elem.L_vertices,
          *V_last = ((xBYTE*) elem.L_vertices) + elem.I_vertices * stride;
    V_curr += stride;

    for (; V_curr != V_last; V_curr += stride)
    {
        xPoint3 *vertexP = (xPoint3*) V_curr;
        if (vertexP->x > res.P_max.x) res.P_max.x = vertexP->x;
        else
        if (vertexP->x < res.P_min.x) res.P_min.x = vertexP->x;
        if (vertexP->y > res.P_max.y) res.P_max.y = vertexP->y;
        else
        if (vertexP->y < res.P_min.y) res.P_min.y = vertexP->y;
        if (vertexP->z > res.P_max.z) res.P_max.z = vertexP->z;
        else
        if (vertexP->z < res.P_min.z) res.P_min.z = vertexP->z;
    }

    return res;
}

void     _xElementInstance_GetBounds(const xElement &elem, xModelInstance &modelInstance)
{
    for (xElement *selem = elem.L_kids; selem; selem = selem->Next)
        _xElementInstance_GetBounds(*selem, modelInstance);

    xElementInstance &instance = modelInstance.L_elements[elem.ID];

    if (instance.bSphere_T) { delete instance.bSphere_T; instance.bSphere_T = NULL; }
    if (instance.bBox_T)    { delete instance.bBox_T; instance.bBox_T = NULL; }

    if (!elem.I_vertices)
    {
        instance.bBox.P_center.zero();
        instance.bBox.S_front = 0.f;
        instance.bBox.S_side = 0.f;
        instance.bBox.S_top = 0.f;
        instance.bSphere.P_center.zero();
        instance.bSphere.S_radius = 0.f;
        return;
    }

    xBoxA boxA;
    if (instance.L_vertices)
        boxA = _xBoundingBox(instance.L_vertices, instance.I_vertices);
    else
        boxA = _xBoundingBox(elem);
    instance.bBox.Init(boxA);
    instance.bSphere.P_center = instance.bBox.P_center;

    xFLOAT r = 0.f;
    if (instance.L_vertices)
    {
        xVector4 *V_curr = instance.L_vertices,
                 *V_last = instance.L_vertices + instance.I_vertices;

        for (; V_curr != V_last; ++V_curr)
            r = max (r, (V_curr->vector3 - instance.bSphere.P_center).lengthSqr());
    }
    else
    {
        size_t stride = elem.GetVertexStride();
        xBYTE *V_curr = (xBYTE*) elem.L_vertices,
              *V_last = ((xBYTE*) elem.L_vertices) + elem.I_vertices * stride;

        for (; V_curr != V_last; V_curr += stride)
        {
            xPoint3 &vertexP = *(xPoint3*) V_curr;
            r = max (r, (vertexP - instance.bSphere.P_center).lengthSqr());
        }
    }
    instance.bSphere.S_radius = sqrtf(r);
}

xVector3 xModel_GetBounds(const xModel &model, xModelInstance &instance)
{
    xVector3 res; res.zero();
    xDWORD   vcnt = 0;

    for (xElement *elem = model.L_kids; elem; elem = elem->Next)
        _xElementInstance_GetBounds(*elem, instance);
    /*
    xElementInstance *iter = instance.L_elements;
    for (xBYTE i = instance.I_elements; i; --i, ++iter)
    {
        _xElementInstance_GetBounds(*iter);
        vcnt += iter->I_vertices;
        res  += iter->bSphere.P_center * iter->I_vertices;
    }
    */
    if (vcnt) res /= (float)vcnt;
    return res;
}

#include <vector>
#include <algorithm>

void xElement_FreeCollisionHierarchyBounds(xCollisionData *pcData, xCollisionHierarchyBounds *hierarchyBP)
{
    if (pcData->L_kids && hierarchyBP)
    {
        xCollisionHierarchy       *hierarchy  = pcData->L_kids;
        xCollisionHierarchyBounds *hierarchyB = hierarchyBP;
        for (int i=pcData->I_kids; i; --i, ++hierarchy, ++hierarchyB)
            xElement_FreeCollisionHierarchyBounds(hierarchy, hierarchyB->kids);
        delete[] hierarchyBP;
    }
}

void xElement_CalcCollisionHierarchyBox(const xVector4* vertices,
                                        xCollisionData *pcData, xCollisionHierarchyBounds *pBound)
{
    xBoxA res;
    res.P_max.x = -1000000000.f; res.P_max.y = -1000000000.f; res.P_max.z = -1000000000.f;
    res.P_min.x =  1000000000.f; res.P_min.y =  1000000000.f; res.P_min.z =  1000000000.f;

    if (pcData->L_kids)
    {
        bool init = !pBound->kids;
        if (init)
            pBound->kids = new xCollisionHierarchyBounds[pcData->I_kids];

        xCollisionHierarchyBounds *bound     = pBound->kids;
        xCollisionHierarchy       *hierarchy = pcData->L_kids;

        for (int i=pcData->I_kids; i; --i, ++bound, ++hierarchy)
        {
            if (init)
                bound->kids = NULL;
            bound->sorted = false;

            if (hierarchy->I_kids)
              {
                xElement_CalcCollisionHierarchyBox(vertices, hierarchy, bound);
                if (bound->bounding.P_min.x < res.P_min.x) res.P_min.x = bound->bounding.P_min.x;
                if (bound->bounding.P_min.y < res.P_min.y) res.P_min.y = bound->bounding.P_min.y;
                if (bound->bounding.P_min.z < res.P_min.z) res.P_min.z = bound->bounding.P_min.z;
                if (bound->bounding.P_max.x > res.P_max.x) res.P_max.x = bound->bounding.P_max.x;
                if (bound->bounding.P_max.y > res.P_max.y) res.P_max.y = bound->bounding.P_max.y;
                if (bound->bounding.P_max.z > res.P_max.z) res.P_max.z = bound->bounding.P_max.z;
            }
            else
            if (hierarchy->L_vertices)
            {
                xWORD * iterV  = hierarchy->L_vertices;
                xWORD   vCount = hierarchy->I_vertices;

                xBoxA tmp;
#ifndef WIN32
                const xVector4 *vert = vertices + *iterV;
                ++iterV; --vCount;
                tmp.P_max.x = vert->x; tmp.P_max.y = vert->y; tmp.P_max.z = vert->z;
                tmp.P_min.x = vert->x; tmp.P_min.y = vert->y; tmp.P_min.z = vert->z;
                for (int j=vCount; j; --j, ++iterV)
                {
                    vert = vertices + *iterV;
                    if (vert->x < tmp.P_min.x) tmp.P_min.x = vert->x;
                    if (vert->x > tmp.P_max.x) tmp.P_max.x = vert->x;
                    if (vert->y < tmp.P_min.y) tmp.P_min.y = vert->y;
                    if (vert->y > tmp.P_max.y) tmp.P_max.y = vert->y;
                    if (vert->z < tmp.P_min.z) tmp.P_min.z = vert->z;
                    if (vert->z > tmp.P_max.z) tmp.P_max.z = vert->z;
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
                    // tmp.P_max.x = vert->x; tmp.P_max.y = vert->y; tmp.P_max.z = vert->z;
                    // tmp.P_min.x = vert->x; tmp.P_min.y = vert->y; tmp.P_min.z = vert->z;
                    MOV edx, dword ptr [ecx];
                    MOV tmp.P_max.x, edx;
                    MOV tmp.P_min.x, edx;
                    ADD ecx, 4;
                    MOV edx, dword ptr [ecx];
                    MOV tmp.P_max.y, edx;
                    MOV tmp.P_min.y, edx;
                    ADD ecx, 4;
                    MOV edx, dword ptr [ecx];
                    MOV tmp.P_max.z, edx;
                    MOV tmp.P_min.z, edx;

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

                        // if (vert->x > tmp.P_max.x) tmp.P_max.x = vert->x;
                        MOV    edx, dword ptr [ecx];

                        FLD    tmp.P_max.x;
                        FCOMP  dword ptr [ecx];
                        FNSTSW ax;
                        TEST   ah,5;

                        MOV    eax, tmp.P_max.x;
                        CMOVNP eax, edx;
                        MOV    tmp.P_max.x, eax;

                        // if (vert->x < tmp.P_min.x) tmp.P_min.x = vert->x;
                        FLD    tmp.P_min.x;
                        FCOMP  dword ptr [ecx];
                        FNSTSW ax;
                        TEST   ah,41h;

                        MOV    eax, tmp.P_min.x;
                        CMOVE  eax, edx;
                        MOV    tmp.P_min.x, eax;

                        // if (vert->y > tmp.P_max.y) tmp.P_max.y = vert->y;
                        ADD    ecx, 4;
                        MOV    edx, dword ptr [ecx];

                        FLD    tmp.P_max.y;
                        FCOMP  dword ptr [ecx];
                        FNSTSW ax;
                        TEST   ah,5;

                        MOV    eax, tmp.P_max.y;
                        CMOVNP eax, edx;
                        MOV    tmp.P_max.y, eax;

                        // if (vert->y < tmp.P_min.y) tmp.P_min.y = vert->y;
                        FLD    tmp.P_min.y;
                        FCOMP  dword ptr [ecx];
                        FNSTSW ax;
                        TEST   ah,41h;

                        MOV    eax, tmp.P_min.y;
                        CMOVE  eax, edx;
                        MOV    tmp.P_min.y, eax;

                        // if (vert->z > tmp.P_max.z) tmp.P_max.z = vert->z;
                        ADD    ecx, 4;
                        MOV    edx, dword ptr [ecx];

                        FLD    tmp.P_max.z;
                        FCOMP  dword ptr [ecx];
                        FNSTSW ax;
                        TEST   ah,5;

                        MOV    eax, tmp.P_max.z;
                        CMOVNP eax, edx;
                        MOV    tmp.P_max.z, eax;

                        // if (vert->z > tmp.P_max.z) tmp.P_max.z = vert->z;
                        FLD    tmp.P_min.z;
                        FCOMP  dword ptr [ecx];
                        FNSTSW ax;
                        TEST   ah,41h;

                        MOV    eax, tmp.P_min.z;
                        CMOVE  eax, edx;
                        MOV    tmp.P_min.z, eax;

                        // LOOP_FINITO
                        JMP    LOOP_COPY;
                    FIN_FIN:
                }
#endif
                bound->bounding = tmp;
                bound->center   = (tmp.P_min + tmp.P_max) * 0.5f;
                bound->radius   = (tmp.P_min - bound->center).length();
                if (tmp.P_min.x < res.P_min.x) res.P_min.x = tmp.P_min.x;
                if (tmp.P_min.y < res.P_min.y) res.P_min.y = tmp.P_min.y;
                if (tmp.P_min.z < res.P_min.z) res.P_min.z = tmp.P_min.z;
                if (tmp.P_max.x > res.P_max.x) res.P_max.x = tmp.P_max.x;
                if (tmp.P_max.y > res.P_max.y) res.P_max.y = tmp.P_max.y;
                if (tmp.P_max.z > res.P_max.z) res.P_max.z = tmp.P_max.z;
            }
            else
            if (hierarchy->L_faces)
            {
                xBoxA tmp;
                tmp.P_max.x = -1000000000.f; tmp.P_max.y = -1000000000.f; tmp.P_max.z = -1000000000.f;
                tmp.P_min.x =  1000000000.f; tmp.P_min.y =  1000000000.f; tmp.P_min.z =  1000000000.f;

                xFace ** iterF = hierarchy->L_faces;
                xBoxA    iterB;
                for (int j=hierarchy->I_faces; j; --j, ++iterF)
                {
                    const xVector4 *v1 = vertices + (**iterF)[0];
                    const xVector4 *v2 = vertices + (**iterF)[1];
                    const xVector4 *v3 = vertices + (**iterF)[2];

                    iterB.P_min.x = min(v1->x, min(v2->x, v3->x));
                    iterB.P_min.y = min(v1->y, min(v2->y, v3->y));
                    iterB.P_min.z = min(v1->z, min(v2->z, v3->z));
                    iterB.P_max.x = max(v1->x, max(v2->x, v3->x));
                    iterB.P_max.y = max(v1->y, max(v2->y, v3->y));
                    iterB.P_max.z = max(v1->z, max(v2->z, v3->z));

                    if (iterB.P_min.x < tmp.P_min.x) tmp.P_min.x = iterB.P_min.x;
                    if (iterB.P_min.y < tmp.P_min.y) tmp.P_min.y = iterB.P_min.y;
                    if (iterB.P_min.z < tmp.P_min.z) tmp.P_min.z = iterB.P_min.z;
                    if (iterB.P_max.x > tmp.P_max.x) tmp.P_max.x = iterB.P_max.x;
                    if (iterB.P_max.y > tmp.P_max.y) tmp.P_max.y = iterB.P_max.y;
                    if (iterB.P_max.z > tmp.P_max.z) tmp.P_max.z = iterB.P_max.z;
                }

                bound->bounding = tmp;
                bound->center   = (tmp.P_min + tmp.P_max) * 0.5f;
                bound->radius   = (tmp.P_min - bound->center).length();
                if (tmp.P_min.x < res.P_min.x) res.P_min.x = tmp.P_min.x;
                if (tmp.P_min.y < res.P_min.y) res.P_min.y = tmp.P_min.y;
                if (tmp.P_min.z < res.P_min.z) res.P_min.z = tmp.P_min.z;
                if (tmp.P_max.x > res.P_max.x) res.P_max.x = tmp.P_max.x;
                if (tmp.P_max.y > res.P_max.y) res.P_max.y = tmp.P_max.y;
                if (tmp.P_max.z > res.P_max.z) res.P_max.z = tmp.P_max.z;
            }
        }
    }

    pBound->bounding = res;
    pBound->center   = (res.P_min + res.P_max) * 0.5f;
    pBound->radius   = (res.P_min - pBound->center).length();
}
