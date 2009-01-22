#ifndef __incl_Graphics_D3D_Texture_h
#define __incl_Graphics_D3D_Texture_h

#ifdef USE_D3D

#include "d3d.h"
#include "../Textures/Texture.h"

namespace Graphics { namespace D3D {

    struct Texture : public Graphics::Texture
    {
        LPDIRECT3DTEXTURE9 ID_DXTexture; // DX Texture identifier

    public:
        Texture() { Clear(); }

        virtual void Clear() {
            Graphics::Texture::Clear();
            ID_DXTexture = 0;
        }

        virtual bool Create();
        virtual void Dispose();

        virtual void Invalidate()
        { ID_DXTexture = 0; }
        virtual bool IsDisposed()
        { return ID_DXTexture == 0; }

        virtual void Bind()
        {
            Graphics::D3D::d3ddev->SetTexture(0, ID_DXTexture);
        }
    };

} } // namespace Graphics::D3D

#endif

#endif
