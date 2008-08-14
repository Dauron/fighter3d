#ifndef __incl_lib3dx_xIKNode_h
#define __incl_lib3dx_xIKNode_h

#include "../../Math/xMath.h"
#include <cstdio>
#include <vector>

struct xIKNode
{
    xBYTE     ID;
    char     *Name;
    xFLOAT    weight;

    xVector3  pointB;
    xBYTE    *joinsBP;
    xBYTE     joinsBC;

    xVector3  pointE;
    xBYTE    *joinsEP;
    xBYTE     joinsEC;

    // mods
    xQuaternion quaternion;
    xFLOAT      curLengthSq;

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
        ID     = 0;
        Name   = 0;
        weight = 1.f;
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
        if (Name)    delete[] Name;
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
