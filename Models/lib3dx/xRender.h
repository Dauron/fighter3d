#ifndef __incl_lib3dx_xRender_h
#define __incl_lib3dx_xRender_h

#include "../ModelMgr.h"
#include "xBone.h"
#include "../../Math/xFieldOfView.h"
#include "../../Math/xLight.h"
#include "xUtils.h"

struct xShadowMap
{
    xDWORD  texId;
    xMatrix receiverUVMatrix;
};

class xRender
{
protected:
    HModel     hModelGraphics;
    HModel     hModelPhysical;
    xModel   * xModelToRender;

    xDWORD             shadowMapTexId;

    xElementInstance  *instanceDataGrP;
    xBYTE              instanceDataGrC;
    xElementInstance  *instanceDataPhP;
    xBYTE              instanceDataPhC;
    xElementInstance  *instanceDataTRP;
    xBYTE              instanceDataTRC;

    xMatrix            location;

public:
    enum SelectionMode { smNone, smElement, smVertex };
    enum RenderingMode { rmGraphical, rmPhysical };

    void UpdatePointers()
    {
        bool rend_graph = xModelToRender == xModelGraphics;
        xModelGraphics = g_ModelMgr.GetModel(hModelGraphics)->model;
        xModelPhysical = g_ModelMgr.GetModel(hModelPhysical)->model;
        xModelToRender = rend_graph ? xModelGraphics : xModelPhysical;
        instanceDataTRP = rend_graph ? instanceDataGrP : instanceDataPhP;
        instanceDataTRC = rend_graph ? instanceDataGrC : instanceDataPhC;
        spineP = xModelGraphics->spineP;
    }
    void SetRenderMode(RenderingMode mode)
    {
        xModelToRender =  (mode == rmGraphical) ? xModelGraphics : xModelPhysical;
        instanceDataTRP = (mode == rmGraphical) ? instanceDataGrP : instanceDataPhP;
        instanceDataTRC = (mode == rmGraphical) ? instanceDataGrC : instanceDataPhC;
    }

    void SetLocation(const xMatrix &locationMatrix)
    {
        location = locationMatrix;
    }

    void PrepareInstanceData(xElementInstance *&instanceDataP, xBYTE instanceDataC)
    {
        if (!instanceDataP)
        {
            instanceDataP = new xElementInstance[instanceDataC];
            memset(instanceDataP, 0, sizeof(xElementInstance)*instanceDataC);
        }
    }

    void PrepareInstanceDataTr()
    {
        PrepareInstanceData(instanceDataTRP, instanceDataTRC = xModelToRender->elementC);
        bool rend_graph = xModelToRender == xModelGraphics;
        if (rend_graph)
        {
            instanceDataGrP = instanceDataTRP;
            instanceDataGrC = instanceDataTRC;
        }
        else
        {
            instanceDataPhP = instanceDataTRP;
            instanceDataPhC = instanceDataTRC;
        }
    }

    void InstanceDataGet(xElementInstance *&instanceP, xBYTE &instanceC)
    {
        PrepareInstanceDataTr();
        instanceP = instanceDataTRP;
        instanceC = instanceDataTRC;
    }

    xModel    * xModelGraphics;
    xModel    * xModelPhysical;

    xBone    * spineP; // common spine of the model
    xMatrix  * bonesM;
    xVector4 * bonesQ;
    xBYTE      bonesC;
    bool     * bonesMod;
    const xFieldOfView *FOV;

    virtual void RenderModel              ( bool transparent, const xFieldOfView *FOV ) = 0;
    virtual void RenderShadowVolume       ( xLight &light, xFieldOfView *FOV ) = 0;
    virtual void RenderShadowMap          ( const xShadowMap &shadowMap, const xFieldOfView *FOV ) = 0;
    virtual xDWORD CreateShadowMapTexture ( xWORD width, xMatrix &mtxBlockerToLight ) = 0;

    virtual void RenderSkeleton( bool selectionRendering, xWORD selBoneId = xWORD_MAX ) = 0;

    virtual void RenderVertices( SelectionMode        selectionMode    = smNone,
                                 xWORD                 selElementId     = xWORD_MAX,
                                 std::vector<xDWORD> * selectedVertices = NULL )  = 0;

    virtual void RenderFaces   ( xWORD                 selectedElement,
                                 std::vector<xDWORD> * facesToRender ) = 0;

    xRender () : xModelToRender(NULL), xModelGraphics(NULL), xModelPhysical(NULL) {}
    virtual ~xRender() {}

    virtual void Initialize(bool isStatic, HModel hGrModel, HModel hPhModel = HModel());

    virtual void Invalidate()
    {
        shadowMapTexId = 0;
    }

    virtual void Finalize()
    {
        if (bonesM) delete[] bonesM;
        if (bonesQ) delete[] bonesQ;
        if (bonesMod) delete[] bonesMod;

        g_ModelMgr.DeleteModel(hModelGraphics);
        g_ModelMgr.DeleteModel(hModelPhysical);
        hModelGraphics = hModelPhysical = HModel();
        xModelPhysical = xModelGraphics = xModelToRender = NULL;
        FreeShadowRenderData();
    }

    void CopySpine(const xBone *src, xBone *&dst)
    {
        if (src != dst)
        {
            if (dst) dst->Free();
            dst = src ? src->Clone() : NULL;
        }
    }

    void CopySpineToPhysical()
    {
        CopySpine(xModelGraphics->spineP, xModelPhysical->spineP);
    }
    void CopySpineToGraphics()
    {
        CopySpine(xModelPhysical->spineP, xModelGraphics->spineP);
    }

    void FreeShadowRenderData(xElementInstance *instanceDataP, xBYTE instanceDataC)
    {
        if (instanceDataP)
        {
            xElementInstance *iter = instanceDataP;
            for (int i = instanceDataC; i; --i, ++iter)
                iter->FreeShadowData();
        }
    }
    void FreeShadowRenderData()
    {
        FreeShadowRenderData(instanceDataGrP, instanceDataGrC);
        if (hModelGraphics != hModelPhysical)
            FreeShadowRenderData(instanceDataPhP, instanceDataPhC);
    }

    void InvalidateShadowRenderData(xElementInstance *instanceDataP, xBYTE instanceDataC)
    {
        if (instanceDataP)
        {
            xElementInstance *iter = instanceDataP;
            for (int i = instanceDataC; i; --i, ++iter)
                iter->InvalidateShadowData();
        }
    }
    void InvalidateShadowRenderData()
    {
        InvalidateShadowRenderData(instanceDataGrP, instanceDataGrC);
        if (hModelGraphics != hModelPhysical)
            InvalidateShadowRenderData(instanceDataPhP, instanceDataPhC);
    }

    virtual void VerticesChanged()
    {
        PrepareInstanceData(instanceDataGrP, instanceDataGrC = xModelGraphics->elementC);
        xModel_GetBounds(xModelGraphics, bonesM, instanceDataGrP);
        if (hModelGraphics != hModelPhysical)
        {
            PrepareInstanceData(instanceDataPhP, instanceDataPhC = xModelPhysical->elementC);
            xModel_GetBounds(xModelPhysical, bonesM, instanceDataPhP);
        }
        else
        {
            instanceDataPhC = instanceDataGrC;
            instanceDataPhP = instanceDataGrP;
        }
        FreeShadowRenderData();
    }

    virtual void CalculateSkeleton()
    {
        if (!spineP) return;
        if (spineP->CountAllKids()+1 != bonesC)
        {
            if (bonesM)   { delete[] bonesM; bonesM = NULL; }
            if (bonesQ)   { delete[] bonesQ; bonesQ = NULL; }
            if (bonesMod) { delete[] bonesMod; bonesMod = NULL; }
            if (!bonesC) // skeleton was added, so refresh VBO data
                VerticesChanged();
        }
        xBoneCalculateMatrices (spineP, bonesM, bonesMod, bonesC);
        xBoneCalculateQuats    (spineP, bonesQ, bonesC);

        PrepareInstanceData(instanceDataGrP, instanceDataGrC = xModelGraphics->elementC);
        xModel_GetBounds(xModelGraphics, bonesM, instanceDataGrP);
        if (hModelGraphics != hModelPhysical)
        {
            PrepareInstanceData(instanceDataPhP, instanceDataPhC = xModelPhysical->elementC);
            xModel_GetBounds(xModelPhysical, bonesM, instanceDataPhP);
        }
        else
        {
            instanceDataPhC = instanceDataGrC;
            instanceDataPhP = instanceDataGrP;
        }
        //InvalidateShadowRenderData();
    }
};

#endif
