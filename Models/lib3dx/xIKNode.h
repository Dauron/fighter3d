#ifndef __incl_lib3dx_xIKNode_h
#define __incl_lib3dx_xIKNode_h

#include "../../Math/xMath.h"
#include <cstdio>
#include <vector>

struct xIKNode
{
    xBYTE     id;
    char     *name;

    xVector3  pointB;
    xBYTE    *joinsBP;
    xBYTE     joinsBC;

    xVector3  pointE;
    xBYTE    *joinsEP;
    xBYTE     joinsEC;

    // mods
    xVector4  quaternion;
    xFLOAT    curLengthSq;

    // constraints
    xFLOAT    minLengthSq;
    xFLOAT    maxLengthSq;
    // + rotation constraints

    // forces
    bool      forcesValid;
    xVector3  destination;

    // engine helpers
    xVector3  pointBT;
    xVector3  pointET;
    bool      checked;
    bool      modified;

    void Zero()
    {
        id     = 0;
        name   = 0;
        pointB.zero();
        joinsBP = NULL;
        joinsBC = 0;
        pointE.zero();
        joinsEP = NULL;
        joinsEC = 0;
        quaternion.zeroQ();
        curLengthSq = 0;
        minLengthSq = 0;
        maxLengthSq = 0;
        forcesValid = false;
    }

    void Clear()
    {
        if (name)    delete[] name;
        if (joinsBP) delete[] joinsBP;
        if (joinsEP) delete[] joinsEP;
    }

    void JoinBAdd(xBYTE joinId);
    void JoinEAdd(xBYTE joinId);
    void JoinBDelete(xBYTE joinId);
    void JoinEDelete(xBYTE joinId);
    void JoinBReplace(xBYTE oldId, xBYTE newId);
    void JoinEReplace(xBYTE oldId, xBYTE newId);

    void CloneTo(xIKNode &dst) const;
    void Load( FILE *file );
    void Save( FILE *file ) const;
};

#endif
