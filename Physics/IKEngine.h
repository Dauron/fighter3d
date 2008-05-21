#ifndef __incl_Physics_IKEngine_h
#define __incl_Physics_IKEngine_h

#include "../Models/lib3dx/xIKNode.h"

class IKEngine
{
private:
    void ProcessNode(xIKNode *nodeArray, xBYTE nodeId, xMatrix *boneM);
    void PropagateChangesForward(xIKNode *nodeArray, xBYTE nodeId, xMatrix *boneM, const xVector3 &changeRoot);

public:
    void Calculate(xIKNode *nodeArray, xBYTE nodeCount, xMatrix *boneM);
};

#endif
