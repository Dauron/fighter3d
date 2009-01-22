#ifndef __incl_Graphics_D3D_Font_h
#define __incl_Graphics_D3D_Font_h

#ifdef USE_D3D

#include "d3d.h"
#include <string>

#include "../../Utils/Resource.h"
#include "../../Utils/Utils.h"

namespace Graphics { namespace D3D {

    struct Font : public Resource
    {
        static const float INTERLINE;
        static const int   NUM_CHARS  = 32+96+128;

    private:
        std::string Id;
        LPD3DXFONT  dxFont;

        float LWidth[NUM_CHARS];

    public:
        std::string Name;          // for reconstruction
        int         Size;          // for reconstruction

        float       LineH() const
        { return Size * (1+INTERLINE); }

        Font() { Clear(); }

        virtual void Clear() {
            Resource::Clear();
            Id.clear();
            Name.clear();
            dxFont = 0;
        }

        virtual bool Create();
        virtual bool Create( const std::string& name, int size = 14)
        {
            Name = name;
            Size = size;
            Id   = Name + "|" + itos(Size);
            return Create();
        }

        virtual void Dispose();
        virtual void Invalidate()
        { dxFont = 0; }
        virtual bool IsDisposed()
        { return !dxFont; }

        virtual const std::string &Identifier() { return Id; }


        /////// font usage functions

        void Print   (float x, float y, D3DCOLOR color, float maxHeight, int skipLines, const char *text) const;
        void Print   (float x, float y, D3DCOLOR color, const char *text) const;
        void PrintF  (float x, float y, D3DCOLOR color, const char *fmt, ...) const;
        //void Print3d (const char *fmt, ...) const;

        float Length (const char *text) const;
        //GLfloat Length3d (const char *text) const;
    };

} } // namespace Graphics::D3D

#endif

#endif
