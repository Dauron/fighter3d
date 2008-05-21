#ifndef __incl_lib3dx_xRender_h
#define __incl_lib3dx_xRender_h

#include "../ModelMgr.h"
#include "xBone.h"
#include "../../Math/xFieldOfView.h"
#include "../../Math/xLight.h"
#include "xUtils.h"

struct xShadowMap
{
    xDWORD  texId;
    xMatrix receiverUVMatrix;
};

class xRender
{
public:
    enum SelectionMode { smNone, smElement, smVertex, smModel };

    virtual void RenderModel    ( xModel &model, xModelInstance &instance,
                                  bool transparent, const xFieldOfView &FOV ) = 0;
    virtual void RenderSkeleton ( xModel &model, xModelInstance &instance,
                                  bool selectionRendering, xWORD selBoneId = xWORD_MAX ) = 0;
    virtual void RenderVertices ( xModel &model, xModelInstance &instance,
                                  SelectionMode         selectionMode    = smNone,
                                  xWORD                 selElementId     = xWORD_MAX,
                                  std::vector<xDWORD> * selectedVertices = NULL ) = 0;
    virtual void RenderFaces    ( xModel &model, xModelInstance &instance,
                                  xWORD                 selectedElement,
                                  std::vector<xDWORD>  * facesToRender ) = 0;

    virtual void RenderShadowVolume    ( xModel &model, xModelInstance &instance,
                                         xLight &light, xFieldOfView &FOV ) = 0;
    virtual void RenderShadowMap       ( xModel &model, xModelInstance &instance,
                                         const xShadowMap &shadowMap, const xFieldOfView &FOV ) = 0;
    virtual void CreateShadowMapTexture( xModel &model, xModelInstance &instance, xDWORD &shadowMapTexId,
                                         xWORD width, xMatrix &mtxBlockerToLight ) = 0;

    virtual void InvalidateInstanceGraphics(xModelInstance &instance) = 0;
    virtual void FreeInstanceGraphics(xModelInstance &instance)       = 0;
    virtual void InvalidateBonePositions(xModelInstance &instance)    = 0;
};

#endif
