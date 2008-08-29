/*
 * Based on http://nehe.gamedev.net/data/lessons/lesson.asp?lesson=13
 * Based on http://nehe.gamedev.net/data/lessons/lesson.asp?lesson=14
 */

#ifndef __incl_GLFont_h
#define __incl_GLFont_h

#include "../../../AppFramework/System.h" // Header File For System
#include <GL/gl.h>                      // Header File For The OpenGL32 Library
#include <string>

#include "../../../Utils/HandleDst.h"

struct GLFont : public HandleDst
{
    static const float INTERLINE;
    static const int FIRST_CHAR = 0;
    static const int NUM_CHARS  = 32+96+128;

    std::string m_Name;          // for reconstruction
    int         m_Size;          // for reconstruction
    int         m_GLFontBase;    // GL list identifier

    float       LineH() const
    {
        return m_Size * (1+INTERLINE);
    }

    GLFont()
    {
        Invalidate();
    }

    bool Load(const std::string& name, int size = 14);
    void Unload();

    bool ReLoad  ()
    {
        std::string name = m_Name;
        Unload();
        return Load (name, m_Size);
    }

    void Invalidate()
    {
        m_GLFontBase   = -1;
    }
    bool IsValid() const
        { return m_GLFontBase != -1 && glIsList( m_GLFontBase ); }

    void Print   (float x, float y, float z, float maxHeight, int skipLines, const char *text) const;
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

