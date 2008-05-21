#ifndef __incl_lib3dx_xRenderGL_h
#define __incl_lib3dx_xRenderGL_h

#include "xRender.h"
#include "../../GLExtensions/aGL_Extensions.h"

#include "../../Utils/Debug.h"

class xRenderGL : public xRender
{
  public:
    virtual void RenderModel    ( bool transparent, const xFieldOfView *FOV );
    virtual void RenderSkeleton ( bool selectionRendering, xWORD selBoneId = xWORD_MAX);
    virtual void RenderVertices ( SelectionMode         selectionMode    = smNone,
                                  xWORD                 selElementId     = xWORD_MAX,
                                  std::vector<xDWORD> * selectedVertices = NULL );
    virtual void RenderFaces    ( xWORD                 selectedElement,
                                  std::vector<xDWORD>  * facesToRender );

    virtual void   RenderShadow   ( const xShadowMap &shadowMap, const xFieldOfView *FOV );
            void   RenderShadowMap( bool transparent );
    virtual xDWORD CreateShadowMap(xWORD width, xMatrix &mtxBlockerToLight);

    virtual void RenderShadowVolume(xLight &light, xFieldOfView *FOV);

    xRenderGL() : UseVBO(agl_VBOLoaded), UseList(true) {};

    virtual void Initialize(bool isStatic, HModel hGrModel, HModel hPhModel = HModel())
    {
        xRender::Initialize(isStatic, hGrModel, hPhModel);
        UseVBO = agl_VBOLoaded && !isStatic;
    }

    virtual void Invalidate()
    {
        xRender::Invalidate();

        xElementInstance *iter;
        xBYTE cnt;
        if (instanceDataGrP)
        {
            cnt = instanceDataGrC;
            for (iter = instanceDataGrP; cnt; --cnt, ++iter)
            {
                iter->vertexB = 0;
                iter->normalB = 0;
                iter->indexB = 0;
                iter->mode = xElementInstance::xRenderMode_NULL;
            }
        }
        if (instanceDataPhP)
        {
            cnt = instanceDataPhC;
            for (iter = instanceDataPhP; cnt; --cnt, ++iter)
            {
                iter->vertexB = 0;
                iter->normalB = 0;
                iter->indexB = 0;
                iter->mode = xElementInstance::xRenderMode_NULL;
            }
        }
    }

    virtual void Finalize()
    {
        if (this->shadowMapTexId)
        {
            glDeleteTextures(1, (GLuint*)&this->shadowMapTexId);
            this->shadowMapTexId = 0;
        }

        FreeAllRenderData();
        if (instanceDataGrP)
        {
            delete[] instanceDataGrP;
            instanceDataGrP = NULL;
        }
        if (instanceDataPhP)
        {
            if (hModelGraphics != hModelPhysical)
                delete[] instanceDataPhP;
            instanceDataPhP = NULL;
        }

        xRender::Finalize();
    }

    void FreeListRenderData(xElementInstance *instanceDataP, xBYTE instanceDataC)
    {
        if (instanceDataP)
        {
            xElementInstance *iter = instanceDataP;
            for (int i = instanceDataC; i; --i, ++iter)
                if (iter->mode == xElementInstance::xRenderMode_LIST)
                {
                    if (iter->listID)       glDeleteLists(iter->listID, 1);
                    if (iter->listIDTransp) glDeleteLists(iter->listIDTransp, 1);
                    iter->listID = 0;
                    iter->listIDTransp = 0;
                    iter->mode = xElementInstance::xRenderMode_NULL;
                }
        }
    }
    void FreeVBORenderData(xElementInstance *instanceDataP, xBYTE instanceDataC)
    {
        if (instanceDataP)
        {
            xElementInstance *iter = instanceDataP;
            for (int i = instanceDataC; i; --i, ++iter)
                if (iter->mode == xElementInstance::xRenderMode_VBO)
                {
                    GLuint p = iter->vertexB;
                    if (p) glDeleteBuffersARB(1, &p);
                    p = iter->normalB;
                    if (p) glDeleteBuffersARB(1, &p);
                    p = iter->indexB;
                    if (p) glDeleteBuffersARB(1, &p);
                    iter->vertexB = 0;
                    iter->normalB = 0;
                    iter->indexB = 0;
                    iter->mode = xElementInstance::xRenderMode_NULL;
                }
        }
    }
    void FreeListRenderData()
    {
        FreeListRenderData(instanceDataGrP, instanceDataGrC);
        if (hModelGraphics != hModelPhysical)
            FreeListRenderData(instanceDataPhP, instanceDataPhC);
    }
    void FreeVBORenderData()
    {
        FreeVBORenderData(instanceDataGrP, instanceDataGrC);
        if (hModelGraphics != hModelPhysical)
            FreeVBORenderData(instanceDataPhP, instanceDataPhC);
    }

    virtual void FreeAllRenderData()
    {
        if (this->shadowMapTexId)
        {
            glDeleteTextures(1, (GLuint*)&this->shadowMapTexId);
            this->shadowMapTexId = 0;
        }
        if (UseVBO) FreeVBORenderData();
        else        FreeListRenderData();
    }

    virtual void VerticesChanged() // need to refresh VBO & list data
    {
        FreeAllRenderData();
        xRender::VerticesChanged();
    }

    virtual void CalculateSkeleton()
    {
        xRender::CalculateSkeleton();
        if (!UseVBO) FreeListRenderData();
    }

  private:
    bool       UseVBO;
    bool       UseList;
    xMaterial *m_currentMaterial;

    void RenderBone(const xBone * bone, bool selectionRendering, xWORD selBoneId);

    void InitVBO     (xElement *elem, xElementInstance *instance);
    void SetMaterial (xColor color, xMaterial *mat);

    void RenderElementVerticesVBO( xElement            * elem,
                                   SelectionMode         selectionMode,
                                   xWORD                 selElementId,
                                   std::vector<xDWORD> * selectedVertices);
    void RenderElementVerticesLST( xElement            * elem,
                                   SelectionMode         selectionMode,
                                   xWORD                 selElementId,
                                   std::vector<xDWORD> * selectedVertices);

    void RenderElementFacesVBO( xElement            * elem,
                                xWORD                 selElementId,
                                std::vector<xDWORD> * facesToRender);
    void RenderElementFacesLST( xElement            * elem,
                                xWORD                 selElementId,
                                std::vector<xDWORD> * facesToRender);

    void RenderModelVBO( xElement * elem, bool transparent );
    void RenderModelLST( xElement * elem, bool transparent );

    void RenderShadowLST( xElement *elem );
    void RenderShadowVBO( xElement *elem );

    void RenderShadowMapLST( xElement *elem, bool transparent );
    void RenderShadowMapVBO( xElement *elem, bool transparent );

    void RenderShadowVolumeElem(xElement *elem, xLight &light);
    void RenderShadowVolumeZPass(xElement *elem, xLight &light);
    void RenderShadowVolumeZFail(xElement *elem, xLight &light);
};

#endif
