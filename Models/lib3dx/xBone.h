#ifndef __incl_lib3dx_xBone_h
#define __incl_lib3dx_xBone_h

#include "../../Math/xMath.h"
#include "xIKNode.h"

struct xSkeleton
{
    xIKNode *boneP;
    xBYTE    boneC;

    void      Clear();
    xSkeleton Clone() const;
    void      ResetQ();

    xIKNode  *BoneAdd(xBYTE parentId, xVector3 ending);

    void Load( FILE *file );
    void Save( FILE *file ) const;
};

struct xModelInstance;

void    xBoneCalculateMatrices (const xSkeleton &spine, xModelInstance *instance);
void    xBoneCalculateQuats    (const xSkeleton &spine, xModelInstance *instance);

#endif
