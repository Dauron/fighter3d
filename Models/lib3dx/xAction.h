#ifndef __incl_lib3dx_xAction_h
#define __incl_lib3dx_xAction_h

#include "xAnimationMgr.h"
#include <vector>

struct xAction {
    HAnimation hAnimation;
    xDWORD     startTime;
    xDWORD     endTime;
};

struct xActionSet {
    std::vector<xAction> actions;
    xDWORD               progress;

    void       Update(xDWORD deltaTime);
    xVector4 * GetTransformations();

    xActionSet () : progress(0) {}
};

#endif
