#include "xAction.h"

void       xActionSet :: Update(xDWORD T_delta)
{
    this->T_progress += T_delta;
}

xQuaternion * xActionSet :: GetTransformations()
{
    std::vector<xAction>::iterator iterF = L_actions.begin(), iterE = L_actions.end();

    xQuaternion* bones = NULL, *trans;
    for (; iterF != iterE; ++iterF)
        if (iterF->T_start <= T_progress && (!iterF->T_end || iterF->T_end > T_progress))
        {
            xAnimation *anim   = g_AnimationMgr.GetAnimation( iterF->hAnimation );
            anim->T_progress   = T_progress - iterF->T_start;
            anim->CurrentFrame = anim->L_frames;
            anim->UpdatePosition();
            if (!anim->CurrentFrame)
                anim->CurrentFrame = anim->L_frames;
            trans = anim->GetTransformations();
            xAnimation::Combine(trans, bones, anim->I_bones, bones);
            delete[] trans;
        }

    return bones;
}
