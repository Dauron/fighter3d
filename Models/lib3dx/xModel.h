#ifndef __incl_xModel_h
#define __incl_xModel_h

#include "xBone.h"
#include "xMaterial.h"
#include "xElement.h"

struct xModel {
    char      *fileName;

    xMaterial *materialP; // first material
    xBYTE      materialC; // no of materials

    xElement  *kidsP;     // model's elements
    xBYTE      kidsC;     // model's elements
    xBYTE      elementC;  // count of all kids

    xBone     *spineP;    // spine of the model

    bool       texturesInited;
    bool       saveCollisionData;
    bool       transparent;  // Are there transparent faces?
    bool       opaque;       // Are there opaque faces?

    void   Free();
    xBone *BoneAdd   ( xBone *parent, xVector3 ending );
    void   BoneDelete( xBone *boneToDel );
    void   SkeletonAdd();
    void   SkeletonReset();

    static xModel *Load( const char *fileName, bool createCollisionInfo = true );
           void    Save();
};

#endif
