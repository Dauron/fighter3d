#ifndef __incl_lib3dx_xVertex_h
#define __incl_lib3dx_xVertex_h

#include "../../Math/xMath.h"

// Vertices
union xVertex {
    struct {
        xFLOAT x, y, z; // vertex coordinates
    };
    struct {
        xPoint3 pos;
    };
};

union xVertexTex {
    struct {
        xFLOAT x, y, z, tx, ty; // vertex and texture coordinates
    };
    struct {
        xPoint3 pos;
        xFLOAT2 tex;
    };
};

union xVertexSkel {
    struct {
        xFLOAT x, y, z;        // vertex coordinates
        xFLOAT b0, b1, b2, b3; // bones
    };
    struct {
        xPoint3 pos;
        xFLOAT4 bone; // up to 4 bones per vertex. boneIndex = floor(bone), boneInfluence = fract(bone)*10
    };
};

union xVertexTexSkel {
    struct {
        xFLOAT x, y, z;        // vertex coordinates
        xFLOAT b0, b1, b2, b3; // bones
        xFLOAT tx, ty;         // texture coordinates
    };
    struct {
        xPoint3 pos;
        xFLOAT4 bone; // up to 4 bones per vertex. boneIndex = floor(bone), boneInfluence = fract(bone)*10
        xFLOAT2 tex;
    };
};

#endif
