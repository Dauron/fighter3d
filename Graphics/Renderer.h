#ifndef __incl_Graphics_Render_h
#define __incl_Graphics_Render_h

#include "../Models/lib3dx/xModel.h"
#include "../Math/xFieldOfView.h"
#include "../Math/xLight.h"

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
                                  bool transparent, const xFieldOfView &FOV ) = 0;
    virtual void RenderDepth    ( xModel &model, xModelInstance &instance,
                                  bool transparent, const xFieldOfView &FOV ) = 0;
    virtual void RenderAmbient  ( xModel &model, xModelInstance &instance, const xLightVector &lights,
                                  bool transparent, const xFieldOfView &FOV ) = 0;
    virtual void RenderDiffuse  ( xModel &model, xModelInstance &instance, const xLight &light,
                                  bool transparent, const xFieldOfView &FOV ) = 0;
    virtual void RenderSkeleton ( xModel &model, xModelInstance &instance, xWORD selBoneId = xWORD_MAX ) = 0;
    virtual void RenderVertices ( xModel &model, xModelInstance &instance,
                                  SelectionMode         selectionMode    = smNone,
                                  xWORD                 selElementId     = xWORD_MAX,
                                  std::vector<xDWORD> * selectedVertices = NULL ) = 0;
    virtual void RenderFaces    ( xModel &model, xModelInstance &instance,
                                  xWORD                 selectedElement,
                                  std::vector<xDWORD>  * facesToRender ) = 0;

    virtual void RenderSkeletonSelection ( xModel &model, xModelInstance &instance, bool selectConstraint = false ) = 0;

    virtual void RenderShadowVolume    ( xModel &model, xModelInstance &instance,
                                         xLight &light, xFieldOfView &FOV ) = 0;
    virtual void RenderShadowMap       ( xModel &model, xModelInstance &instance,
                                         const xShadowMap &shadowMap, const xFieldOfView &FOV ) = 0;
    virtual void CreateShadowMapTexture( xModel &model, xModelInstance &instance, xDWORD &shadowMapTexId,
                                         xWORD width, xMatrix &mtxBlockerToLight ) = 0;

    virtual void InvalidateGraphics(xModel &model, xModelInstance &instance) = 0;
    virtual void FreeGraphics      (xModel &model, xModelInstance &instance, bool freeShared = true) = 0;
    virtual void InvalidateBonePositions(xModelInstance &instance)    = 0;
};

#endif
