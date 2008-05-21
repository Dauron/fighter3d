#ifndef __incl_lib3dx_xRenderGL_h
#define __incl_lib3dx_xRenderGL_h

#include "xRender.h"
#include "../../GLExtensions/ARB_vertex_buffer_object.h"
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

    virtual void   RenderShadowVolume    ( xLight &light, xFieldOfView *FOV );
    virtual void   RenderShadowMap       ( const xShadowMap &shadowMap, const xFieldOfView *FOV );
    virtual xDWORD CreateShadowMapTexture( xWORD width, xMatrix &mtxBlockerToLight );

    xRenderGL() : UseVBO(GLExtensions::Exists_ARB_VertexBufferObject), UseList(true) {};

    virtual void Initialize(bool isStatic, HModel hGrModel, HModel hPhModel = HModel())
    {
        xRender::Initialize(isStatic, hGrModel, hPhModel);
        UseVBO = GLExtensions::Exists_ARB_VertexBufferObject;// && !isStatic;
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
                iter->gpuMain.vertexB = 0;
                iter->gpuMain.normalB = 0;
                iter->gpuMain.indexB = 0;
                iter->mode = xElementInstance::xRenderMode_NULL;
            }
        }
        if (instanceDataPhP)
        {
            cnt = instanceDataPhC;
            for (iter = instanceDataPhP; cnt; --cnt, ++iter)
            {
                iter->gpuMain.vertexB = 0;
                iter->gpuMain.normalB = 0;
                iter->gpuMain.indexB = 0;
                iter->mode = xElementInstance::xRenderMode_NULL;
            }
        }
    }

    virtual void Finalize()
    {
        if (this->shadowMapTexId)
        {
            GLuint tid = this->shadowMapTexId;
            glDeleteTextures(1, &tid);
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
                    if (iter->gpuMain.listID)       glDeleteLists(iter->gpuMain.listID, 1);
                    if (iter->gpuMain.listIDTransp) glDeleteLists(iter->gpuMain.listIDTransp, 1);
                    iter->gpuMain.listID = 0;
                    iter->gpuMain.listIDTransp = 0;
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
                    GLuint p = iter->gpuMain.vertexB;
                    if (p) glDeleteBuffersARB(1, &p);
                    p = iter->gpuMain.normalB;
                    if (p) glDeleteBuffersARB(1, &p);
                    p = iter->gpuMain.indexB;
                    if (p) glDeleteBuffersARB(1, &p);
                    iter->gpuMain.vertexB = 0;
                    iter->gpuMain.normalB = 0;
                    iter->gpuMain.indexB = 0;
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
            GLuint tid = this->shadowMapTexId;
            glDeleteTextures(1, &tid);
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

    void RenderShadowMapTexture( bool transparent );
    void RenderShadowMapLST    ( xElement *elem );
    void RenderShadowMapVBO    ( xElement *elem );

    void RenderShadowMapTextureLST( xElement *elem, bool transparent );
    void RenderShadowMapTextureVBO( xElement *elem, bool transparent );

    void RenderShadowVolumeElem(xElement *elem, xLight &light);
    void RenderShadowVolumeZPass(xShadowData &shadowData, bool infiniteL);
    void RenderShadowVolumeZFail(xShadowData &shadowData, bool infiniteL);
};

#endif
