#ifndef __incl_Graphics_ShadowVolume_h
#define __incl_Graphics_ShadowVolume_h

#include "../Models/lib3dx/xModel.h"
#include "../Math/xLight.h"

namespace ShadowVolume
{
  void ExtrudePoints (const xElement *elem, bool infiniteL, const xVector3 &lightPos,
                      xShadowData &shadowData);
  void GetBackFaces  (const xElement *elem, const xElementInstance &instance, bool infiniteL,
                      const xShadowData &shadowData, bool *&backFaces);
  void GetSilhouette (const xElement *elem, bool infiniteL, bool optimizeBackCap,
                      const bool *facingFlag, xShadowData &shadowData);
  bool ViewportMaybeShadowed (const xElement *elem, xElementInstance &instance,
                              const xMatrix &location,
                              const Math::Cameras::FieldOfView &FOV, const xLight& light);
}

#endif
