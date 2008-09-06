#include "xAction.h"

xQuaternion xActionSet :: GetTransformation(xBYTE ID_bone)
{
    std::vector<xAction>::iterator iterF = L_actions.begin(), iterE = L_actions.end();

    xQuaternion bone, trans;
    bone.zeroQ();
    for (; iterF != iterE; ++iterF)
        if (iterF->T_start <= T_progress && (!iterF->T_end || iterF->T_end > T_progress))
        {
            xAnimation *anim   = g_AnimationMgr.GetAnimation( iterF->hAnimation );
            if (!anim) continue;

            anim->T_progress   = T_progress - iterF->T_start;
            anim->CurrentFrame = anim->L_frames;
            anim->UpdatePosition();
            if (!anim->CurrentFrame)
                anim->CurrentFrame = anim->L_frames;
            trans = anim->GetTransformation(ID_bone);

            if (ID_bone == 0) // root
            {
                bone += trans;
                bone.w = 1.f;
            }
            else
            if (bone.w < 1.f-EPSILON)
                bone = xQuaternion::Product(bone, trans);
            else
                bone = trans;
        }

    return bone;
}

xQuaternion * xActionSet :: GetTransformations()
{
    std::vector<xAction>::iterator iterF = L_actions.begin(), iterE = L_actions.end();

    xQuaternion* bones = NULL, *trans;
    for (; iterF != iterE; ++iterF)
        if (iterF->T_start <= T_progress && (!iterF->T_end || iterF->T_end > T_progress))
        {
            xAnimation *anim   = g_AnimationMgr.GetAnimation( iterF->hAnimation );
            if (!anim) continue;

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
