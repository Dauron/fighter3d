/*
 * Based on http://nehe.gamedev.net/data/lessons/lesson.asp?lesson=13
 * Based on http://nehe.gamedev.net/data/lessons/lesson.asp?lesson=14
 */

#ifndef __incl_GLFont_h
#define __incl_GLFont_h

#include "../../../AppFramework/System.h" // Header File For System
#include <GL/gl.h>                      // Header File For The OpenGL32 Library
#include <string>

#include "../../../Utils/Resource.h"
#include "../../../Utils/Utils.h"

struct GLFont : public Resource
{
private:
    std::string Id;

public:
    static const float INTERLINE;
    static const int   FIRST_CHAR = 0;
    static const int   NUM_CHARS  = 32+96+128;

    std::string Name;          // for reconstruction
    int         Size;          // for reconstruction
    int         ID_GLFontBase; // GL list identifier

    float       LineH() const
    { return Size * (1+INTERLINE); }

    GLFont() { Clear(); }

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

    float LWidth[NUM_CHARS];

private:
    void Init();
};

#endif

