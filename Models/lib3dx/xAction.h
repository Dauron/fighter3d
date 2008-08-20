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
    std::vector<xAction> L_actions;
    xDWORD               T_progress;

    void          Update(xDWORD T_delta);
    xQuaternion * GetTransformations();

    xActionSet () : T_progress(0) {}

    void AddAnimation(const char *fileName, xDWORD T_start = 0, xDWORD T_end = 0)
    {
        L_actions.resize(L_actions.size()+1);
        L_actions.rbegin()->hAnimation = g_AnimationMgr.GetAnimation(fileName);
        L_actions.rbegin()->T_start = T_start;
        L_actions.rbegin()->T_end   = T_end;
    }
};

#endif
