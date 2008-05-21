#ifndef __incl_lib3dx_xElementRunTime_h
#define __incl_lib3dx_xElementRunTime_h

#include <vector>
#include "xVertex.h"
#include "xElementData.h"
#include "../../Math/xLight.h"

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

struct xShadowData
{
    enum xShadowDataLevel
    {
        LIGHT_ONLY,
        ZPASS_ONLY,
        ZFAIL_PASS
    };

    xGPUPointers gpuLightPointers;
    xGPUPointers gpuShadowPointers;

    xBYTE            lightId;
    xShadowDataLevel zDataLevel;

    xVector4 *verticesP;
    xWORD    *indexP;
    xWORD     sideC;
    xWORD     frontC;
    xWORD     backC;

    void FreeData()
    {
        if (verticesP) delete[] verticesP;
        if (indexP)    delete[] indexP;
        verticesP = NULL;
        indexP    = NULL;
    }

    void InvalidateData()
    {
        if (indexP) delete[] indexP;
        indexP = NULL;
    }
};

typedef std::vector<xShadowData> xShadowDataVector;

struct xElementInstance
{
    typedef enum {
        xRenderMode_NULL = 0,
        xRenderMode_VBO  = 1,
        xRenderMode_LIST = 2
    } xRenderMode;

    xRenderMode mode;

    xGPUPointers      gpuMain;
    xShadowDataVector gpuShadows;

    xBox     bbBox;
    xVector3 bsCenter;
    xFLOAT   bsRadius;

    xWORD     verticesC;
    xVector4 *verticesP; // skinned
    xVector3 *normalsP;  // skinned

    xShadowData &GetShadowData(xLight &light, xShadowData::xShadowDataLevel zLevel);
    void         Zero();
    void         Clear();
    void         InvalidateVertexData();
};

struct xRenderData
{
    union {                  // smooth vertices
        xVertex        *verticesP;
        xVertexTex     *verticesTP;
        xVertexSkel    *verticesSP;
        xVertexTexSkel *verticesTSP;
    };
    xWORD              verticesC;
    xVector3          *normalP; // smooth normals
    xFace             *facesP;

    xVector3          *faceNormalsP;
};

#endif
