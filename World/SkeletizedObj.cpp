#include "SkeletizedObj.h"

void SkeletizedObj :: Initialize (const char *gr_filename, const char *ph_filename,
                                  bool physicalNotLocked, bool phantom)
{
    forceNotStatic = true;
    ModelObj::Initialize(gr_filename, ph_filename);
    resilience = 0.2f;
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

void SkeletizedObj :: Update(float deltaTime)
{
    ModelObj::Update(deltaTime);
    CollidedModels.clear();

    xRender *renderer = GetRenderer();
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
            xAnimation::Combine(bones2, bones, renderer->bonesC , bones);
            delete[] bones2;
        }
        else
            bones = bones2;

    if (bones)
    {
        xAnimation::SaveToSkeleton(renderer->spineP, bones);
        delete[] bones;

        renderer->CalculateSkeleton();
        CollisionInfo_ReFill();
        centerOfTheMass = xCenterOfTheModelMass(renderer->xModelPhysical, renderer->bonesM);
    }
    else
        renderer->spineP->ResetQ();
}