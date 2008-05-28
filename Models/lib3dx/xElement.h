#ifndef __incl_lib3dx_xElement_h
#define __incl_lib3dx_xElement_h

#include "xVertex.h"
#include "xElementData.h"
#include "xElementRunTime.h"

struct xModel;

struct xElement {
    xBYTE       id;
    char       *name;
    xColor      color;

    xMatrix     matrix;
    union {
        xVertex        *verticesP;
        xVertexTex     *verticesTP;
        xVertexSkel    *verticesSP;
        xVertexTexSkel *verticesTSP;
    };
    xWORD       verticesC;

    xDWORD     *smoothP;
    xFace      *facesP;
    xWORD       facesC;
    xFaceList  *faceListP;
    xWORD       faceListC;
    xEdge      *edgesP;
    xWORD       edgesC;

    bool       textured;
    bool       skeletized;
    bool       transparent;  // Are there transparent faces?
    bool       opaque;       // Are there opaque faces?

    xElement  *nextP; // next sibling
    xElement  *kidsP; // first kid
    xBYTE      kidsC; // no of kids

    xCollisionData    collisionData;
    xRenderData       renderData;

    xElement  *ById     ( xBYTE eid );
    xBYTE      CountAll ();

    size_t     GetVertexStride() {
        return skeletized
            ? (textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel))
            : (textured ? sizeof(xVertexTex) : sizeof(xVertex));
    }

    void         FillShadowEdges();
    void         CalculateSmoothVertices();
    xSkinnedData GetSkinnedVertices(const xMatrix *bones) const;

           void      Free();
           void      Save( FILE *file, const xModel *xmodel );
    static xElement *Load( FILE *file, xModel *xmodel, bool createCollisionInfo );
};

// Other methods
xMaterial *xFaceGetMaterial(const xElement *elem, int faceIdx);
bool       xFaceTransparent(const xElement *elem, int faceIdx);

#endif
