#include "xSkeleton.h"
#include "xModel.h"

/* xSkeleton */

void xSkeleton :: QuatsToArray  (xVector4 *qarray) const
{
    xIKNode *node = boneP;
    xVector4 *quat = qarray;
    for (int i = boneC; i; --i, ++node, ++quat)
        *quat = node->quaternion;
}

void xSkeleton :: QuatsFromArray(const xVector4 *qarray)
{
    xIKNode *node = boneP;
    const xVector4 *quat = qarray;
    for (int i = boneC; i; --i, ++node, ++quat)
        node->quaternion = *quat;
}

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
    if (boneConstrP)
    {
        delete[] boneConstrP;
        boneConstrP = NULL;
    }
    if (constraintsP)
    {
        xIVConstraint **constr = constraintsP;
        for (; constraintsC; --constraintsC, ++constr)
            delete *constr;
        delete[] constraintsP;
        constraintsP = NULL;
    }
}

void xSkeleton :: CalcQuats(const xVector3 *pos, xBYTE boneId, xMatrix parentMtxInv)
{
    xIKNode &bone = boneP[boneId];
    xVector3 endN = parentMtxInv.postTransformP(pos[boneId]);

    if (boneId)
    {
        bone.quaternion = xQuaternion::getRotation(bone.pointE, endN, bone.pointB);
        xVector4 quat;
        quat.init(-bone.quaternion.vector3, bone.quaternion.w);
        parentMtxInv.preMultiply(xMatrixFromQuaternion(quat).preTranslate(bone.pointB).postTranslate(-bone.pointB));
    }
    else
    {
        bone.quaternion.init(endN - bone.pointE, 1.f);
        parentMtxInv.preMultiply(xMatrixTranslate(-bone.quaternion.x, -bone.quaternion.y, -bone.quaternion.z));
    }

    for (int i = 0; i < bone.joinsEC; ++i)
        CalcQuats(pos, bone.joinsEP[i], parentMtxInv);
}

void xSkeleton :: FillBoneConstraints()
{
    if (boneConstrP)
    {
        delete[] boneConstrP;
        boneConstrP = NULL;
    }
    if (boneC)
    {
        boneConstrP = new xVConstraintLengthEql[boneC-1];
        xVConstraintLengthEql *constrIter = boneConstrP;

        xIKNode *bone = boneP+1;
        for (int i = boneC-1; i; --i, ++bone, ++constrIter)
        {
            xVConstraintLengthEql &constraint = *constrIter;
            constraint.particleA = bone->id;
            constraint.particleB = bone->joinsBP[0];
            constraint.restLengthSqr = bone->curLengthSq;
            constraint.restLength = sqrt(bone->curLengthSq);
        }
    }
}

xSkeleton xSkeleton :: Clone() const
{
    xSkeleton res;
    res.boneC        = this->boneC;
    res.constraintsC = this->constraintsC;

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

    res.FillBoneConstraints();

    if (this->constraintsP)
    {
        res.constraintsP = new xIVConstraint*[res.constraintsC];
        xIVConstraint **nodeS = this->constraintsP;
        xIVConstraint **nodeD = res.constraintsP;

        for (int i = this->constraintsC; i; --i, ++nodeS, ++nodeD)
            (*nodeS)->CloneTo(*nodeD);
    }
    else
        res.constraintsP = NULL;

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

    FillBoneConstraints();

    return &newBone;
}

void xSkeleton :: Load( FILE *file )
{
    fread(&boneC, sizeof(xBYTE), 1, file);
    boneP = new xIKNode[boneC];
    xIKNode *node = boneP;
    for (int i = boneC; i; --i, ++node)
        node->Load(file);

    FillBoneConstraints();

    fread(&constraintsC, sizeof(xBYTE), 1, file);
    constraintsP = new xIVConstraint*[constraintsC];
    xIVConstraint **constr = constraintsP;
    for (int i = constraintsC; i; --i, ++constr)
        *constr = xIVConstraint::LoadType(file);
}

void xSkeleton :: Save( FILE *file ) const
{
    fwrite(&boneC, sizeof(xBYTE), 1, file);
    xIKNode *node = boneP;
    for (int i = boneC; i; --i, ++node)
        node->Save(file);

    fwrite(&constraintsC, sizeof(xBYTE), 1, file);
    xIVConstraint **constr = constraintsP;
    for (int i = constraintsC; i; --i, ++constr)
        (*constr)->Save(file);
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

bool   _xBoneCalculateQuatForVerlet(const xIKNode *boneP, xBYTE destId, xBYTE boneId, xVector4 &quatPar, xVector4 &quatCur)
{
    const xIKNode *bone = boneP + boneId;

    if (bone->id == destId)
    {
        quatCur = bone->quaternion;
        if (!bone->joinsBP[0])
        {
            const xIKNode *par;
            if (boneP->joinsEP[0] == boneId)
                par = boneP + boneP->joinsEP[1];
            else
                par = boneP + boneP->joinsEP[0];
            quatPar.init(par->quaternion.vector3, par->quaternion.w);
        }
        else
            quatPar.zeroQ();

        return true;
    }

    xBYTE *cbone = bone->joinsEP;
    for (int i = bone->joinsEC; i; --i, ++cbone)
        if (_xBoneCalculateQuatForVerlet(boneP, destId, *cbone, quatPar, quatCur))
        {
            if (boneId)
                quatPar = xQuaternion::product(bone->quaternion, quatPar);
            return true;
        }
    return false;
}
void    xBoneCalculateQuatForVerlet(const xSkeleton &spine, xBYTE destId, xVector4 &quatPar, xVector4 &quatCur)
{
    _xBoneCalculateQuatForVerlet(spine.boneP, destId, 0, quatPar, quatCur);
}