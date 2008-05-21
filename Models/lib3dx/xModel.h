#ifndef __incl_xModel_h
#define __incl_xModel_h

#include "../../Math/xMath.h"

// Materials
struct xTexture {
    char    *name;
    xDWORD   htex;
};

struct xMaterial {
    char    *name;
    xWORD    id;
    xColor   ambient;
    xColor   diffuse;
    xColor   specular;
    xFLOAT   shininess_gloss;
    xFLOAT   shininess_level;
    xFLOAT   transparency;
    bool     self_illum;
    char    *shader;
    bool     two_sided;
    bool     use_wire;
    bool     use_wire_abs;
    xFLOAT   wire_size;

    xTexture texture;

    xMaterial *nextP; // next sibling
};

// Vertices
union xVertex {
    struct {
        xFLOAT x, y, z; // vertex coordinates
    };
    struct {
        xFLOAT3 pos;
    };
};

union xVertexTex {
    struct {
        xFLOAT x, y, z, tx, ty; // vertex and texture coordinates
    };
    struct {
        xFLOAT3 pos;
        xFLOAT2 tex;
    };
};

union xVertexSkel {
    struct {
        xFLOAT x, y, z;        // vertex coordinates
        xFLOAT b0, b1, b2, b3; // bones
    };
    struct {
        xFLOAT3 pos;
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
        xFLOAT3 pos;
        xFLOAT4 bone; // up to 4 bones per vertex. boneIndex = floor(bone), boneInfluence = fract(bone)*10
        xFLOAT2 tex;
    };
};

// Faces, Elements, Bones, Files
struct xFaceList {
    xWORD      indexOffset; // first face in the facesP array
    xWORD      indexCount;  // no of faces in the facesP array
    xVector3  *normalP;     // non-smooth normals (not saved)

    bool       smooth;
    xWORD      materialId;
    xMaterial *materialP;
};

struct xElementInstance
{
    typedef enum {
        xRenderMode_NULL = 0,
        xRenderMode_VBO  = 1,
        xRenderMode_LIST = 2
    } xRenderMode;

    xRenderMode mode;

    union {
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
    xDWORD             verticesC;
    xVector3          *normalP; // smooth normals
    xWORD3            *facesP;
};

struct xCollisionHierarchy;
struct xCollisionData
{
    xWORD                kidsC;
    xCollisionHierarchy *kidsP;
};

struct xCollisionHierarchy : xCollisionData
{
    xWORD                 facesC;
    xWORD3             ** facesP;
    xWORD                 verticesC;
    xDWORD             *  verticesP;
};

struct xElement {
    xWORD       id;
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
    xWORD3     *facesP;
    xWORD       facesC;
    xFaceList  *faceListP;
    xWORD       faceListC;

    bool       textured;
    bool       skeletized;

    xElement  *nextP; // next sibling
    xElement  *kidsP; // first kid
    xWORD      kidsC; // no of kids

    xCollisionData    collisionData;
    xRenderData       renderData;
};

struct xBone {
    char      *name;
    xWORD      id;
    xVector4   quaternion;
    xVector3   ending;

    xBone     *nextP; // next sibling
    xBone     *kidsP; // first kid
    xWORD      kidsC; // no of kids
};

struct xFile {
    char      *fileName;

    xMaterial *materialP; // first material
    xWORD      materialC; // no of materials

    xElement  *firstP;    // first child
    xBYTE      elementC;  // count of all childs

    xBone     *spineP;    // spine of the model

    bool       texturesInited;
    bool       saveCollisionData;
    bool       transparent;  // Are there transparent faces?
    bool       opaque;       // Are there opaque faces?
};


// Other methods
xMaterial *xMaterialByName(const xFile *file, char *materialName);
xMaterial *xMaterialById  (const xFile *file,  xWORD materialId);
xElement  *xElementById   (const xFile* model, xWORD elementId = -1);
xWORD      xElementCount  (const xFile* model);

void       xBoneFree(xBone *bone);
void       xBoneCopy(const xBone *boneSrc, xBone *&boneDst);

xFile     *xFileLoad(const char *fileName, bool createCollisionInfo = true);
void       xFileSave(const xFile *xfile);
void       xFileFree(xFile *xfile);

void       xFaceListCalculateNormals(xElement *elem, xFaceList *faceL);
void       xElementCalculateSmoothVertices(xElement *elem);

#endif
