#ifndef __incl_lib3dx_xRender_h
#define __incl_lib3dx_xRender_h

#include "xModel.h"
#include "xSkeleton.h"
#include <vector>

class xRender
{
  public:
    enum SelectionMode { smNone, smElement, smVertex };

    xFile    * xModel;
    xMatrix  * bonesM;
    xVector4 * bonesQ;
    xWORD      bonesC;

    virtual void RenderModel() = 0;

    virtual void RenderSkeleton( bool selectionRendering, xWORD selBoneId = -1 )  = 0;

    virtual void RenderVertices( SelectionMode         selectionMode    = smNone,
                                 xWORD                 selElementId     = -1,
                                 std::vector<xDWORD> * selectedVertices = NULL )  = 0;

    virtual void RenderFaces   ( xWORD                 selectedElement,
                                 std::vector<xDWORD> * facesToRender ) = 0;

    xRender () : xModel(NULL) {}
    virtual ~xRender() {}

    virtual void Initialize(xFile* model)
    {
        xModel = model;
        bonesM = NULL;
        bonesQ = NULL;
        bonesC = 0;
    }

    virtual void Invalidate()  = 0;

    virtual void FreeRenderData()
    {
        FreeElementRenderData(xModel->firstP);
    }

    virtual void CalculateSkeleton()
    {
        if (!xModel->spineP) return;
        if (xBoneChildCount(xModel->spineP)+1 != bonesC)
        {
            if (bonesM) { delete[] bonesM; bonesM = NULL; }
            if (bonesQ) { delete[] bonesQ; bonesQ = NULL; }
            if (!bonesC) FreeRenderData(); // skeleton was added
        }
        xBoneCalculateMatrices (xModel->spineP, bonesM, bonesC);
        xBoneCalculateQuats    (xModel->spineP, bonesQ, bonesC);
    }

    virtual void Finalize()
    {
        if (bonesM) delete[] bonesM;
        if (bonesQ) delete[] bonesQ;
        if (xModel) { FreeRenderData(); xFileFree(xModel); }
        xModel = NULL;
    }

  private:
    virtual void InvalidateElementRenderData(xElement *elem)                  = 0;
    virtual void FreeElementRenderData(xElement *elem, bool listOnly = false) = 0;
};

#endif
