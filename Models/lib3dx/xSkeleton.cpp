#include "xSkeleton.h"
/* BONE HELPERS */
xBone *xBoneByName(xBone *spine, const char *boneName)
{
    if (!strcmp(boneName, spine->name))
        return spine;
    xBone *bone = spine->kidsP, *res;
    for (; bone; bone = bone->nextP)
    {
        res = xBoneByName(bone, boneName);
        if (res) return res;
    }
    return NULL;
}

xBone *xBoneById(xBone *spine, xWORD id)
{
    if (spine->id == id)
        return spine;
    xBone *bone = spine->kidsP, *res;
    for (; bone; bone = bone->nextP)
    {
        res = xBoneById(bone, id);
        if (res) return res;
    }
    return NULL;
}

xBone *xBoneParentById(xBone *spine, xWORD id)
{
    if (spine->id == id) return NULL;
    xBone *bone = spine->kidsP, *res;
    for (; bone; bone = bone->nextP)
    {
        if (bone->id == id) return spine;
        res = xBoneParentById(bone, id);
        if (res) return res;
    }
    return NULL;
}

int    xBoneChildCount(const xBone *spine)
{
    if (spine) return spine->kidsC + xBoneChildCount(spine->kidsP) + xBoneChildCount(spine->nextP);
    else       return 0;
}

/* MATRICES */
bool   _xBoneCalculateMatrix(const xBone *bone, xMatrix &boneP, const xBone *pbone, int boneId)
{
    if (boneId == bone->id) {
        if (pbone)
            boneP  = xMatrixTranslate(pbone->ending.xyz) * xMatrixFromQuaternion(bone->quaternion)
                * xMatrixTranslate(-pbone->ending.x, -pbone->ending.y, -pbone->ending.z);
        else
            boneP = xMatrixTranslate(bone->quaternion.vector3.xyz);
        return true;
    }
    for (const xBone *cbone = bone->kidsP; cbone; cbone = cbone->nextP) {
        bool res = _xBoneCalculateMatrix(cbone, boneP, bone, boneId);
        if (res) {
            if (pbone)
                boneP = xMatrixTranslate(pbone->ending.xyz) * xMatrixFromQuaternion(bone->quaternion)
                    * xMatrixTranslate(-pbone->ending.x, -pbone->ending.y, -pbone->ending.z)
                    * boneP;
            else
                boneP = xMatrixTranslate(bone->quaternion.vector3.xyz) * boneP;
            return true;
        }
    }
    return false;
}

xMatrix xBoneCalculateMatrix(const xBone *spine, int boneId)
{
    xMatrix res;
    _xBoneCalculateMatrix(spine, res, NULL, boneId);
    return res;
}

void   _xBoneCalculateMatrices(const xBone *bone, xMatrix *&boneP, const xBone *pbone)
{
    xMatrix *boneDst = boneP + bone->id;
    xMatrix *parent  = pbone ? boneP + pbone->id : NULL;

    if (parent)
    {
        *boneDst  = *parent;
        *boneDst *= xMatrixTranslate(pbone->ending.xyz);
        *boneDst *= xMatrixFromQuaternion(bone->quaternion);
        *boneDst *= xMatrixTranslate(-pbone->ending.x, -pbone->ending.y, -pbone->ending.z);
    }
    else
        *boneDst = xMatrixTranslate(bone->quaternion.vector3.xyz);
    for (xBone *cbone = bone->kidsP; cbone; cbone = cbone->nextP)
        _xBoneCalculateMatrices(cbone, boneP, bone);
}

void    xBoneCalculateMatrices(const xBone *spine, xMatrix *&boneP, xWORD &boneC)
{
    boneC = xBoneChildCount(spine) + 1;
    if (!boneP) boneP = new xMatrix[boneC];
    _xBoneCalculateMatrices(spine, boneP, NULL);
}

void   _xBoneCalculateQuats(const xBone *bone, xVector4 *&boneP, const xBone *pBone)
{
    xVector4 *boneDst = boneP + bone->id*2;
    *(boneDst+0) = bone->quaternion;
    if (pBone)
        (boneDst+1)->init(pBone->ending, pBone->id);
    else
        (boneDst+1)->init(bone->quaternion.vector3, -1.f);
    for (xBone *cbone = bone->kidsP; cbone; cbone = cbone->nextP)
        _xBoneCalculateQuats(cbone, boneP, bone);
}

void    xBoneCalculateQuats(const xBone *spine, xVector4 *&boneP, xWORD &boneC)
{
    boneC = xBoneChildCount(spine) + 1;
    if (!boneP) boneP = new xVector4[boneC*2];
    _xBoneCalculateQuats(spine, boneP, NULL);
}
/* SKELETON */
void _xElement_AddSkeleton(xFile *file, xElement *elem)
{
    if (!elem->skeletized)
    {
        elem->skeletized = true;
        size_t strideO = elem->textured ? sizeof(xVertexTex) : sizeof(xVertex);
        size_t strideN = elem->textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel);
        for (int dataSrc = 0; dataSrc < 2; ++dataSrc)
        {
            int    count = dataSrc ? elem->verticesC : elem->renderData.verticesC;
            if (!count) continue;
            xBYTE *ptrO  = (xBYTE *) (dataSrc ? elem->verticesP : elem->renderData.verticesP);
            xBYTE *ptrN  = new xBYTE[strideN*count];
            xBYTE *oldF = ptrO, *oldE = ptrO + strideO*count, *newF = ptrN;
            for (; oldF != oldE; oldF+=strideO, newF+=strideN) {
                memcpy(newF, oldF, 3*sizeof(xFLOAT));
                if (elem->textured)
                    memcpy(newF + 7*sizeof(xFLOAT), oldF + 3*sizeof(xFLOAT), 2*sizeof(xFLOAT));
                xVertexSkel *curr = (xVertexSkel*)newF;
                curr->b0 = 0.100f;
                curr->b1 = 0.000f;
                curr->b2 = 0.000f;
                curr->b3 = 0.000f;
            }
            if (dataSrc)
            {
                delete[] elem->verticesP;
                elem->verticesP = (xVertex*)ptrN;
            }
            else
            {
                delete[] elem->renderData.verticesP;
                elem->renderData.verticesP = (xVertex*)ptrN;
            }
        }
    }
    if (elem->kidsC)
    {
        xElement *last = elem->kidsP;
        for (; last; last = last->nextP)
            _xElement_AddSkeleton(file, last);
    }
}

void  xSkeletonAdd(xFile *file)
{
    if (!file->spineP)
    {
        file->spineP = new xBone();
        xBoneReset(file->spineP);
        _xElement_AddSkeleton(file, file->firstP);
    }
}

void  xSkeletonReset(xBone *spine)
{
    if (!spine) return;
    spine->quaternion.zeroQuaternion();
    if (spine->kidsC)
        for (xBone *last = spine->kidsP; last; last = last->nextP)
            xSkeletonReset(last);
}

/* BONES */
void   xBoneReset(xBone *bone)
{
    bone->id = 0;
    bone->name = 0;
    bone->kidsC = 0;
    bone->kidsP = 0;
    bone->nextP = 0;
    bone->quaternion.zeroQuaternion();
}

xBone *xBoneAdd(xFile *model, xBone *parent, xVector3 ending)
{
    int cnt = xBoneChildCount(model->spineP)+1;

    xBone *ptr;
    ++(parent->kidsC);

    if (parent->kidsP)
    {
        for (ptr = parent->kidsP; ptr->nextP != NULL; ptr = ptr->nextP) {}
        ptr = ptr->nextP = new xBone();
    }
    else
        ptr = parent->kidsP = new xBone();

    xBoneReset(ptr);
    ptr->id = cnt;
    ptr->ending = ending;
    return ptr;
}

void  _xBoneDelete_CorrectElementIds(xElement *elem, xWORD deleteId, xWORD parentId, xWORD topId)
{
    xElement *last = elem->kidsP;
    for (; last; last = last->nextP)
        _xBoneDelete_CorrectElementIds(last, deleteId, parentId, topId);

    if (elem->skeletized)
    {
        xDWORD stride = elem->textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel);
        for (int dataSrc = 0; dataSrc < 2; ++dataSrc)
        {
            int    count = dataSrc ? elem->verticesC : elem->renderData.verticesC;
            xBYTE *src   = (xBYTE *) (dataSrc ? elem->verticesP : elem->renderData.verticesP);
            for (int i = count; i; --i, src += stride)
            {
                xVertexSkel *vert = (xVertexSkel *)src;
                int i0 = (int) floor(vert->b0);
                int i1 = (int) floor(vert->b1);
                int i2 = (int) floor(vert->b2);
                int i3 = (int) floor(vert->b3);

                if (i0 == deleteId) vert->b0 += -i0 + parentId; // vertices from deleted bone are reassigned to parent
                else
                if (i0 == topId)    vert->b0 += -i0 + deleteId; // vertices from top bone are reassigned to its new id

                if (i1 == deleteId) vert->b1 += -i1 + parentId; // vertices from deleted bone are reassigned to parent
                else
                if (i1 == topId)    vert->b1 += -i1 + deleteId; // vertices from top bone are reassigned to its new id

                if (i2 == deleteId) vert->b2 += -i2 + parentId; // vertices from deleted bone are reassigned to parent
                else
                if (i2 == topId)    vert->b2 += -i2 + deleteId; // vertices from top bone are reassigned to its new id

                if (i3 == deleteId) vert->b3 += -i3 + parentId; // vertices from deleted bone are reassigned to parent
                else
                if (i3 == topId)    vert->b3 += -i3 + deleteId; // vertices from top bone are reassigned to its new id

                i0 = (int) floor(vert->b0);
                i1 = (int) floor(vert->b1);
                i2 = (int) floor(vert->b2);
                i3 = (int) floor(vert->b3);

                if (i0 == i3)
                {
                    vert->b0 += vert->b3 - i3;
                    vert->b3 = 0.f;      i3 = -1;
                }
                if (i0 == i2)
                {
                    vert->b0 += vert->b2 - i2;
                    vert->b2 = vert->b3; i2 = i3;
                    vert->b3 = 0.f;      i3 = -1;
                }
                if (i0 == i1)
                {
                    vert->b0 += vert->b1 - i1;
                    vert->b1 = vert->b2; i1 = i2;
                    vert->b2 = vert->b3; i2 = i3;
                    vert->b3 = 0.f;      i3 = -1;
                }
                if (i1 >= 0 && i1 == i3)
                {
                    vert->b1 += vert->b3 - i3;
                    vert->b3 = 0.f;      i3 = -1;
                }
                if (i1 >= 0 && i1 == i2)
                {
                    vert->b1 += vert->b2 - i2;
                    vert->b2 = vert->b3; i2 = i3;
                    vert->b3 = 0.f;      i3 = -1;
                }
                if (i2 >= 0 && i2 == i3)
                {
                    vert->b2 += vert->b3 - i3;
                    vert->b3 = 0.f;      i3 = -1;
                }
            }
        }
    }
}

void   xBoneDelete(xFile *model, xBone *boneToDel)
{
    xBone *parent = xBoneParentById(model->spineP, boneToDel->id);

    if (parent) {                                   // cannot delete spine root
        int cnt = xBoneChildCount(model->spineP);

        if (parent->kidsP == boneToDel)             // if first bone is the del bone
            parent->kidsP = boneToDel->nextP;       //   remove it from the list

        if (parent->kidsP) {
            xBone *ptr = parent->kidsP;
            while (ptr->nextP) {             // move through the whole list
                if (ptr->nextP == boneToDel)        //   if next bone is the del bone
                    ptr->nextP = boneToDel->nextP;  //     remove it from the list
                else
                    ptr = ptr->nextP;               //   check next bone on the list
            }
            ptr->nextP = boneToDel->kidsP;          // move kids to the parent bone
        }
        else
            parent->kidsP = boneToDel->kidsP;       // move kids to the parent bone

        parent->kidsC += boneToDel->kidsC - 1;      // add node's kids, but remove node

        if (boneToDel->id != cnt) // if it is not the bone with the last id,
        {                         //   then we have to correct largest id (= count)
            xBoneById(model->spineP, cnt)->id = boneToDel->id;

            xElement *last = model->firstP;
            for (; last; last = last->nextP)
                _xBoneDelete_CorrectElementIds(last, boneToDel->id, parent->id, cnt);
        }
    }
}
