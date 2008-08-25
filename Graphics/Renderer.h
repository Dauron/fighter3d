#ifndef __incl_Graphics_Render_h
#define __incl_Graphics_Render_h

#include "../Models/lib3dx/xModel.h"
#include "../Math/Cameras/FieldOfView.h"
#include "../Math/xLight.h"
#include "../Math/Figures/xBVHierarchy.h"

struct xShadowMap
{
    xDWORD  texId;
    xMatrix receiverUVMatrix;
};

class Renderer
{
public:
    virtual ~Renderer() {}

    enum SelectionMode { smNone, smElement, smVertex, smModel };

    virtual void RenderModel    ( xModel &model, xModelInstance &instance,
                                  bool transparent,
                                  const Math::Cameras::FieldOfView &FOV ) = 0;
    virtual void RenderDepth    ( xModel &model, xModelInstance &instance,
                                  bool transparent,
                                  const Math::Cameras::FieldOfView &FOV ) = 0;
    virtual void RenderAmbient  ( xModel &model, xModelInstance &instance,
                                  const Vec_xLight &lights, bool transparent,
                                  const Math::Cameras::FieldOfView &FOV ) = 0;
    virtual void RenderDiffuse  ( xModel &model, xModelInstance &instance,
                                  const xLight &light, bool transparent,
                                  const Math::Cameras::FieldOfView &FOV ) = 0;
    virtual void RenderSkeleton ( xModel &model, xModelInstance &instance,
                                  xWORD selBoneId = xWORD_MAX ) = 0;
    virtual void RenderVertices ( xModel &model, xModelInstance &instance,
                                  SelectionMode         selectionMode    = smNone,
                                  xWORD                 selElementId     = xWORD_MAX,
                                  std::vector<xDWORD> * selectedVertices = NULL ) = 0;
    virtual void RenderFaces    ( xModel &model, xModelInstance &instance,
                                  xWORD                 selectedElement,
                                  std::vector<xDWORD>  * facesToRender ) = 0;

    virtual void RenderSkeletonSelection ( xModel &model, xModelInstance &instance,
                                           bool selectConstraint = false ) = 0;

    void RenderBVH              ( Math::Figures::xBVHierarchy &bvh,
                                  const xMatrix &MX_LocalToWorld,
                                  xBYTE I_level = 0, xBYTE ID_selected = xBYTE_MAX,
                                  bool FL_selection = false)
    {
        xBYTE ID = 0;
        RenderBVHExt(bvh, MX_LocalToWorld, I_level, ID, ID_selected, FL_selection);
    }

    virtual void RenderShadowVolume      ( xModel &model, xModelInstance &instance,
                                           xLight &light,
                                           const Math::Cameras::FieldOfView &FOV ) = 0;
    virtual void RenderShadowMap         ( xModel &model, xModelInstance &instance,
                                           const xShadowMap &shadowMap,
                                           const Math::Cameras::FieldOfView &FOV ) = 0;
    virtual void CreateShadowMapTexture  ( xModel &model, xModelInstance &instance,
                                           xDWORD &shadowMapTexId,
                                           xWORD width, xMatrix &mtxBlockerToLight ) = 0;

    virtual void InvalidateGraphics(xModel &model, xModelInstance &instance) = 0;
    virtual void FreeGraphics      (xModel &model, xModelInstance &instance,
                                    bool freeShared = true) = 0;
    virtual void InvalidateBonePositions(xModelInstance &instance)    = 0;

protected:
    virtual void RenderBVHExt            ( Math::Figures::xBVHierarchy &bvh,
                                           const xMatrix &MX_LocalToWorld,
                                           xBYTE          I_level,
                                           xBYTE         &ID,
                                           xBYTE          ID_selected = xBYTE_MAX,
                                           bool           FL_selection = false) = 0;
};

#endif
