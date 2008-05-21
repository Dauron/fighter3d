#ifndef __incl_xModel_h
#define __incl_xModel_h

#include "xSkeleton.h"
#include "xMaterial.h"
#include "xElement.h"


struct xModel {
    char      *fileName;

    xMaterial *materialP; // first material
    xBYTE      materialC; // no of materials

    xElement  *kidsP;     // model's elements
    xBYTE      kidsC;     // model's elements
    xBYTE      elementC;  // count of all kids

    xSkeleton  spine;

    bool       texturesInited;
    bool       saveCollisionData;
    bool       transparent;  // Are there transparent faces?
    bool       opaque;       // Are there opaque faces?

    void   Free();
    void   BoneDelete( xBYTE boneId );
    void   SkeletonAdd();
    void   SkeletonReset();

    static xModel *Load( const char *fileName, bool createCollisionInfo = true );
           void    Save();
};

struct xModelInstance {
    xMatrix    location;
    xVector3   center;

    xBYTE      bonesC;
    xMatrix  * bonesM;
    xVector4 * bonesQ;
    bool     * bonesMod;

    xElementInstance  *elementInstanceP;
    xBYTE              elementInstanceC;

    void Zero();
    void ZeroElements();
    void Clear();
    void ClearSkeleton();
};

#endif
