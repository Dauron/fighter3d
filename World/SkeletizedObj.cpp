#include "SkeletizedObj.h"
#include "../Physics/SkeletizedBody.h"

void SkeletizedObj :: ApplyDefaults()
{
    RigidObj::ApplyDefaults();

    W_resilience = 0.2f;
}

void SkeletizedObj :: Initialize ()
{
    RigidObj::Initialize();
    Type = Model_Verlet;

    verletTime = verletWeight = 0.f;
    verletTimeMaxInv = 0.0f;
    postHit = 0.f;
}

void SkeletizedObj :: Initialize (const char *gr_filename, const char *ph_filename,
                                  bool physicalNotLocked, bool phantom)
{
    forceNotStatic = true;
    RigidObj::Initialize(gr_filename, ph_filename);
}

void SkeletizedObj :: Finalize ()
{
    RigidObj::Finalize();
    if (actions.L_actions.size())
    {
        std::vector<xAction>::iterator iterF = actions.L_actions.begin(), iterE = actions.L_actions.end();
        for (; iterF != iterE; ++iterF)
            g_AnimationMgr.DeleteAnimation(iterF->hAnimation);
        actions.L_actions.clear();
    }
}



void SkeletizedObj :: CreateVerletSystem()
{
    xModel &modelInst = *GetModelGr();
    verletSystem.Free();
    verletSystem.Init(modelInst.Spine.I_bones);
    verletSystem.C_collisions = &collisionConstraints;
    xBone    *bone    = modelInst.Spine.L_bones;
    xPoint3  *P_cur   = verletSystem.P_current,
             *P_old   = verletSystem.P_previous;
    xVector3 *A_iter  = verletSystem.A_forces;
    xQuaternion *QT_skew = verletSystem.QT_boneSkew;
    xFLOAT      *M_iter  = verletSystem.M_weight_Inv;
    bool        *FL_lock = verletSystem.FL_attached;
    xMatrix     *MX_bone = modelInstanceGr.MX_bones;
    xQuaternion *QT_bone = modelInstanceGr.QT_bones;

    if (MX_bone)
        for (int i = modelInst.Spine.I_bones; i; --i, ++bone, ++P_cur, ++P_old, ++QT_skew, ++A_iter, ++M_iter, ++FL_lock,
            ++MX_bone, QT_bone+=2)
        {
            *P_old   = *P_cur = MX_LocalToWorld_Get().preTransformP( MX_bone->postTransformP(bone->P_end) );
            *QT_skew = bone->getSkew(*QT_bone);
            *M_iter  = 1 / bone->M_weight;
            *FL_lock = false;
            A_iter->zero();
        }
    else
        for (int i = modelInst.Spine.I_bones; i; --i, ++bone, ++P_cur, ++P_old, ++QT_skew, ++A_iter, ++M_iter, ++FL_lock)
        {
            *P_old   = *P_cur = MX_LocalToWorld_Get().preTransformP( bone->P_end );
            QT_skew->zeroQ();
            *M_iter  = 1 / bone->M_weight;
            *FL_lock = false;
            A_iter->zero();
        }
}

void SkeletizedObj :: DestroyVerletSystem()
{
    verletSystem.Free();
}

void SkeletizedObj :: UpdateVerletSystem()
{
    xModel      &modelInst = *GetModelGr();
    xBone       *bone    = modelInst.Spine.L_bones;
    xPoint3     *P_old   = verletSystem.P_current;
    xQuaternion *QT_skew = verletSystem.QT_boneSkew;
    xMatrix     *MX_bone = modelInstanceGr.MX_bones;
    xQuaternion *QT_bone = modelInstanceGr.QT_bones;
    for (int i = modelInst.Spine.I_bones; i; --i, ++bone, ++P_old, ++QT_skew, ++MX_bone, QT_bone+=2)
    {
        *P_old   = MX_LocalToWorld_Get().preTransformP( MX_bone->postTransformP(bone->P_end) );
        *QT_skew = bone->getSkew(*QT_bone);
    }
}



void SkeletizedObj :: AddAnimation(const char *fileName, xDWORD T_start, xDWORD T_end)
{
    actions.L_actions.resize(actions.L_actions.size()+1);
    actions.L_actions.rbegin()->hAnimation = g_AnimationMgr.GetAnimation(fileName);
    actions.L_actions.rbegin()->T_start = T_start;
    actions.L_actions.rbegin()->T_end   = T_end;
}

void SkeletizedObj:: PreUpdate(float deltaTime)
{
    SkeletizedBody::CalculateCollisions(this, deltaTime);
}

void SkeletizedObj :: Update(float deltaTime)
{
    SkeletizedBody::CalculateMovement(this, deltaTime);
    CollidedModels.clear();

    xQuaternion *bones = NULL, *bones2 = NULL;

    if (actions.L_actions.size())
    {
        xDWORD delta = (xDWORD)(deltaTime*1000);

        actions.Update(delta);
        bones = actions.GetTransformations();

        //if (actions.progress > 10000) actions.progress = 0;
    }
    if (ControlType == Control_CaptureInput)
        bones2 = g_CaptureInput.GetTransformations();
    //if (ControlType == Control_NetworkInput)
    //    bones2 = g_NetworkInput.GetTransformations();

    if (bones2)
        if (bones)
        {
            xAnimation::Combine(bones2, bones, modelInstanceGr.I_bones , bones);
            delete[] bones2;
        }
        else
            bones = bones2;

    if (false && !bones)
    {
        bones = new xQuaternion[modelInstanceGr.I_bones];
        for (int i = 0; i < modelInstanceGr.I_bones; ++i)
            bones[i].zeroQ();
    }

    if (verletQuaternions && bones && verletWeight > 0.f)
        xAnimation::Average(verletQuaternions, bones, modelInstanceGr.I_bones, 1.f-verletWeight, bones);

    if (bones)
    {
        GetModelGr()->Spine.QuatsFromArray(bones);
        delete[] bones;

        CalculateSkeleton();
        CollisionInfo_ReFill();
    }
    else
    if (verletQuaternions)
    {
        GetModelGr()->Spine.QuatsFromArray(verletQuaternions);
        CalculateSkeleton();
        CollisionInfo_ReFill();
    }
    else
        GetModelGr()->Spine.ResetQ();

    UpdateVerletSystem();
}
