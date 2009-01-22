#ifndef __incl_lib3dx_xVertex_h
#define __incl_lib3dx_xVertex_h

#include "../../Math/xMath.h"

// Vertices
struct xVertex {
    xPoint3 pos;
};

struct xVertexTex {
    xPoint3 pos;
    xTextUV tex;
};

struct xVertexSkel {
    xPoint3 pos;
    union {
        struct {
            xFLOAT b0, b1, b2, b3; // bones
        };
        xFLOAT4 bone;          // up to 4 bones per vertex. boneIndex = floor(bone), boneInfluence = fract(bone)*10
    };
};

struct xVertexTexSkel {
    xPoint3 pos;
    union {
        struct {
            xFLOAT b0, b1, b2, b3; // bones
        };
        xFLOAT4 bone;          // up to 4 bones per vertex. boneIndex = floor(bone), boneInfluence = fract(bone)*10
    };
    xTextUV tex;
};

#endif
