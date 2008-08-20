#ifndef __incl_lib3dx_xElement_h
#define __incl_lib3dx_xElement_h

#include "xVertex.h"
#include "xElementData.h"
#include "xElementRunTime.h"
#include "../../Math/Figures/xBVHierarchy.h"
#include "../../Math/Figures/xBoxA.h"
#include "../../Math/Figures/xMesh.h"

struct xModel;

struct xElement {
    xBYTE       ID;
    char       *Name;
    xColor      Color;

    xMatrix     MX_MeshToLocal;
    union {
        xVertex        *L_vertices;
        xVertexTex     *L_verticesT;
        xVertexSkel    *L_verticesS;
        xVertexTexSkel *L_verticesTS;
    };
    xWORD       I_vertices;

    xDWORD     *L_smooth;
    xFace      *L_faces;
    xWORD       I_faces;
    xFaceList  *L_faceLists;
    xWORD       I_faceLists;
    xEdge      *L_edges;
    xWORD       I_edges;

    bool       FL_textured;     // Are there texture coordinates attached
    bool       FL_skeletized;   // Are there bone weights attached
    bool       FL_transparent;  // Are there transparent faces?
    bool       FL_opaque;       // Are there opaque faces?

    xElement  *Next;   // next sibling
    xElement  *L_kids; // first kid
    xBYTE      I_kids; // no of kids

    xCollisionData    collisionData;
    xRenderData       renderData;

    xElement  *ById     ( xBYTE eid );
    xBYTE      CountAll ();

    size_t     GetVertexStride() const {
        return FL_skeletized
            ? (FL_textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel))
            : (FL_textured ? sizeof(xVertexTex)     : sizeof(xVertex));
    }

           void      Free();
           void      Save( FILE *file, const xModel *xmodel );
    static xElement *Load( FILE *file, xModel *xmodel, bool FL_create_CollisionInfo );

    void         FillShadowEdges();
    void         CalculateSmoothVertices();
    xSkinnedData GetSkinnedVertices(const xMatrix *bones) const;

    void                 FillCollisionInfo (xModel &xmodel);
    Math::Figures::xBoxA FillBVH  ( Math::Figures::xBVHierarchy *L_BVH, Math::Figures::xMeshData *MeshData );
    Math::Figures::xBoxA ReFillBVH( Math::Figures::xBVHierarchy *L_BVH, Math::Figures::xMeshData *MeshData, const xMatrix &MX_LocalToWorld );

private:
    Math::Figures::xBoxA FillBVHNode(xCollisionHierarchy            &CH_node,
                                     Math::Figures::xBVHierarchy    &BVH_node,
                                     Math::Figures::xMeshData       &MeshData);
    Math::Figures::xBoxA ReFillBVHNode(Math::Figures::xBVHierarchy    &BVH_node,
                                       Math::Figures::xMeshData       &MeshData,
                                       const xMatrix &MX_LocalToWorld );
};

// Other methods
xMaterial *xFaceGetMaterial(const xElement *elem, int faceIdx);
bool       xFaceTransparent(const xElement *elem, int faceIdx);

#endif
