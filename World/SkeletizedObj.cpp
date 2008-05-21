#include "SkeletizedObj.h"
#include "../Physics/VerletBody.h"

void SkeletizedObj :: Initialize (const char *gr_filename, const char *ph_filename,
                                  bool physicalNotLocked, bool phantom)
{
    forceNotStatic = true;
    ModelObj::Initialize(gr_filename, ph_filename);
    Type = Model_Verlet;
    ResetVerletSystem();

    resilience = 0.2f;
}

void SkeletizedObj :: ResetVerletSystem()
{
    verletSystem.Free();
    verletSystem.Init(xModelGr->spine.boneC);
    verletSystem.constraintsP = xModelGr->spine.constraintsP; // NOTE: should be redone on constraint changes in skeleton editor
    verletSystem.constraintsC = xModelGr->spine.constraintsC; // NOTE: should be redone on constraint changes in skeleton editor
    verletSystem.collisions = &collisionConstraints;
    xIKNode  *bone   = xModelGr->spine.boneP;
    xVector3 *pos    = verletSystem.positionP,
             *posOld = verletSystem.positionOldP,
             *a      = verletSystem.accelerationP;
    xMatrix  *mtx    = modelInstanceGr.bonesM;

    if (mtx)
        for (int i = xModelGr->spine.boneC; i; --i, ++bone, ++pos, ++posOld, ++a, ++mtx)
        {
            *pos = *posOld = mLocationMatrix.preTransformP( mtx->postTransformP(bone->pointE) );
            a->zero();
        }
    else
        for (int i = xModelGr->spine.boneC; i; --i, ++bone, ++pos, ++posOld, ++a)
        {
            *pos = *posOld = mLocationMatrix.preTransformP(bone->pointE);
            a->zero();
        }
}

void SkeletizedObj :: Finalize ()
{
    ModelObj::Finalize();
    if (actions.actions.size())
    {
        std::vector<xAction>::iterator iterF = actions.actions.begin(), iterE = actions.actions.end();
        for (; iterF != iterE; ++iterF)
            g_AnimationMgr.DeleteAnimation(iterF->hAnimation);
        actions.actions.clear();
    }
}

void SkeletizedObj :: AddAnimation(const char *fileName, xDWORD startTime, xDWORD endTime)
{
    actions.actions.resize(actions.actions.size()+1);
    actions.actions.rbegin()->hAnimation = g_AnimationMgr.GetAnimation(fileName);
    actions.actions.rbegin()->startTime = startTime;
    actions.actions.rbegin()->endTime = endTime;
}

void SkeletizedObj:: PreUpdate()
{
    VerletBody::CalculateCollisions(this);
}

void SkeletizedObj :: Update(float deltaTime)
{
    VerletBody::CalculateMovement(this, deltaTime);
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
    if (ControlType == Control_NetworkInput)
        bones2 = g_NetworkInput.GetTransformations();

    if (bones2)
        if (bones)
        {
            xAnimation::Combine(bones2, bones, modelInstanceGr.bonesC , bones);
            delete[] bones2;
        }
        else
            bones = bones2;

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

    xIKNode  *bone   = xModelGr->spine.boneP;
    xVector3 *pos    = verletSystem.positionOldP,
             *a      = verletSystem.accelerationP;
    xMatrix  *mtx    = modelInstanceGr.bonesM;
    for (int i = xModelGr->spine.boneC; i; --i, ++bone, ++pos, ++a, ++mtx)
    {
        *pos   = mLocationMatrix.preTransformP( mtx->postTransformP(bone->pointE) );
        a->zero();
    }
    verletSystem.SwapPositions();
}
