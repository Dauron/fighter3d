#ifndef __incl_lib3dx_xRenderGL_h
#define __incl_lib3dx_xRenderGL_h

#include "../../Graphics/Renderer.h"
#include "../Extensions/ARB_vertex_buffer_object.h"

class xRenderGL : public Renderer
{
  public:
    virtual ~xRenderGL() {}

    bool       UseVBO;
    bool       UseList;

    virtual void RenderModel    ( xModel &model, xModelInstance &instance,
                                  bool transparent, const xFieldOfView &FOV );
    virtual void RenderDepth    ( xModel &model, xModelInstance &instance,
                                  bool transparent, const xFieldOfView &FOV );
    virtual void RenderAmbient  ( xModel &model, xModelInstance &instance, const xLightVector &lights,
                                  bool transparent, const xFieldOfView &FOV );
    virtual void RenderDiffuse  ( xModel &model, xModelInstance &instance, const xLight &light,
                                  bool transparent, const xFieldOfView &FOV );
    virtual void RenderSkeleton ( xModel &model, xModelInstance &instance, xWORD selBoneId = xWORD_MAX );
    virtual void RenderVertices ( xModel &model, xModelInstance &instance,
                                  SelectionMode         selectionMode    = smNone,
                                  xWORD                 selElementId     = xWORD_MAX,
                                  std::vector<xDWORD> * selectedVertices = NULL );
    virtual void RenderFaces    ( xModel &model, xModelInstance &instance,
                                  xWORD                 selectedElement,
                                  std::vector<xDWORD>  * facesToRender );

    virtual void RenderSkeletonSelection ( xModel &model, xModelInstance &instance, bool selectConstraint = false );

    virtual void RenderShadowVolume    ( xModel &model, xModelInstance &instance,
                                         xLight &light, xFieldOfView &FOV );
    virtual void RenderShadowMap       ( xModel &model, xModelInstance &instance,
                                         const xShadowMap &shadowMap, const xFieldOfView &FOV );
    virtual void CreateShadowMapTexture( xModel &model, xModelInstance &instance, xDWORD &shadowMapTexId,
                                         xWORD width, xMatrix &mtxBlockerToLight );

    xRenderGL() : UseVBO(GLExtensions::Exists_ARB_VertexBufferObject), UseList(true) {};

    virtual void InvalidateGraphics(xModel &model, xModelInstance &instance)
    {
        if (instance.elementInstanceP)
        {
            xBYTE cnt = instance.elementInstanceC;
            for (xElementInstance *iter = instance.elementInstanceP; cnt; --cnt, ++iter)
            {
                iter->gpuMain.Invalidate();
                iter->mode = xGPURender::NONE;

                xShadowDataVector::iterator iterS = iter->gpuShadows.begin(), iterE = iter->gpuShadows.end();
                for (; iterS != iterE; ++iterS)
                {
                    iterS->gpuLightPointers.Invalidate();
                    iterS->gpuShadowPointers.Invalidate();
                }
            }
        }
        for (xElement *celem = model.kidsP; celem; celem = celem->nextP)
            InvalidateElementGraphics(celem);
    }

    virtual void FreeGraphics(xModel &model, xModelInstance &instance, bool freeShared = true)
    {
        if (UseVBO) FreeVBORenderData(instance.elementInstanceP, instance.elementInstanceC);
        else        FreeListRenderData(instance.elementInstanceP, instance.elementInstanceC);

        if (freeShared)
            for (xElement *celem = model.kidsP; celem; celem = celem->nextP)
                FreeElementGraphics(celem);
    }

    virtual void InvalidateBonePositions(xModelInstance &instance)
    {
        if (!UseVBO) FreeListRenderData(instance.elementInstanceP, instance.elementInstanceC);
    }

    static void InitVBO (xElement *elem);
    static void SetMaterial(xColor color, xMaterial *mat, bool toggleShader = true);

  private:
    void InitTextures(xModel &model);

    void InvalidateElementGraphics(xElement *elem)
    {
        elem->renderData.gpuMain.Invalidate();
        elem->renderData.mode = xGPURender::NONE;
        for (xElement *celem = elem->kidsP; celem; celem = celem->nextP)
            InvalidateElementGraphics(celem);
    }

    void FreeElementGraphics(xElement *elem)
    {
        if (elem->renderData.mode == xGPURender::VBO)
        {
            GLuint p = elem->renderData.gpuMain.vertexB;
            if (p) glDeleteBuffersARB(1, &p);
            p = elem->renderData.gpuMain.indexB;
            if (p) glDeleteBuffersARB(1, &p);
            p = elem->renderData.gpuMain.normalB;
            if (p) glDeleteBuffersARB(1, &p);
            elem->renderData.gpuMain.Invalidate();
            elem->renderData.mode = xGPURender::NONE;
        }
        if (elem->renderData.mode == xGPURender::LIST)
        {
            if (elem->renderData.gpuMain.listID)          glDeleteLists(elem->renderData.gpuMain.listID, 1);
            if (elem->renderData.gpuMain.listIDTex)       glDeleteLists(elem->renderData.gpuMain.listIDTex, 1);
            if (elem->renderData.gpuMain.listIDTransp)    glDeleteLists(elem->renderData.gpuMain.listIDTransp, 1);
            if (elem->renderData.gpuMain.listIDTexTransp) glDeleteLists(elem->renderData.gpuMain.listIDTexTransp, 1);
            elem->renderData.gpuMain.Invalidate();
            elem->renderData.mode = xGPURender::NONE;
        }

        for (xElement *celem = elem->kidsP; celem; celem = celem->nextP)
            FreeElementGraphics(celem);
    }

    void FreeListRenderData(xElementInstance *instanceDataP, xBYTE instanceDataC)
    {
        if (instanceDataP)
        {
            xElementInstance *iter = instanceDataP;
            for (int i = instanceDataC; i; --i, ++iter)
                if (iter->mode == xGPURender::LIST)
                {
                    if (iter->gpuMain.listID)       glDeleteLists(iter->gpuMain.listID, 1);
                    if (iter->gpuMain.listIDTex)    glDeleteLists(iter->gpuMain.listIDTex, 1);
                    if (iter->gpuMain.listIDTransp) glDeleteLists(iter->gpuMain.listIDTransp, 1);
                    if (iter->gpuMain.listIDTexTransp) glDeleteLists(iter->gpuMain.listIDTexTransp, 1);
                    iter->gpuMain.Invalidate();
                    iter->mode = xGPURender::NONE;

                    xShadowDataVector::iterator iterS = iter->gpuShadows.begin(), iterE = iter->gpuShadows.end();
                    for (; iterS != iterE; ++iterS)
                    {
                        if (iterS->gpuLightPointers.listID)       glDeleteLists(iterS->gpuLightPointers.listID, 1);
                        if (iterS->gpuLightPointers.listIDTex)    glDeleteLists(iterS->gpuLightPointers.listIDTex, 1);
                        if (iterS->gpuLightPointers.listIDTransp) glDeleteLists(iterS->gpuLightPointers.listIDTransp, 1);
                        if (iterS->gpuLightPointers.listIDTexTransp) glDeleteLists(iterS->gpuLightPointers.listIDTexTransp, 1);
                        iterS->gpuLightPointers.Invalidate();

                        if (iterS->gpuShadowPointers.listIDPass)  glDeleteLists(iterS->gpuShadowPointers.listIDPass, 1);
                        if (iterS->gpuShadowPointers.listIDFail)  glDeleteLists(iterS->gpuShadowPointers.listIDFail, 1);
                        iterS->gpuShadowPointers.Invalidate();
                    }
                }
        }
    }
    void FreeVBORenderData(xElementInstance *instanceDataP, xBYTE instanceDataC)
    {
        if (instanceDataP)
        {
            xElementInstance *iter = instanceDataP;
            for (int i = instanceDataC; i; --i, ++iter)
                if (iter->mode == xGPURender::VBO)
                {
                    GLuint p = iter->gpuMain.vertexB;
                    if (p) glDeleteBuffersARB(1, &p);
                    p = iter->gpuMain.indexB;
                    if (p) glDeleteBuffersARB(1, &p);
                    p = iter->gpuMain.normalB;
                    if (p) glDeleteBuffersARB(1, &p);
                    iter->gpuMain.Invalidate();
                    iter->mode = xGPURender::NONE;

                    xShadowDataVector::iterator iterS = iter->gpuShadows.begin(), iterE = iter->gpuShadows.end();
                    for (; iterS != iterE; ++iterS)
                    {
                        p = iterS->gpuLightPointers.vertexB;
                        if (p) glDeleteBuffersARB(1, &p);
                        p = iterS->gpuLightPointers.indexB;
                        if (p) glDeleteBuffersARB(1, &p);
                        p = iterS->gpuLightPointers.normalB;
                        if (p) glDeleteBuffersARB(1, &p);
                        iterS->gpuLightPointers.Invalidate();

                        /*
                        p = iterS->gpuShadowPointers.vertexB;
                        if (p) glDeleteBuffersARB(1, &p);
                        p = iterS->gpuShadowPointers.indexB;
                        if (p) glDeleteBuffersARB(1, &p);
                        iterS->gpuShadowPointers.Invalidate();
                        */
                        if (iterS->gpuShadowPointers.listIDPass)  glDeleteLists(iterS->gpuShadowPointers.listIDPass, 1);
                        if (iterS->gpuShadowPointers.listIDFail)  glDeleteLists(iterS->gpuShadowPointers.listIDFail, 1);
                        iterS->gpuShadowPointers.Invalidate();
                    }
                }
        }
    }
};

#endif
