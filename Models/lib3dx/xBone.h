#ifndef __incl_lib3dx_xBone_h
#define __incl_lib3dx_xBone_h

#include "../../Math/xMath.h"
#include <cstdio>

struct xBone
{
    xBYTE     ID;
    xBYTE     ID_parent;
    char     *Name;

    xFLOAT    M_weight;
    xVector3  P_begin;
    xVector3  P_end;

    xBYTE    *ID_kids;
    xBYTE     I_kids;

    // temporary
    xVector4  QT_rotation;
    xFLOAT    S_length;
    xFLOAT    S_lengthSqr;

    void Zero()
    {
        memset(this, 0, sizeof(xBone));
        QT_rotation.zeroQ();
    }

    void Clear()
    {
        if (Name)    delete[] Name;
        if (ID_kids) delete[] ID_kids;
    }

    void KidAdd    (xBYTE ID_newKid);
    void KidDelete (xBYTE ID_delKid);
    void KidReplace(xBYTE ID_oldKid, xBYTE ID_newKid);

    void CloneTo(xBone &dst) const;
    void Load( FILE *file );
    void Save( FILE *file ) const;
};

#endif
