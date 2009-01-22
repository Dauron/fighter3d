#ifndef __incl_Graphics_TexResource_h
#define __incl_Graphics_TexResource_h

#include <string>
#include "../../AppFramework/Application.h"

#include "../OGL/Texture.h"
#ifdef WIN32
#   include "../D3D/Texture.h"
#endif


namespace Graphics {

    struct TexResource : public Resource
    {
    public:
        Graphics::Texture *texture;

        TexResource() { Clear(); }

        virtual void Clear() {
            Resource::Clear();
            texture = NULL;
        }

        bool Create( const std::string& name, bool fl_mipmap )
        {
            if (g_Application.IsOpenGL())
                texture = new Graphics::OGL::Texture();
            else
#ifdef USE_D3D
                texture = new Graphics::D3D::Texture();
#else
            {
                texture = NULL;
                return false;
            }
#endif
            return texture->Create(name, fl_mipmap);
        }
        bool Create( const std::string& name, Image image )
        {
            if (g_Application.IsOpenGL())
                texture = new Graphics::OGL::Texture();
            else
#ifdef USE_D3D
                texture = new Graphics::D3D::Texture();
#else
            {
                texture = NULL;
                return false;
            }
#endif
            return texture->Create(name, image);
        }

        virtual bool Create()
        { return texture->Create(); }
        virtual void Dispose()
        { texture->Dispose(); }
        virtual void Destroy() { Dispose(); if (texture) delete texture; Clear(); }
        virtual void Invalidate()
        { texture->Invalidate(); }
        virtual bool IsDisposed()
        { return texture->IsDisposed(); }
        virtual void Bind()
        { texture->Bind(); }

        virtual const std::string &Identifier() { return texture->Identifier(); }
    };

} // namespace Graphics

#endif
