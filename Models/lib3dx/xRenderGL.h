#ifndef __incl_lib3dx_xRenderGL_h
#define __incl_lib3dx_xRenderGL_h

#include "xRender.h"
#include "../../GLExtensions/ARB_vertex_buffer_object.h"
#include "../../Utils/Debug.h"

class xRenderGL : public xRender
{
  public:
    virtual void RenderModel    ( xModel &model, xModelInstance &instance,
                                  bool transparent, const xFieldOfView &FOV );
    virtual void RenderSkeleton ( xModel &model, xModelInstance &instance,
                                  bool selectionRendering, xWORD selBoneId = xWORD_MAX );
    virtual void RenderVertices ( xModel &model, xModelInstance &instance,
                                  SelectionMode         selectionMode    = smNone,
                                  xWORD                 selElementId     = xWORD_MAX,
                                  std::vector<xDWORD> * selectedVertices = NULL );
    virtual void RenderFaces    ( xModel &model, xModelInstance &instance,
                                  xWORD                 selectedElement,
                                  std::vector<xDWORD>  * facesToRender );

    virtual void RenderShadowVolume    ( xModel &model, xModelInstance &instance,
                                         xLight &light, xFieldOfView &FOV );
    virtual void RenderShadowMap       ( xModel &model, xModelInstance &instance,
                                         const xShadowMap &shadowMap, const xFieldOfView &FOV );
    virtual void CreateShadowMapTexture( xModel &model, xModelInstance &instance, xDWORD &shadowMapTexId,
                                         xWORD width, xMatrix &mtxBlockerToLight );

    xRenderGL() : UseVBO(GLExtensions::Exists_ARB_VertexBufferObject), UseList(true) {};

    virtual void InvalidateInstanceGraphics(xModelInstance &instance)
    {
        if (instance.elementInstanceP)
        {
            xBYTE cnt = instance.elementInstanceC;
            for (xElementInstance *iter = instance.elementInstanceP; cnt; --cnt, ++iter)
            {
                iter->gpuMain.vertexB = 0;
                iter->gpuMain.normalB = 0;
                iter->gpuMain.indexB = 0;
                iter->mode = xElementInstance::xRenderMode_NULL;
            }
        }
    }

    virtual void FreeInstanceGraphics(xModelInstance &instance)
    {
        if (UseVBO) FreeVBORenderData(instance.elementInstanceP, instance.elementInstanceC);
        else        FreeListRenderData(instance.elementInstanceP, instance.elementInstanceC);
    }

    virtual void InvalidateBonePositions(xModelInstance &instance)
    {
        if (!UseVBO) FreeListRenderData(instance.elementInstanceP, instance.elementInstanceC);
    }

    static void InitVBO (const xElement *elem, xElementInstance &instance);

  private:
    bool       UseVBO;
    bool       UseList;

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
};

#endif
