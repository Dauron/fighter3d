#ifndef __incl_lib3dx_xElementData_h
#define __incl_lib3dx_xElementData_h

#include "xMaterial.h"
#include "../../Math/Figures/xBoxA.h"

using namespace Math::Figures;

struct xFaceList {
    xWORD      indexOffset; // first face in the facesP array
    xWORD      indexCount;  // no of faces in the facesP array

    bool       smooth;
    xBYTE      materialId;
    xMaterial *materialP;
};

typedef xWORD3 xFace;

struct xEdge {
    xWORD face1;
    xWORD face2;
    xWORD vert1;
    xWORD vert2;
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

    void Subdivide(const xElement *elem, float scale, int depth, const xBoxA &bounding);
};

#endif
