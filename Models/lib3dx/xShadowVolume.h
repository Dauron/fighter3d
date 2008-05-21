#ifndef __incl_lib3dx_xUtils_h
#define __incl_lib3dx_xUtils_h

#include "xModel.h"
#include "../../Math/xLight.h"

void xShadows_ExtrudePoints (const xElement *elem, bool infiniteL, const xVector3 &lightPos, xVector4 *&verticesP);
void xShadows_GetBackFaces (const xElement *elem, const xVector4 *extrVerticesP, bool *&backFaces);
void xShadows_GetSilhouette(const xElement *elem, const bool *facingFlag, xWORD4 *&sideQadsP, xWORD3 *&backCapP, xWORD &edgesC);
bool xShadows_ViewportMaybeShadowed (const xElement *elem, xElementInstance *instance,
                                     const xMatrix &location, const xFieldOfView *FOV, const xLight& light);

#endif
