#ifndef __incl_Graphics_OGL_Texture_h
#define __incl_Graphics_OGL_Texture_h

#include "ogl.h"
#include "../Textures/Texture.h"

namespace Graphics { namespace OGL {

    struct Texture : public Graphics::Texture
    {
        unsigned int ID_GLTexture; // GL Texture identifier

    public:

        Texture() { Clear(); }

        virtual void Clear() {
            Graphics::Texture::Clear();
            ID_GLTexture = 0;
        }

        virtual bool Create();
        virtual void Dispose();

        virtual void Invalidate()
        { ID_GLTexture = 0; }
        virtual bool IsDisposed()
        { return ID_GLTexture == 0; }

        virtual void Bind()
        {
            glBindTexture(GL_TEXTURE_2D, ID_GLTexture );
        }
    };

} } // namespace Graphics::OGL

#endif
