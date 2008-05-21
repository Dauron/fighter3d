#ifndef __incl_lib3dx_xRender_h
#define __incl_lib3dx_xRender_h

#include "../ModelMgr.h"
#include "xSkeleton.h"
#include "../../Math/xFieldOfView.h"
#include "xUtils.h"

struct xShadowMap
{
    xDWORD  texId;
    xMatrix receiverUVMatrix;
};

class xRender
{
protected:
    HModel hModelGraphics;
    HModel hModelPhysical;
    xFile    * xModelToRender;

    //static xDWORD*     texture;
    //static xWORD       shadowWidth;
    xDWORD             shadowTexId;

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
        bool rend_graph = xModelToRender == xModelGraphics;
        bool rend_phys  = xModelToRender == xModelPhysical;
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

    xFile    * xModelGraphics;
    xFile    * xModelPhysical;

    xBone    * spineP; // common spine of the model
    xMatrix  * bonesM;
    xVector4 * bonesQ;
    xWORD      bonesC;
    const xFieldOfView *FOV;

    virtual void RenderModel( bool transparent, const xFieldOfView *FOV ) = 0;
    virtual void RenderShadow(const xShadowMap &shadowMap, const xFieldOfView *FOV) = 0;

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
        shadowTexId = 0;
    }

    virtual void Finalize()
    {
        if (bonesM) delete[] bonesM;
        if (bonesQ) delete[] bonesQ;

        g_ModelMgr.DeleteModel(hModelGraphics);
        g_ModelMgr.DeleteModel(hModelPhysical);
        hModelGraphics = hModelPhysical = HModel();
        xModelPhysical = xModelGraphics = xModelToRender = NULL;
    }

    void CopySpineToPhysical()
    {
        if (xModelGraphics != xModelPhysical)
        {
            xBoneFree(xModelPhysical->spineP);
            xBoneCopy(xModelGraphics->spineP, xModelPhysical->spineP);
        }
    }
    void CopySpineToGraphics()
    {
        if (xModelGraphics != xModelPhysical)
        {
            xBoneFree(xModelGraphics->spineP);
            xBoneCopy(xModelPhysical->spineP, xModelGraphics->spineP);
            spineP = xModelGraphics->spineP;
        }
    }

    virtual void VerticesChanged()
    {
        PrepareInstanceData(instanceDataGrP, instanceDataGrC = xModelGraphics->elementC);
        xFile_GetBounds(xModelGraphics, bonesM, instanceDataGrP);
        if (hModelGraphics != hModelPhysical)
        {
            PrepareInstanceData(instanceDataPhP, instanceDataPhC = xModelPhysical->elementC);
            xFile_GetBounds(xModelPhysical, bonesM, instanceDataPhP);
        }
        else
        {
            instanceDataPhC = instanceDataGrC;
            instanceDataPhP = instanceDataGrP;
        }
    }

    virtual void CalculateSkeleton()
    {
        if (!spineP) return;
        if (xBoneChildCount(spineP)+1 != bonesC)
        {
            if (bonesM) { delete[] bonesM; bonesM = NULL; }
            if (bonesQ) { delete[] bonesQ; bonesQ = NULL; }
            if (!bonesC) // skeleton was added, so refresh VBO data
                VerticesChanged();
        }
        xBoneCalculateMatrices (spineP, bonesM, bonesC);
        xBoneCalculateQuats    (spineP, bonesQ, bonesC);

        PrepareInstanceData(instanceDataGrP, instanceDataGrC = xModelGraphics->elementC);
        xFile_GetBounds(xModelGraphics, bonesM, instanceDataGrP);
        if (hModelGraphics != hModelPhysical)
        {
            PrepareInstanceData(instanceDataPhP, instanceDataPhC = xModelPhysical->elementC);
            xFile_GetBounds(xModelPhysical, bonesM, instanceDataPhP);
        }
        else
        {
            instanceDataPhC = instanceDataGrC;
            instanceDataPhP = instanceDataGrP;
        }
    }

    virtual xDWORD CreateShadowMap   (xWORD width, xMatrix &mtxBlockerToLight) = 0;
};

#endif
