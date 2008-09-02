#include "xSkeleton.h"
#include "xModel.h"

/* xSkeleton */

void xSkeleton :: QuatsToArray  (xQuaternion *QT_array) const
{
    const xBone *bone = L_bones;
    xQuaternion *quat = QT_array;
    for (int i = I_bones; i; --i, ++bone, ++quat)
        *quat = bone->QT_rotation;
}

void xSkeleton :: QuatsFromArray(const xQuaternion *QT_array)
{
    xBone *bone = L_bones;
    const xQuaternion *quat = QT_array;
    for (int i = I_bones; i; --i, ++bone, ++quat)
        bone->QT_rotation = *quat;
}

void xSkeleton :: Clear()
{
    if (I_bones)
    {
        xBone *bone = L_bones;
        for (; I_bones; --I_bones, ++bone)
            bone->Clear();
        delete[] L_bones;
        L_bones = NULL;
    }
    if (C_boneLength)
    {
        delete[] C_boneLength;
        C_boneLength = NULL;
    }
    if (C_constraints)
    {
        VConstraint **constr = C_constraints;
        for (; I_constraints; --I_constraints, ++constr)
            delete *constr;
        delete[] C_constraints;
        C_constraints = NULL;
    }
}

void xSkeleton :: CalcQuats(const xPoint3 *P_current, const xQuaternion *QT_boneSkew,
                            xBYTE ID_bone, xMatrix MX_parent_Inv)
{
    xBone   &bone   = L_bones[ID_bone];

    if (ID_bone)
    {
        xPoint3 NW_upN = MX_parent_Inv.postTransformV(P_current[ID_bone]-P_current[bone.ID_parent]);
        if (QT_boneSkew && QT_boneSkew[ID_bone].w < EPSILON)
            NW_upN = QT_boneSkew[ID_bone].unrotate(NW_upN);
        xQuaternion quat = xQuaternion::GetRotation(bone.P_end-bone.P_begin, NW_upN);

        if (QT_boneSkew && QT_boneSkew[ID_bone].w < EPSILON)
            bone.QT_rotation = xQuaternion::Product(QT_boneSkew[ID_bone], quat);
        else
            bone.QT_rotation = quat;
        quat.init(-bone.QT_rotation.vector3, bone.QT_rotation.w);
        MX_parent_Inv.preMultiply(xMatrixFromQuaternion(quat));
    }
    else
    {
        xPoint3 P_end_N = MX_parent_Inv.postTransformP(P_current[0]);
        bone.QT_rotation.init(P_end_N - bone.P_end, 1.f);
    }

    for (int i = 0; i < bone.I_kids; ++i)
        CalcQuats(P_current, QT_boneSkew, bone.ID_kids[i], MX_parent_Inv);
}

void xSkeleton :: CalcQuats(const xPoint3 *P_current, const xQuaternion *QT_boneSkew,
                            const bool *FL_calculate, xBYTE ID_bone,
                            xMatrix MX_parent_Inv)
{
    xBone   &bone   = L_bones[ID_bone];

    if (ID_bone)
    {
        xPoint3 NW_upN = MX_parent_Inv.postTransformV(P_current[ID_bone]-P_current[bone.ID_parent]);
        if (QT_boneSkew && QT_boneSkew[ID_bone].w < EPSILON)
            NW_upN = QT_boneSkew[ID_bone].unrotate(NW_upN);
        xQuaternion quat = xQuaternion::GetRotation(bone.P_end-bone.P_begin, NW_upN);

        if (QT_boneSkew && QT_boneSkew[ID_bone].w < EPSILON)
            bone.QT_rotation = xQuaternion::Product(QT_boneSkew[ID_bone], quat);
        else
            bone.QT_rotation = quat;
        quat.init(-bone.QT_rotation.vector3, bone.QT_rotation.w);
        MX_parent_Inv.preMultiply(xMatrixFromQuaternion(quat));
    }
    else
    {
        xPoint3 P_end_N = MX_parent_Inv.postTransformP(P_current[0]);
        bone.QT_rotation.init(P_end_N - bone.P_end, 1.f);
    }

    for (int i = 0; i < bone.I_kids; ++i)
        if (FL_calculate[bone.ID_kids[i]])
            CalcQuats(P_current, QT_boneSkew, FL_calculate, bone.ID_kids[i], MX_parent_Inv);
}

void xSkeleton :: FillBoneConstraints()
{
    if (C_boneLength)
    {
        delete[] C_boneLength;
        C_boneLength = NULL;
    }
    if (I_bones)
    {
        C_boneLength = new VConstraintLengthEql[I_bones-1];
        VConstraintLengthEql *C_iter = C_boneLength;

        xBone *bone = L_bones+1;
        for (int i = I_bones-1; i; --i, ++bone, ++C_iter)
        {
            VConstraintLengthEql &constraint = *C_iter;
            constraint.particleA     = bone->ID;
            constraint.particleB     = bone->ID_parent;
            constraint.restLength    = bone->S_length;
            constraint.restLengthSqr = bone->S_lengthSqr;
        }
    }
}

xSkeleton xSkeleton :: Clone() const
{
    xSkeleton res;
    res.I_bones       = this->I_bones;
    res.I_constraints = this->I_constraints;

    if (this->L_bones)
    {
        res.L_bones = new xBone[res.I_bones];
        const xBone *boneS = this->L_bones;
        xBone       *boneD = res.L_bones;

        for (int i = this->I_bones; i; --i, ++boneS, ++boneD)
            boneS->CloneTo(*boneD);
    }
    else
        res.L_bones = NULL;

    res.C_boneLength  = NULL;
    res.FillBoneConstraints();

    if (this->I_constraints)
    {
        res.C_constraints = new VConstraint*[res.I_constraints];
        VConstraint **nodeS = this->C_constraints;
        VConstraint **nodeD = res.C_constraints;

        for (int i = this->I_constraints; i; --i, ++nodeS, ++nodeD)
            (*nodeS)->CloneTo(*nodeD);
    }
    else
        res.C_constraints = NULL;

    return res;
}

void xSkeleton :: ResetQ()
{
    xBone *bone = this->L_bones;
    for (int i = this->I_bones; i; --i, ++bone)
        bone->QT_rotation.zeroQ();
}

xBone * xSkeleton :: BoneAdd(xBYTE ID_parent, xPoint3 P_end)
{
    int I_bones = this->I_bones;

    xBone *bones = new xBone[I_bones+1];
    memcpy(bones, this->L_bones, sizeof(xBone)*I_bones);

    ++(this->I_bones);
    delete[] this->L_bones;
    this->L_bones = bones;

    xBone &parent = this->L_bones[ID_parent];
    parent.KidAdd(I_bones);
    xBone &newBone = this->L_bones[I_bones];
    newBone.Zero();
    newBone.ID = I_bones;
    newBone.ID_parent   = ID_parent;
    newBone.P_begin     = parent.P_end;
    newBone.P_end       = P_end;
    newBone.S_lengthSqr = (P_end - newBone.P_begin).lengthSqr();
    newBone.S_length    = sqrt(newBone.S_lengthSqr);

    FillBoneConstraints();

    return &newBone;
}


void xSkeleton :: Load( FILE *file )
{
    fread(&I_bones, sizeof(xBYTE), 1, file);
    if (I_bones)
    {
        xBone *bone = L_bones = new xBone[I_bones];
        for (int i = I_bones; i; --i, ++bone)
            bone->Load(file);
    }
    else
        L_bones = NULL;

    FillBoneConstraints();

    fread(&I_constraints, sizeof(xBYTE), 1, file);
    if (I_constraints)
    {
        VConstraint **C_iter = C_constraints = new VConstraint*[I_constraints];
        for (int i = I_constraints; i; --i, ++C_iter)
            *C_iter = VConstraint::LoadType(file);
    }
    else
        C_constraints = NULL;
}

void xSkeleton :: Save( FILE *file ) const
{
    fwrite(&I_bones, sizeof(xBYTE), 1, file);
    xBone *bone = L_bones;
    for (int i = I_bones; i; --i, ++bone)
        bone->Save(file);

    fwrite(&I_constraints, sizeof(xBYTE), 1, file);
    VConstraint **C_iter = C_constraints;
    for (int i = I_constraints; i; --i, ++C_iter)
        (*C_iter)->Save(file);
}

/* MATRICES */
void   _xBoneCalculateMatrices(const xBone *L_bones, xBYTE ID_bone, xModelInstance &instance)
{
    const xBone &bone        = L_bones[ID_bone];
    xMatrix     &MX_bone     = instance.MX_bones[ID_bone];
    bool        &FL_modified = instance.FL_modified[ID_bone];

    xMatrix MX_new;
    if (bone.ID)
    {
        xMatrix &MX_parent = instance.MX_bones[bone.ID_parent];
        MX_new = MX_parent * xMatrixFromQuaternion(bone.QT_rotation).preTranslate(bone.P_begin).postTranslate(-bone.P_begin);
    }
    else
        MX_new = xMatrixTranslate(bone.QT_rotation.vector3.xyz);
    FL_modified = MX_bone != MX_new;
    MX_bone = MX_new;

    xBYTE *ID_iter = bone.ID_kids;
    for (int i = bone.I_kids; i; --i, ++ID_iter)
        _xBoneCalculateMatrices(L_bones, *ID_iter, instance);
}
void    xBoneCalculateMatrices(const xSkeleton &spine, xModelInstance &instance)
{
    instance.I_bones = spine.I_bones;
    if (!spine.I_bones) return;
    if (!instance.MX_bones)    instance.MX_bones    = new xMatrix[spine.I_bones];
    if (!instance.FL_modified) instance.FL_modified = new bool[spine.I_bones];
    _xBoneCalculateMatrices(spine.L_bones, 0, instance);
}

void   _xBoneCalculateQuats(const xBone *L_bones, xBYTE ID_bone, xModelInstance &instance)
{
    const xBone &bone    = L_bones[ID_bone];
    xQuaternion &QT_bone = instance.QT_bones[ID_bone];
    xPoint3     &P_root  = instance.P_bone_roots[ID_bone];
    xPoint3     &P_tran  = instance.P_bone_trans[ID_bone];

    P_root = bone.P_begin;
    if (bone.ID)
    {
        xQuaternion &QT_bone_p = instance.QT_bones[bone.ID_parent];

        if (bone.ID_parent)
        {
            xPoint3     &P_root_p  = instance.P_bone_roots[bone.ID_parent];
            xPoint3     &P_tran_p  = instance.P_bone_trans[bone.ID_parent];

            QT_bone = xQuaternion::Product(QT_bone_p, bone.QT_rotation);
            P_tran  = QT_bone_p.rotate(bone.P_begin - P_root_p) + P_tran_p;
        }
        else
        {
            QT_bone = bone.QT_rotation;
            P_tran  = bone.P_begin + QT_bone_p.vector3;
        }
    }
    else
    {
        QT_bone = bone.QT_rotation;
        P_tran  = P_root + QT_bone.vector3;
    }

    xBYTE *ID_iter = bone.ID_kids;
    for (int i = bone.I_kids; i; --i, ++ID_iter)
        _xBoneCalculateQuats(L_bones, *ID_iter, instance);
}
void    xBoneCalculateQuats(const xSkeleton &spine, xModelInstance &instance)
{
    instance.I_bones = spine.I_bones;
    if (!spine.I_bones) return;
    if (!instance.QT_bones)     instance.QT_bones = new xQuaternion[spine.I_bones];
    if (!instance.P_bone_roots) instance.P_bone_roots = new xPoint3[spine.I_bones];
    if (!instance.P_bone_trans) instance.P_bone_trans = new xPoint3[spine.I_bones];
    _xBoneCalculateQuats(spine.L_bones, 0, instance);
}

bool   _xBoneCalculateQuatForVerlet(const xBone *L_bones, xBYTE ID_last, xBYTE ID_bone,
                                    xQuaternion &QT_parent, xQuaternion &QT_current)
{
    const xBone &bone = L_bones[ID_bone];

    if (bone.ID == ID_last)
    {
        QT_current = bone.QT_rotation;
        if (bone.ID_parent == 0)
        {
            const xBone *parent;
            if (L_bones->ID_kids[0] == ID_bone)
                parent = L_bones + L_bones->ID_kids[1];
            else
                parent = L_bones + L_bones->ID_kids[0];
            QT_parent = parent->QT_rotation;
        }
        else
            QT_parent.zeroQ();

        return true;
    }

    xBYTE *ID_iter = bone.ID_kids;
    for (int i = bone.I_kids; i; --i, ++ID_iter)
        if (_xBoneCalculateQuatForVerlet(L_bones, ID_last, *ID_iter, QT_parent, QT_current))
        {
            if (ID_bone)
                QT_parent = xQuaternion::Product(bone.QT_rotation, QT_parent);
            return true;
        }
    return false;
}
void    xBoneCalculateQuatForVerlet(const xSkeleton &spine, xBYTE ID_last,
                                    xQuaternion &QT_parent, xQuaternion &QT_current)
{
    _xBoneCalculateQuatForVerlet(spine.L_bones, ID_last, 0, QT_parent, QT_current);
}

