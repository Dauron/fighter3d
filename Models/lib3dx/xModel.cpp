#include "xModel.h"
#include "xUtils.h"
#include <vector>
#include "../../Utils/Debug.h"

#ifdef WIN32
#pragma warning(disable : 4996) // deprecated
#endif

/* SKELETON */
void _xElement_SkeletonAdd(xElement *elem)
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
                if (elem->renderData.verticesP == elem->verticesP)
                {
                    elem->verticesP = elem->renderData.verticesP = (xVertex*)ptrN;
                    break;
                }
                else
                    elem->renderData.verticesP = (xVertex*)ptrN;
            }
        }
    }
    for (xElement *last = elem->kidsP; last; last = last->nextP)
        _xElement_SkeletonAdd(last);
}

void  xModel :: SkeletonAdd()
{
    if (!this->spine.boneC)
    {
        this->spine.boneC = 1;
        this->spine.boneP = new xIKNode[1];
        this->spine.boneP->Zero();

        for (xElement *last = this->kidsP; last; last = last->nextP)
            _xElement_SkeletonAdd(last);
    }
}

/* BONES */
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

void   xModel :: BoneDelete(xBYTE boneId)
{
    xIKNode &bone   = spine.boneP[boneId];

    if (bone.joinsBC) {                                   // cannot delete spine root
        xIKNode &parent = spine.boneP[bone.joinsBP[0]];

        parent.JoinEDelete(boneId);

        xBYTE *joinId = bone.joinsBP;
        for (int i = bone.joinsBC; i; --i, ++joinId)
            spine.boneP[*joinId].JoinBReplace(boneId, parent.id);

        int lastId = spine.boneC-1;
        if (boneId != lastId) // if it is not the bone with the last id,
        {                     //   then we have to correct largest id (= count)
            bone = spine.boneP[lastId];
            bone.id = boneId;

            joinId = bone.joinsBP;
            for (int i = bone.joinsBC; i; --i, ++joinId)
                spine.boneP[*joinId].JoinBReplace(lastId, boneId);
            joinId = bone.joinsEP;
            for (int i = bone.joinsEC; i; --i, ++joinId)
                spine.boneP[*joinId].JoinEReplace(lastId, boneId);
        }

        --(spine.boneC);
        xIKNode *bones = new xIKNode[spine.boneC];
        memcpy(bones, spine.boneP, sizeof(xIKNode)*spine.boneC);
        delete[] spine.boneP;
        spine.boneP = bones;

        xElement *last = this->kidsP;
        for (; last; last = last->nextP)
            _xBoneDelete_CorrectElementIds(last, boneId, parent.id, lastId);
    }
}



////////////////////// xModel
void   xModel :: Free()
{
    if (!this) return;

    if (this->fileName)
        delete[] this->fileName;
    xMaterial *currM = this->materialP, *nextM;
    while (currM)
    {
        nextM = currM->nextP;
        currM->Free();
        currM = nextM;
    }
    xElement *currE = this->kidsP, *nextE;
    while (currE)
    {
        nextE = currE->nextP;
        currE->Free();
        currE = nextE;
    }
    this->spine.Clear();
    delete this;
}

xModel *xModel :: Load(const char *fileName, bool createCollisionInfo)
{
    FILE *file;
    file = fopen(fileName, "rb");
    if (file)
    {
        xModel *xmodel = new xModel();
        xmodel->fileName = strdup(fileName);
        if (xmodel)
        {
            xmodel->texturesInited = false;
            xmodel->transparent = false;
            xmodel->opaque = false;

            xDWORD len;
            fread(&len, sizeof(len), 1, file);
            fread(&xmodel->saveCollisionData, sizeof(xmodel->saveCollisionData), 1, file);

            xmodel->materialP = NULL;
            xmodel->kidsP = NULL;

            fread(&xmodel->materialC, sizeof(xBYTE), 1, file);
            if (xmodel->materialC)
            {
                xMaterial *last = NULL;
                for (int i=0; i < xmodel->materialC; ++i)
                {
                    if (last)
                        last = last->nextP = xMaterial::Load(file);
                    else
                        last = xmodel->materialP = xMaterial::Load(file);
                    if (!last)
                    {
                        xmodel->Free();
                        fclose(file);
                        return false;
                    }
                    bool transparent = last->transparency > 0.f;
                    xmodel->transparent |= transparent;
                    xmodel->opaque      |= !transparent;
                }
            }

            fread(&xmodel->kidsC, sizeof(xBYTE), 1, file);
            if (xmodel->kidsC)
            {
                xElement *last = NULL;
                for (int i=0; i < xmodel->kidsC; ++i)
                {
                    if (last)
                        last = last->nextP   = xElement::Load(file, xmodel, createCollisionInfo);
                    else
                        last = xmodel->kidsP = xElement::Load(file, xmodel, createCollisionInfo);
                    if (!last)
                    {
                        xmodel->Free();
                        fclose(file);
                        return NULL;
                    }
                }
            }
            xmodel->elementC = xmodel->kidsP->CountAll();

            bool skeletized;
            fread(&skeletized, sizeof(bool), 1, file);
            if (skeletized)
                xmodel->spine.Load(file);
        }
        fclose(file);
        return xmodel;
    }
    return NULL;
}

void   xModel :: Save()
{
    FILE *file;
    file = fopen(this->fileName, "wb");
    if (file)
    {
        xDWORD len = sizeof(this->saveCollisionData);
        fwrite(&len, sizeof(len), 1, file);
        fwrite(&this->saveCollisionData, sizeof(this->saveCollisionData), 1, file);

        fwrite(&this->materialC, sizeof(xBYTE), 1, file);
        if (this->materialC)
        {
            xMaterial *last = this->materialP;
            for (; last; last = last->nextP)
                last->Save(file);
        }

        fwrite(&this->kidsC, sizeof(xBYTE), 1, file);
        if (this->kidsC)
        {
            xElement *last = this->kidsP;
            for (; last; last = last->nextP)
                last->Save(file, this);
        }

        // are the bones defined?
        bool skeletized = this->spine.boneC;
        fwrite(&skeletized, sizeof(bool), 1, file);
        if (skeletized)
            this->spine.Save(file);

        fclose(file);
    }
}

/* model instance */
void xModelInstance :: Zero()
{
    bonesM = NULL;
    bonesQ = NULL;
    bonesC = 0;
    bonesMod = NULL;
    elementInstanceC = 0;
    elementInstanceP = NULL;
}

void xModelInstance :: ClearSkeleton()
{
    if (bonesM)   delete[] bonesM; bonesM = NULL;
    if (bonesQ)   delete[] bonesQ; bonesQ = NULL;
    if (bonesMod) delete[] bonesMod; bonesMod = NULL;
}

void xModelInstance :: Clear()
{
    ClearSkeleton();

    if (elementInstanceP)
    {
        xElementInstance *iter = elementInstanceP;
        for (; elementInstanceC; --elementInstanceC, ++iter)
            iter->Clear();

        delete[] elementInstanceP;
        elementInstanceP = NULL;
    }
}