#include "xAction.h"

void       xActionSet :: Update(xDWORD deltaTime)
{
    this->progress += deltaTime;
}

xQuaternion * xActionSet :: GetTransformations()
{
    std::vector<xAction>::iterator iterF = actions.begin(), iterE = actions.end();

    xQuaternion* bones = NULL, *trans;
    for (; iterF != iterE; ++iterF)
        if (iterF->startTime <= this->progress && (!iterF->endTime || iterF->endTime > this->progress))
        {
            xAnimation *anim   = g_AnimationMgr.GetAnimation( iterF->hAnimation );
            anim->progress     = this->progress - iterF->startTime;
            anim->frameCurrent = anim->frameP;
            anim->UpdatePosition();
            if (!anim->frameCurrent)
                anim->frameCurrent = anim->frameP;
            trans = anim->GetTransformations();
            xAnimation::Combine(trans, bones, anim->boneC, bones);
            delete[] trans;
        }

    return bones;
}
