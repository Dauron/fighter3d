#ifndef __incl_lib3dx_xRenderGL_h
#define __incl_lib3dx_xRenderGL_h

#include "xRender.h"
#include "../../GLExtensions/aGL_Extensions.h"

class xRenderGL : public xRender
{
  public:
    virtual void RenderModel   ();
    virtual void RenderSkeleton( bool selectionRendering, xWORD selBoneId );
    virtual void RenderVertices( SelectionMode         selectionMode    = smNone,
                                 xWORD                 selElementId     = -1,
                                 std::vector<xDWORD> * selectedVertices = NULL );
    virtual void RenderFaces   ( xWORD                 selectedElement,
                                 std::vector<xDWORD> * facesToRender );


    xRenderGL() : UseVBO(agl_VBOLoaded), UseList(true) {};

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

    void RenderModelVBO( xElement * elem );
    void RenderModelLST( xElement * elem );
};

#endif
