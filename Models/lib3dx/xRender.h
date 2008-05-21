#ifndef __incl_lib3dx_xRender_h
#define __incl_lib3dx_xRender_h

#include "../ModelMgr.h"
#include "xSkeleton.h"
#include "xLight.h"

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

    static xDWORD*     texture;
    static xWORD       shadowWidth;
    xDWORD             shadowTexId;

public:
    enum SelectionMode { smNone, smElement, smVertex };

    void UpdatePointers()
    {
        bool rend_graph = xModelToRender == xModelGraphics;
        xModelGraphics = g_ModelMgr.GetModel(hModelGraphics)->model;
        xModelPhysical = g_ModelMgr.GetModel(hModelPhysical)->model;
        xModelToRender = rend_graph ? xModelGraphics : xModelPhysical;
        spineP = xModelGraphics->spineP;
    }

    xFile    * xModelToRender;
    xFile    * xModelGraphics;
    xFile    * xModelPhysical;

    xBone    * spineP; // common spine of the model
    xMatrix  * bonesM;
    xVector4 * bonesQ;
    xWORD      bonesC;

    virtual void RenderModel( bool transparent ) = 0;
    virtual void RenderShadow(const xShadowMap &shadowMap, const xMatrix &locationMatrix) = 0;

    virtual void RenderSkeleton( bool selectionRendering, xWORD selBoneId = xWORD_MAX )  = 0;

    virtual void RenderVertices( SelectionMode        selectionMode    = smNone,
                                 xWORD                 selElementId     = xWORD_MAX,
                                 std::vector<xDWORD> * selectedVertices = NULL )  = 0;

    virtual void RenderFaces   ( xWORD                 selectedElement,
                                 std::vector<xDWORD> * facesToRender ) = 0;

    xRender () : xModelToRender(NULL), xModelGraphics(NULL), xModelPhysical(NULL) {}
    virtual ~xRender() {}

    virtual void Initialize(bool isStatic, HModel hGrModel, HModel hPhModel = HModel())
    {
        hModelGraphics = hGrModel;
        Model3dx *mdl = g_ModelMgr.GetModel(hGrModel);
        xModelGraphics = mdl->model;
        if (hPhModel != HModel())
        {
            hModelPhysical = hPhModel;
            xModelPhysical = g_ModelMgr.GetModel(hPhModel)->model;
        }
        else
        {
            hModelPhysical = hModelGraphics;
            xModelPhysical = xModelGraphics;
            mdl->IncReferences();
        }
        xModelToRender = xModelGraphics;
        spineP = xModelGraphics->spineP;
        bonesM = NULL;
        bonesQ = NULL;
        bonesC = 0;
        shadowTexId  = 0;
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

    void Invalidate()
    {
        shadowTexId = 0;
    }

    virtual void FreeRenderData()
    {
        g_ModelMgr.GetModel(hModelGraphics)->FreeRenderData(false);
        g_ModelMgr.GetModel(hModelPhysical)->FreeRenderData(false);
    }

    virtual void CalculateSkeleton()
    {
        if (!spineP) return;
        if (xBoneChildCount(spineP)+1 != bonesC)
        {
            if (bonesM) { delete[] bonesM; bonesM = NULL; }
            if (bonesQ) { delete[] bonesQ; bonesQ = NULL; }
            if (!bonesC)  FreeRenderData(); // skeleton was added
        }
        xBoneCalculateMatrices (spineP, bonesM, bonesC);
        xBoneCalculateQuats    (spineP, bonesQ, bonesC);
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

    bool IsValid() { return hModelGraphics != HModel(); }

    virtual xDWORD CreateShadowMap   (xWORD width, xMatrix &mtxBlockerToLight) = 0;
};

#endif
