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
};

#endif
