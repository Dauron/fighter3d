#ifndef __incl_lib3dx_xElementData_h
#define __incl_lib3dx_xElementData_h

#include "xMaterial.h"

struct xFaceList {
    xWORD      indexOffset; // first face in the facesP array
    xWORD      indexCount;  // no of faces in the facesP array

    bool       smooth;
    xBYTE      materialId;
    xMaterial *materialP;
};

struct xEdge {
    xWORD face1;
    xWORD face2;
    xWORD vert1;
    xWORD vert2;
};

typedef xWORD3 xFace;

union xGPUPointers
{
    struct {
        xDWORD vertexB;
        xDWORD normalB;
        xDWORD indexB;
    };
    struct {
        xDWORD listID;       // Compiled Render List
        xDWORD listIDTransp; // Compiled Render List for transparent faces
    };
};

struct xRenderShadowData
{
    xGPUPointers gpuPointers;

    xVector4 *verticesP;
    xVector3 *normalsP;
    xWORD    *indexP;
    xWORD     sideC;
    xWORD     frontC;
    xWORD     backC;
};

struct xElementInstance
{
    typedef enum {
        xRenderMode_NULL = 0,
        xRenderMode_VBO  = 1,
        xRenderMode_LIST = 2
    } xRenderMode;

    xRenderMode mode;

    xGPUPointers       gpuMain;
    xRenderShadowData *gpuShadows;

    xBox     bbBox;
    xVector3 bsCenter;
    xFLOAT   bsRadius;
};

struct xRenderData
{
    bool       transparent;  // Are there transparent faces?
    bool       opaque;       // Are there opaque faces?

    union {                  // smooth vertices
        xVertex        *verticesP;
        xVertexTex     *verticesTP;
        xVertexSkel    *verticesSP;
        xVertexTexSkel *verticesTSP;
    };
    xWORD              verticesC;
    xVector3          *normalP; // smooth normals
    xFace            *facesP;

    xVector3          *faceNormalsP;
};

struct xModel;
struct xElement;
struct xCollisionHierarchy;

struct xCollisionData
{
    xWORD                kidsC;
    xCollisionHierarchy *kidsP;

    void Load( FILE *file, xElement *elem );
    void Save( FILE *file, xElement *elem );

    void FreeKids();
    void Fill ( xModel *xmodel, xElement *elem );

    static const xBYTE MAX_HIERARCHY_DEPTH = 10;
};

struct xCollisionHierarchy : xCollisionData
{
    xWORD    facesC;
    xFace ** facesP;
    xWORD    verticesC;
    xWORD *  verticesP;

    void Load( FILE *file, xElement *elem );
    void Save( FILE *file, xElement *elem );

    void Subdivide(const xElement *elem, float scale, int depth, const xBox &bounding);
};

#endif
