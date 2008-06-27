#include "SkeletizedObj.h"
#include "../Physics/SkeletizedBody.h"

void SkeletizedObj :: Initialize (const char *gr_filename, const char *ph_filename,
                                  bool physicalNotLocked, bool phantom)
{
    forceNotStatic = true;
    RigidObj::Initialize(gr_filename, ph_filename);
    Type = Model_Verlet;

    resilience = 0.2f;
    verletTime = verletWeight = 0.f;
    verletTimeMaxInv = 1.f;
}

void SkeletizedObj :: Finalize ()
{
    RigidObj::Finalize();
    if (actions.actions.size())
    {
        std::vector<xAction>::iterator iterF = actions.actions.begin(), iterE = actions.actions.end();
        for (; iterF != iterE; ++iterF)
            g_AnimationMgr.DeleteAnimation(iterF->hAnimation);
        actions.actions.clear();
    }
}



void SkeletizedObj :: CreateVerletSystem()
{
    verletSystem.Free();
    verletSystem.Init(xModelGr->spine.I_bones);
    verletSystem.C_collisions = &collisionConstraints;
    xBone    *bone    = xModelGr->spine.L_bones;
    xVector3 *P_cur   = verletSystem.P_current,
             *P_old   = verletSystem.P_previous,
             *A_iter  = verletSystem.A_forces;
    xFLOAT   *M_iter  = verletSystem.M_weight_Inv;
    xMatrix  *MX_bone = modelInstanceGr.bonesM;

    if (MX_bone)
        for (int i = xModelGr->spine.I_bones; i; --i, ++bone, ++P_cur, ++P_old, ++A_iter, ++M_iter, ++MX_bone)
        {
            *P_old = *P_cur = MX_ModelToWorld.preTransformP( MX_bone->postTransformP(bone->P_end) );
            *M_iter = 1 / bone->M_weight;
            A_iter->zero();
        }
    else
        for (int i = xModelGr->spine.I_bones; i; --i, ++bone, ++P_cur, ++P_old, ++A_iter, ++M_iter)
        {
            *P_old = *P_cur = MX_ModelToWorld.preTransformP( bone->P_end );
            *M_iter = 1 / bone->M_weight;
            A_iter->zero();
        }
}

void SkeletizedObj :: DestroyVerletSystem()
{
    verletSystem.Free();
}

void SkeletizedObj :: UpdateVerletSystem()
{
    xBone    *bone    = xModelGr->spine.L_bones;
    xVector3 *P_old   = verletSystem.P_previous;
    xMatrix  *MX_bone = modelInstanceGr.bonesM;
    for (int i = xModelGr->spine.I_bones; i; --i, ++bone, ++P_old, ++MX_bone)
        *P_old   = MX_ModelToWorld.preTransformP( MX_bone->postTransformP(bone->P_end) );
    verletSystem.SwapPositions();
}



void SkeletizedObj :: AddAnimation(const char *fileName, xDWORD startTime, xDWORD endTime)
{
    actions.actions.resize(actions.actions.size()+1);
    actions.actions.rbegin()->hAnimation = g_AnimationMgr.GetAnimation(fileName);
    actions.actions.rbegin()->startTime = startTime;
    actions.actions.rbegin()->endTime = endTime;
}

void SkeletizedObj:: PreUpdate(float deltaTime)
{
    SkeletizedBody::CalculateCollisions(this, deltaTime);
}

void SkeletizedObj :: Update(float deltaTime)
{
    SkeletizedBody::CalculateMovement(this, deltaTime);
    CollidedModels.clear();

    xVector4 *bones = NULL, *bones2 = NULL;

    if (actions.actions.size())
    {
        xDWORD delta = (xDWORD)(deltaTime*1000);

        actions.Update(delta);
        bones = actions.GetTransformations();

        if (actions.progress > 10000) actions.progress = 0;
    }
    if (ControlType == Control_CaptureInput)
        bones2 = g_CaptureInput.GetTransformations();
    //if (ControlType == Control_NetworkInput)
    //    bones2 = g_NetworkInput.GetTransformations();

    if (bones2)
        if (bones)
        {
            xAnimation::Combine(bones2, bones, modelInstanceGr.bonesC , bones);
            delete[] bones2;
        }
        else
            bones = bones2;

    if (false && !bones)
    {
        bones = new xVector4[modelInstanceGr.bonesC];
        for (int i = 0; i < modelInstanceGr.bonesC; ++i)
            bones[i].zeroQ();
    }

    if (verletQuaternions && bones && verletWeight > 0.f)
        xAnimation::Average(verletQuaternions, bones, modelInstanceGr.bonesC, 1.f-verletWeight, bones);

    if (bones)
    {
        GetModelGr()->spine.QuatsFromArray(bones);
        delete[] bones;

        CalculateSkeleton();
        CollisionInfo_ReFill();
    }
    else
    if (verletQuaternions)
    {
        GetModelGr()->spine.QuatsFromArray(verletQuaternions);
        CalculateSkeleton();
        CollisionInfo_ReFill();
    }
    else
        GetModelGr()->spine.ResetQ();

    UpdateVerletSystem();
}
