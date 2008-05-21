#ifndef __incl_lib3dx_xBone_h
#define __incl_lib3dx_xBone_h

#include "../../Math/xMath.h"
#include "../../Math/xVerletConstraint.h"
#include "xIKNode.h"

struct xSkeleton
{
    xIKNode *boneP;
    xBYTE    boneC;

    xVConstraintLengthEql *boneConstrP;
    xIVConstraint        **constraintsP;
    xBYTE                  constraintsC;

    void      Clear();
    xSkeleton Clone() const;
    void      ResetQ();

    void      CalcQuats(const xVector3 *pos, xBYTE boneId, xMatrix parentMtxInv);
    void      FillBoneConstraints();
    xIKNode  *BoneAdd(xBYTE parentId, xVector3 ending);

    void QuatsToArray  (xVector4 *qarray) const;
    void QuatsFromArray(const xVector4 *qarray);

    void Load( FILE *file );
    void Save( FILE *file ) const;
};

struct xModelInstance;

void    xBoneCalculateMatrices (const xSkeleton &spine, xModelInstance *instance);
void    xBoneCalculateQuats    (const xSkeleton &spine, xModelInstance *instance);

#endif
