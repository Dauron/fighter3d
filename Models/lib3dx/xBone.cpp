#include "xBone.h"
#include "xModel.h"

/* xSkeleton */
void xSkeleton :: Clear()
{
    if (boneP)
    {
        xIKNode *node = boneP;
        for (; boneC; --boneC, ++node)
            node->Clear();
        delete[] boneP;
        boneP = NULL;
    }
}

xSkeleton xSkeleton :: Clone() const
{
    xSkeleton res;
    res.boneC = this->boneC;

    if (this->boneP)
    {
        res.boneP = new xIKNode[res.boneC];
        const xIKNode *nodeS = this->boneP;
        xIKNode       *nodeD = res.boneP;

        for (int i = this->boneC; i; --i, ++nodeS, ++nodeD)
            nodeS->CloneTo(*nodeD);
    }
    else
        res.boneP = NULL;

    return res;
}

void xSkeleton :: ResetQ()
{
    xIKNode *node = this->boneP;
    for (int i = this->boneC; i; --i, ++node)
        node->quaternion.zeroQ();
}

xIKNode * xSkeleton :: BoneAdd(xBYTE parentId, xVector3 ending)
{
    int cnt = this->boneC;

    xIKNode *bones = new xIKNode[cnt+1];
    memcpy(bones, this->boneP, sizeof(xIKNode)*cnt);

    ++(this->boneC);
    delete[] this->boneP;
    this->boneP = bones;

    xIKNode &parent = this->boneP[parentId];
    parent.JoinEAdd(cnt);
    xIKNode &newBone = this->boneP[cnt];
    newBone.Zero();
    newBone.JoinBAdd(parentId);

    newBone.id = cnt;
    newBone.pointB = parent.pointE;
    newBone.destination = newBone.pointE = ending;
    newBone.curLengthSq = newBone.maxLengthSq = newBone.minLengthSq = (ending-newBone.pointB).lengthSqr();
    return &newBone;
}

void xSkeleton :: Load( FILE *file )
{
    fread(&boneC, sizeof(xBYTE), 1, file);

    boneP = new xIKNode[boneC];
    xIKNode *node = boneP;
    for (int i = boneC; i; --i, ++node)
        node->Load(file);
}

void xSkeleton :: Save( FILE *file ) const
{
    fwrite(&boneC, sizeof(xBYTE), 1, file);

    xIKNode *node = boneP;
    for (int i = boneC; i; --i, ++node)
        node->Save(file);
}

/* MATRICES */
void   _xBoneCalculateMatrices(const xIKNode *boneP, xBYTE boneId, xModelInstance &instance)
{
    const xIKNode *bone = boneP + boneId;
    xMatrix *boneDst = instance.bonesM + boneId;
    bool    *modDst  = instance.bonesMod + boneId;

    xMatrix newMat;
    if (bone->joinsBC)
    {
        const xIKNode *pbone = boneP + bone->joinsBP[0];
        xMatrix *parent = instance.bonesM + bone->joinsBP[0];
        newMat = *parent * xMatrixFromQuaternion(bone->quaternion).preTranslate(bone->pointB).postTranslate(-bone->pointB);
    }
    else
        newMat = xMatrixTranslate(bone->quaternion.vector3.xyz);
    *modDst  = *boneDst != newMat;
    *boneDst = newMat;

    xBYTE *cbone = bone->joinsEP;
    for (int i = bone->joinsEC; i; --i, ++cbone)
        _xBoneCalculateMatrices(boneP, *cbone, instance);
}
void    xBoneCalculateMatrices(const xSkeleton &spine, xModelInstance *instance)
{
    instance->bonesC = spine.boneC;
    if (!spine.boneC)
    {
        if (instance->bonesM)   { delete[] instance->bonesM;   instance->bonesM = NULL; }
        if (instance->bonesMod) { delete[] instance->bonesMod; instance->bonesMod = NULL; }
        return;
    }
    if (!instance->bonesM)   instance->bonesM   = new xMatrix[spine.boneC];
    if (!instance->bonesMod) instance->bonesMod = new bool[spine.boneC];
    _xBoneCalculateMatrices(spine.boneP, 0, *instance);
}

void   _xBoneCalculateQuats(const xIKNode *boneP, xBYTE boneId, xModelInstance &instance)
{
    const xIKNode *bone = boneP + boneId;
    xVector4 *boneDst = instance.bonesQ + boneId*2;
    *(boneDst+0) = bone->quaternion;
    if (bone->joinsBC)
        (boneDst+1)->init(bone->pointB, bone->joinsBP[0]);
    else
        (boneDst+1)->init(bone->quaternion.vector3, -1.f);
    xBYTE *cbone = bone->joinsEP;
    for (int i = bone->joinsEC; i; --i, ++cbone)
        _xBoneCalculateQuats(boneP, *cbone, instance);
}
void    xBoneCalculateQuats(const xSkeleton &spine, xModelInstance *instance)
{
    instance->bonesC = spine.boneC;
    if (!spine.boneC)
    {
        if (instance->bonesQ)   { delete[] instance->bonesQ;   instance->bonesQ = NULL; }
        return;
    }
    if (!instance->bonesQ) instance->bonesQ = new xVector4[spine.boneC*2];
    _xBoneCalculateQuats(spine.boneP, 0, *instance);
}
