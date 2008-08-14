#ifndef __incl_lib3dx_xElementData_h
#define __incl_lib3dx_xElementData_h

#include "xMaterial.h"
#include "../../Math/Figures/xBoxA.h"

using namespace Math::Figures;

struct xFaceList {
    xWORD      I_offset; // first face in the L_faces array
    xWORD      I_count;  // no of faces in the L_faces array

    bool       FL_smooth;
    xBYTE      ID_material;
    xMaterial *Material;
};

typedef xWORD3 xFace;

struct xEdge {
    xWORD ID_face_1;
    xWORD ID_face_2;
    xWORD ID_vert_1;
    xWORD ID_vert_2;
};

struct xModel;
struct xElement;
struct xCollisionHierarchy;

struct xCollisionData
{
    xWORD                I_kids;
    xCollisionHierarchy *L_kids;

    void Load( FILE *file, xElement *elem );
    void Save( FILE *file, xElement *elem );

    void FreeKids();
    void Fill ( xModel *xmodel, xElement *elem );

    static const xBYTE MAX_HIERARCHY_DEPTH = 10;
};

struct xCollisionHierarchy : xCollisionData
{
    xWORD    I_faces;
    xFace ** L_faces;
    xWORD    I_vertices;
    xWORD *  L_vertices;

    void Load( FILE *file, xElement *elem );
    void Save( FILE *file, xElement *elem );

    void Subdivide(const xElement *elem, float scale, int depth, const xBoxA &bounding);
};

#endif
