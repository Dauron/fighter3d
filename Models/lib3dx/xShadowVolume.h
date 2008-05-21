#ifndef __incl_lib3dx_xUtils_h
#define __incl_lib3dx_xUtils_h

#include "xModel.h"
#include "../../Math/xLight.h"

void xShadows_ExtrudePoints (const xElement *elem, bool infiniteL, const xVector3 &lightPos,
                             xShadowData &shadowData);
void xShadows_GetBackFaces (const xElement *elem, const xElementInstance &instance, bool infiniteL,
                            const xShadowData &shadowData, bool *&backFaces);
void xShadows_GetSilhouette(const xElement *elem, bool infiniteL, bool optimizeBackCap,
                            const bool *facingFlag, xShadowData &shadowData);
bool xShadows_ViewportMaybeShadowed (const xElement *elem, xElementInstance &instance,
                                     const xMatrix &location, const xFieldOfView &FOV, const xLight& light);

#endif
