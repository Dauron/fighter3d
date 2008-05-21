#include "xBone.h"
#include "xModel.h"

////////////////////// xBone
void  xBone :: ResetQ()
{
    this->quaternion.zeroQ();
    for (xBone *last = this->kidsP; last; last = last->nextP)
        last->ResetQ();
}

void   xBone :: Zero()
{
    memset(this, 0, sizeof(xBone));
    this->quaternion.w = 1.f;
    this->modified = true;
}
void   xBone :: Free()
{
    if (this->name)
        delete[] this->name;
    xBone *curr = this->kidsP, *next;
    while (curr)
    {
        next = curr->nextP;
        curr->Free();
        curr = next;
    }
    delete this;
}

xBone *xBone :: Clone() const
{
    xBone *boneDst = new xBone();
    if (boneDst)
    {
        boneDst->ending = this->ending;
        boneDst->id     = this->id;
        boneDst->kidsC  = this->kidsC;
        if (this->name)
            boneDst->name = strdup(this->name);
        else
            boneDst->name = NULL;
        boneDst->quaternion = this->quaternion;
        boneDst->kidsP = NULL;
        boneDst->nextP = NULL;
        boneDst->modified = this->modified;

        if (boneDst->kidsC)
        {
            xBone *iter = this->kidsP;
            xBone *last = NULL;
            for (int i=boneDst->kidsC; i ; --i)
            {
                if (last)
                    last = last->nextP = iter->Clone();
                else
                    last = boneDst->kidsP = iter->Clone();
                iter = iter->nextP;
                if (!last)
                {
                    boneDst->Free();
                    boneDst = NULL;
                    return boneDst;
                }
            }
        }
    }
    return boneDst;
}

xBone *xBone :: Load(FILE *file)
{
    xBone *bone = new xBone();
    if (!bone) return NULL;

    fread(&bone->name,       sizeof(char*), 1, file);
    fread(&bone->id,         sizeof(xBYTE), 1, file);
    fread(&bone->quaternion, sizeof(xVector4), 1, file);
    fread(&bone->ending,     sizeof(xVector3), 1, file);
    fread(&bone->kidsC,      sizeof(xBYTE), 1, file);

    bone->kidsP = NULL;
    bone->nextP = NULL;
    bone->modified = true;

    if (bone->name)
    {
        size_t len = (size_t) bone->name;
        bone->name = new char[len];
        if (!(bone->name) ||
            fread(bone->name, 1, len, file) != len)
        {
            bone->Free();
            return NULL;
        }
    }
    if (bone->kidsC)
    {
        xBone *last = NULL;
        for (int i=0; i < bone->kidsC; ++i)
        {
            if (last)
                last = last->nextP = xBone::Load(file);
            else
                last = bone->kidsP = xBone::Load(file);
            if (!last)
            {
                bone->Free();
                return NULL;
            }
        }
    }
    return bone;
}

void   xBone :: Save(FILE *file)
{
    char *name = this->name;
    if (this->name)
        this->name = (char *) strlen(name)+1;

    fwrite(&this->name,   sizeof(char*), 1, file);
    fwrite(&this->id,     sizeof(xBYTE), 1, file);
    fwrite(&this->quaternion,  sizeof(xVector4), 1, file);
    fwrite(&this->ending, sizeof(xVector3), 1, file);
    fwrite(&this->kidsC,  sizeof(xBYTE), 1, file);

    if (name)
        fwrite(name, 1, (size_t)this->name, file);
    if (this->kidsC)
    {
        xBone *last = this->kidsP;
        for (; last; last = last->nextP)
            last->Save(file);
    }

    this->name = name;
}

/* BONE HELPERS */
xBone *xBone :: ByName(const char *boneName)
{
    if (!strcmp(boneName, this->name))
        return this;
    xBone *bone = this->kidsP, *res;
    for (; bone; bone = bone->nextP)
    {
        res = bone->ByName(boneName);
        if (res) return res;
    }
    return NULL;
}

xBone *xBone :: ById(xBYTE id)
{
    if (this->id == id)
        return this;
    xBone *bone = this->kidsP, *res;
    for (; bone; bone = bone->nextP)
    {
        res = bone->ById(id);
        if (res) return res;
    }
    return NULL;
}

xBone *xBone :: ParentById(xBYTE id)
{
    if (this->id == id) return NULL;
    xBone *bone = this->kidsP, *res;
    for (; bone; bone = bone->nextP)
    {
        if (bone->id == id) return this;
        res = bone->ParentById(id);
        if (res) return res;
    }
    return NULL;
}

xBYTE  xBone :: CountAllKids() const
{
    const xBone *iter = this->kidsP;
    xBYTE count = this->kidsC;
    for(; iter; iter = iter->nextP)
        count += iter->CountAllKids();
    return count;
}

/* MATRICES */
bool   _xBoneCalculateMatrix(const xBone *bone, xMatrix &boneP, const xBone *pbone, int boneId)
{
    if (boneId == bone->id) {
        if (pbone)
            boneP = xMatrixFromQuaternion(bone->quaternion).preTranslate(pbone->ending).postTranslate(-pbone->ending);
        else
            boneP = xMatrixTranslate(bone->quaternion.vector3.xyz);
        return true;
    }
    for (const xBone *cbone = bone->kidsP; cbone; cbone = cbone->nextP) {
        bool res = _xBoneCalculateMatrix(cbone, boneP, bone, boneId);
        if (res) {
            if (pbone)
                boneP.preMultiply(
                    xMatrixFromQuaternion(bone->quaternion).preTranslate(pbone->ending).postTranslate(-pbone->ending));
            else
                boneP.preTranslate(bone->quaternion.vector3);
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

void   _xBoneCalculateMatrices(const xBone *bone, xModelInstance &instance, const xBone *pbone)
{
    xMatrix *boneDst = instance.bonesM + bone->id;
    bool    *modDst  = instance.bonesMod + bone->id;
    xMatrix *parent  = pbone ? instance.bonesM + pbone->id : NULL;

    xMatrix newMat;
    if (parent)
        newMat = *parent * xMatrixFromQuaternion(bone->quaternion).preTranslate(pbone->ending).postTranslate(-pbone->ending);
    else
        newMat = xMatrixTranslate(bone->quaternion.vector3.xyz);
    *modDst  = *boneDst != newMat;
    *boneDst = newMat;

    for (xBone *cbone = bone->kidsP; cbone; cbone = cbone->nextP)
        _xBoneCalculateMatrices(cbone, instance, bone);
}

void    xBoneCalculateMatrices(const xBone *spine, xModelInstance *instance)
{
    if (!spine)
    {
        instance->bonesC = 0;
        if (instance->bonesM)   { delete[] instance->bonesM; instance->bonesM = NULL; }
        if (instance->bonesMod) { delete[] instance->bonesMod; instance->bonesMod = NULL; }
        return;
    }
    instance->bonesC = spine->CountAllKids() + 1;
    if (!instance->bonesM)   instance->bonesM = new xMatrix[instance->bonesC];
    if (!instance->bonesMod) instance->bonesMod = new bool[instance->bonesC];
    _xBoneCalculateMatrices(spine, *instance, NULL);
}

void   _xBoneCalculateQuats(const xBone *bone, xModelInstance &instance, const xBone *pBone)
{
    xVector4 *boneDst = instance.bonesQ + bone->id*2;
    *(boneDst+0) = bone->quaternion;
    if (pBone)
        (boneDst+1)->init(pBone->ending, pBone->id);
    else
        (boneDst+1)->init(bone->quaternion.vector3, -1.f);
    for (xBone *cbone = bone->kidsP; cbone; cbone = cbone->nextP)
        _xBoneCalculateQuats(cbone, instance, bone);
}

void    xBoneCalculateQuats(const xBone *spine, xModelInstance *instance)
{
    if (!spine)
    {
        instance->bonesC = 0;
        instance->bonesQ = NULL;
        return;
    }
    instance->bonesC = spine->CountAllKids() + 1;
    if (!instance->bonesQ) instance->bonesQ = new xVector4[instance->bonesC*2];
    _xBoneCalculateQuats(spine, *instance, NULL);
}
