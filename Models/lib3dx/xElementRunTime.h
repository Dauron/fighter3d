#ifndef __incl_lib3dx_xElementRunTime_h
#define __incl_lib3dx_xElementRunTime_h

#include <vector>
#include "xVertex.h"
#include "xElementData.h"
#include "../../Math/xLight.h"
#include "../../Math/Figures/xBoxA.h"

using namespace Math::Figures;

union xGPUPointers
{
    struct {
        xDWORD vertexB;
        xDWORD normalB;
        xDWORD indexB;
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
        if (L_vertices) delete[] L_vertices;
        if (L_indices)  delete[] L_indices;
        L_vertices = NULL;
        L_indices  = NULL;
    }

    void InvalidateData()
    {
        if (L_indices) delete[] L_indices;
        L_indices = NULL;
    }
};

typedef std::vector<xShadowData> xShadowDataVector;

struct xGPURender {
    typedef enum {
        NONE = 0,
        VBO  = 1,
        LIST = 2
    } Mode;
};

struct xElementInstance
{
    xGPURender::Mode  mode;

    xGPUPointers      gpuMain;
    xShadowDataVector gpuShadows;

    xBoxA    bbBox;
    xVector3 bsCenter;
    xFLOAT   bsRadius;

    xWORD     I_vertices;
    xVector4 *L_vertices; // skinned
    xVector3 *L_normals;  // skinned

    xShadowData &GetShadowData(xLight &light, xShadowData::xShadowDataLevel zLevel);
    void         Zero();
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

    xVector3          *L_face_normals;

    xGPURender::Mode   mode;
    xGPUPointers       gpuMain;
};

struct xSkinnedData
{
    xVector3 *L_vertices;
    xVector3 *L_normals;
};

#endif
