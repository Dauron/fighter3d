#ifndef __incl_lib3dx_xBone_h
#define __incl_lib3dx_xBone_h

#include <cstdio>
#include "../../Math/xMath.h"

struct xBone {
    char      *name;
    xBYTE      id;
    xVector4   quaternion;
    xVector3   ending;

    xBone     *nextP; // next sibling
    xBone     *kidsP; // first kid
    xBYTE      kidsC; // no of kids

    bool       modified;

    xBone *ByName    ( const char *boneName );
    xBone *ById      ( xBYTE id );
    xBone *ParentById( xBYTE id );
    xBYTE  CountAllKids() const;

    void   Zero();
    void   Free();
    void   ResetQ();
    xBone *Clone() const;

           void   Save( FILE *file );
    static xBone *Load( FILE *file );
};

xMatrix xBoneCalculateMatrix   (const xBone *spine, int boneId);
void    xBoneCalculateMatrices (const xBone *spine, xMatrix  *&boneP, bool *&boneMod, xBYTE &boneC);
void    xBoneCalculateQuats    (const xBone *spine, xVector4 *&boneP, xBYTE &boneC);

#endif
