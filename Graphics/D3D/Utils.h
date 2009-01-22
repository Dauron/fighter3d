#ifndef __incl_Graphics_D3D_Utils_h
#define __incl_Graphics_D3D_Utils_h

#ifdef USE_D3D

#include "d3d.h"
#include "../../Math/Cameras/Camera.h"
#include "../../Math/Cameras/FieldOfView.h"

inline void ViewportSet_DX(LPDIRECT3DDEVICE9 d3ddev, const Math::Cameras::Camera &camera)
{
    D3DVIEWPORT9 viewport = { camera.FOV.ViewportLeft,  camera.FOV.ViewportTop,
                              camera.FOV.ViewportWidth, camera.FOV.ViewportHeight,
                              0.f, 1.f };
    d3ddev->SetViewport (&viewport);
    d3ddev->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*) &camera.FOV.MX_Projection_Get());
    d3ddev->SetTransform(D3DTS_VIEW,       (D3DMATRIX*) &camera.MX_WorldToView_Get());
}

#include "../../Math/xLight.h"

void LightSet_DX(xLight &light, bool t_Ambient, bool t_Diffuse, bool t_Specular, xBYTE light_id = 0);

#endif

#endif
