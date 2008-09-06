#ifndef __incl_lib3dx_xAction_h
#define __incl_lib3dx_xAction_h

#include "xAnimationMgr.h"
#include <vector>

struct xAction {
    HAnimation hAnimation;
    xDWORD     T_start;
    xDWORD     T_end;
};

struct xActionSet {
    typedef std::vector<xAction> Vec_xAction;

    Vec_xAction   L_actions;
    xDWORD        T_progress;

    xActionSet () { Clear(); }

    void Clear()
    {
        T_progress = 0;
        L_actions.clear();
    }

    void Destroy()
    {
        if (L_actions.size())
        {
            Vec_xAction::iterator A_curr = L_actions.begin(),
                                  A_last = L_actions.end();
            for (; A_curr != A_last; ++A_curr)
                g_AnimationMgr.Release(A_curr->hAnimation);
        }
        Clear();
    }

    void AddAnimation(const char *fileName, xDWORD T_start = 0, xDWORD T_end = 0)
    {
        L_actions.resize(L_actions.size()+1);
        L_actions.back().hAnimation = g_AnimationMgr.GetAnimation(fileName);
        L_actions.back().T_start    = T_start;
        L_actions.back().T_end      = T_end;
    }

    void  Update(xDWORD T_delta)
    { this->T_progress += T_delta; }
    xQuaternion   GetTransformation(xBYTE ID_bone);
    xQuaternion * GetTransformations();
};

#endif
