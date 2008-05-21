#ifndef __incl_lib3dx_xRenderGL_h
#define __incl_lib3dx_xRenderGL_h

#include "xRender.h"
#include "../../GLExtensions/aGL_Extensions.h"

#include "../../Utils/Debug.h"

class xRenderGL : public xRender
{
  public:
    virtual void RenderModel   ( bool transparent );
    virtual void RenderShadow  ( const xShadowMap &shadowMap, const xMatrix &locationMatrix);
    virtual void RenderSkeleton( bool selectionRendering, xWORD selBoneId = xWORD_MAX);
    virtual void RenderVertices( SelectionMode         selectionMode    = smNone,
                                  xWORD                 selElementId     = xWORD_MAX,
                                  std::vector<xDWORD> * selectedVertices = NULL );
    virtual void RenderFaces   ( xWORD                 selectedElement,
                                 std::vector<xDWORD>  * facesToRender );


    xRenderGL() : UseVBO(agl_VBOLoaded), UseList(true) {};

    virtual void Initialize(bool isStatic, HModel hGrModel, HModel hPhModel = HModel())
    {
        xRender::Initialize(isStatic, hGrModel, hPhModel);
        UseVBO = agl_VBOLoaded && !isStatic;
    }

    virtual void CalculateSkeleton()
    {
        xRender::CalculateSkeleton();
        if (!UseVBO) // refresh lists
        {
            UseList = false;
            g_ModelMgr.GetModel(hModelGraphics)->FreeRenderData(true);
            g_ModelMgr.GetModel(hModelPhysical)->FreeRenderData(true);
        }
    }

    virtual void FreeRenderData()
    {
        if (this->shadowTexId) glDeleteTextures(1, (GLuint*)&this->shadowTexId);
    }

    virtual xDWORD CreateShadowMap(xWORD width, xMatrix &mtxBlockerToLight);

  private:
    bool       UseVBO;
    bool       UseList;
    xMaterial *m_currentMaterial;

    void RenderBone(const xBone * bone, bool selectionRendering, xWORD selBoneId);

    void InitVBO     (xElement *elem);
    void SetMaterial (xColor color, xMaterial *mat);

    void RenderElementVerticesVBO( xElement            * elem,
                                   SelectionMode         selectionMode,
                                   xWORD                 selElementId,
                                   std::vector<xDWORD> * selectedVertices);
    void RenderElementVerticesLST( xElement            * elem,
                                   SelectionMode         selectionMode,
                                   xWORD                 selElementId,
                                   std::vector<xDWORD> * selectedVertices);

    void RenderElementFacesVBO(
                               xElement            * elem,
                               xWORD                 selElementId,
                               std::vector<xDWORD> * facesToRender);
    void RenderElementFacesLST(
                               xElement            * elem,
                               xWORD                 selElementId,
                               std::vector<xDWORD> * facesToRender);

    void RenderModelVBO( xElement * elem, bool transparent );
    void RenderModelLST( xElement * elem, bool transparent );

    void RenderShadowLST(xElement *elem);
    void RenderShadowVBO(xElement *elem);
};

#endif
