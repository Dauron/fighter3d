#ifndef __incl_lib3dx_xElementRunTime_h
#define __incl_lib3dx_xElementRunTime_h

#include <vector>
#include "xVertex.h"
#include "xElementData.h"
#include "../../Math/xLight.h"
#include "../../Math/Figures/xSphere.h"
#include "../../Math/Figures/xBoxO.h"

union xGPUPointers
{
    typedef enum {
        NONE = 0,
        VBO  = 1,
        LIST = 2
    } Mode;

    struct {
        xDWORD vertexB;
        xDWORD normalB;
        xDWORD indexB;
        xDWORD colorB;
    };
    struct {
        xDWORD listID;          // Compiled Render List
        xDWORD listIDTex;       // Compiled Render List
        xDWORD listIDTransp;    // Compiled Render List for transparent faces
        xDWORD listIDTexTransp; // Compiled Render List
    };

    void Invalidate()
    {
        memset(this, 0, sizeof(xGPUPointers));
    }
};

union xGPUShadowPointers
{
    struct {
        xDWORD vertexB;
        xDWORD indexB;
    };
    struct {
        xDWORD listIDPass;
        xDWORD listIDFail;
        xDWORD listIDFailS;
        xDWORD listIDFailF;
        xDWORD listIDFailB;
    };

    void Invalidate()
    {
        memset(this, 0, sizeof(xGPUShadowPointers));
    }
};

struct xShadowData
{
    enum xShadowDataLevel
    {
        LIGHT_ONLY,
        ZPASS_ONLY,
        ZFAIL_PASS
    };

    xGPUPointers       gpuLightPointers;
    xGPUShadowPointers gpuShadowPointers;

    xBYTE            ID_light;
    xShadowDataLevel zDataLevel;

    xVector4 *L_vertices;
    xWORD    *L_indices;
    size_t    I_indices;
    xWORD     I_sides;
    xWORD     I_fronts;
    xWORD     I_backs;

    void FreeData()
    {
        if (L_indices)  delete[] L_indices;
        if (L_vertices) delete[] L_vertices;
        L_indices  = NULL;
        L_vertices = NULL;
    }

    void InvalidateData()
    {
        if (L_indices) delete[] L_indices;
        L_indices = NULL;
    }
};

typedef std::vector<xShadowData> xShadowDataVector;

struct xElementInstance
{
    xGPUPointers::Mode mode;

    xGPUPointers      gpuMain;
    xShadowDataVector gpuShadows;

    Math::Figures::xBoxO    bBox;
    Math::Figures::xSphere  bSphere;
    Math::Figures::xBoxO   *bBox_T;
    Math::Figures::xSphere *bSphere_T;

    xWORD     I_vertices;
    xPoint4  *L_vertices; // skinned
    xVector3 *L_normals;  // skinned

    void         Transform (const xMatrix &MX_MeshToWorld)
    {
        if (!bSphere_T) bSphere_T = (xSphere*)bSphere.Transform(MX_MeshToWorld);
        if (!bBox_T)  { bBox_T    = (xBoxO*)  bBox.Transform(MX_MeshToWorld); bBox_T->FillCorners(); }
    }

    xShadowData &GetShadowData(xLight &light, xShadowData::xShadowDataLevel zLevel);
    void         Zero();
    void         Free();
    void         Clear();
    void         InvalidateVertexData();
};

struct xRenderData
{
    union {                  // smooth vertices
        xVertex        *L_vertices;
        xVertexTex     *L_verticesT;
        xVertexSkel    *L_verticesS;
        xVertexTexSkel *L_verticesTS;
    };
    xWORD              I_vertices;
    xVector3          *L_normals; // smooth normals
    xFace             *L_faces;
    xColor3b          *L_colors;

    xVector3          *L_face_normals;

    xGPUPointers::Mode mode;
    xGPUPointers       gpuMain;

    xRenderData() { Clear(); }

    void Clear()
    {
        memset(this, 0, sizeof(xRenderData));
    }
    void Destroy(const xElement &elem);

    void Load( FILE *file, xElement &elem );
    void Save( FILE *file, const xElement &elem ) const;
};

struct xSkinnedData
{
    xVector3 *L_vertices;
    xVector3 *L_normals;
};

#endif
