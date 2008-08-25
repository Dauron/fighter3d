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
    xPoint3   P_begin;
    xPoint3   P_end;

    xBYTE    *ID_kids;
    xBYTE     I_kids;

    // temporary
    xQuaternion QT_rotation;
    xFLOAT      S_length;
    xFLOAT      S_lengthSqr;

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

    xQuaternion getSkew(const xQuaternion &QT_bone) const
    {
        xPoint3 P_def  = P_end-P_begin;
        xPoint3 P_rot  = QT_bone.rotate(P_def);
        xQuaternion QT_rot = xQuaternion::GetRotation(P_rot, P_def);
        return xQuaternion::Product(QT_bone, QT_rot);
    }

    xVector3 getFront() const
    {
        xVector3 N_up = (P_end - P_begin).normalize();
        xVector3 N_side = fabs(N_up.z) >= fabs(N_up.x) && fabs(N_up.z) >= fabs(N_up.y)
            ? xVector3::Create(-1,0,0)
            : xVector3::Create(0,0,1);
        return xVector3::CrossProduct(N_up, N_side).normalize();
    }

    void KidAdd    (xBYTE ID_newKid);
    void KidDelete (xBYTE ID_delKid);
    void KidReplace(xBYTE ID_oldKid, xBYTE ID_newKid);

    void CloneTo(xBone &dst) const;
    void Load( FILE *file );
    void Save( FILE *file ) const;
};

#endif
