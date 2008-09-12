/*
 * Based on http://nehe.gamedev.net/data/lessons/lesson.asp?lesson=13
 * Based on http://nehe.gamedev.net/data/lessons/lesson.asp?lesson=14
 */

#ifndef __incl_Graphics_OGL_Font_h
#define __incl_Graphics_OGL_Font_h

#include "ogl.h"
#include <string>

#include "../../Utils/Resource.h"
#include "../../Utils/Utils.h"

namespace Graphics { namespace OGL {

    struct Font : public Resource
    {
    private:
        std::string Id;

        static const float INTERLINE;
        static const int   FIRST_CHAR = 0;
        static const int   NUM_CHARS  = 32+96+128;

        int         ID_GLFontBase; // GL list identifier
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
            ID_GLFontBase = -1;
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
        { ID_GLFontBase = -1; }
        virtual bool IsDisposed()
        { return ID_GLFontBase == -1 || !glIsList( ID_GLFontBase ); }

        virtual const std::string &Identifier() { return Id; }


        /////// font usage functions

        void Print   (float x, float y, float z, float maxHeight, int skipLines, const char *text) const;
        void Print   (float x, float y, float z, const char *text) const;
        void PrintF  (float x, float y, float z, const char *fmt, ...) const;
        void Print   (const char *text) const;
        void PrintF  (const char *fmt, ...) const;
        //void Print3d (const char *fmt, ...) const;

        float Length (const char *text) const;
        //GLfloat Length3d (const char *text) const;
    };

} } // namespace Graphics::OGL

#endif

