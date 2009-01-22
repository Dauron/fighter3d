#ifndef __incl_Graphics_D3D_d3d_h
#define __incl_Graphics_D3D_d3d_h

#ifdef USE_D3D

#include <d3d9.h>
#include <d3dx9.h>
// include the Direct3D Library file
#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")

namespace Graphics { namespace D3D {

    extern LPDIRECT3DDEVICE9 d3ddev; // current d3d device

}}

#endif

#endif
