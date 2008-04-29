#ifndef __incl_xSkeleton_h
#define __incl_xSkeleton_h

#include "xModel.h"

xBone * xBoneByName     (xBone *spine, const char *boneName);
xBone * xBoneById       (xBone *spine, xWORD id);
xBone * xBoneParentById (xBone *spine, xWORD id);
int     xBoneChildCount (const xBone *spine);

xMatrix xBoneCalculateMatrix   (const xBone *spine, int boneId);
void    xBoneCalculateMatrices (const xBone *spine, xMatrix *&boneP, xWORD &boneC);
void    xBoneCalculateQuats    (const xBone *spine, xVector4 *&boneP, xWORD &boneC);

void    xSkeletonAdd  (xFile *file);
void    xSkeletonReset(xBone *spine);
void    xBoneReset   (xBone *bone);
xBone * xBoneAdd     (xFile *model, xBone *parent, xVector3 ending);
void    xBoneDelete  (xFile *model, xBone *boneToDel);

#endif
