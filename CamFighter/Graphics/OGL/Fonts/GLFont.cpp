#include "GLFont.h"
#include "../../../App Framework/Application.h"
#include <cstdio>

#ifdef WIN32
#pragma warning(disable : 4996) // deprecated
#else
#include <stdarg.h>
#include "../../../Utils/Utils.h"
#endif

const float GLFont::INTERLINE = 0.2f;

bool GLFont::Load(const std::string& name, int size)
{
    assert (m_GLFontBase == -1);
    
    m_Name = name;
    m_Size = size;
    
    Init();

    return true;
}

void GLFont::Unload()
{
    if (m_GLFontBase != -1)
    {
        m_Name.erase();
        glDeleteLists(m_GLFontBase, NUM_CHARS);   // Delete All Characters
        m_GLFontBase = -1;
    }
}

void GLFont::Init()
{
    assert(m_GLFontBase == -1);

    HDC hDC = g_Application.MainWindow().HDC();
    if (!hDC) return;
    
    m_GLFontBase = glGenLists(NUM_CHARS);           // Storage For 96 Characters
     
#ifdef WIN32
    HFONT    font;                                  // Windows Font ID
    HFONT    oldfont;                               // Used For Good House Keeping

    font = CreateFont( -m_Size,                     // Height Of Font
                        0,                          // Width Of Font
                        0,                          // Angle Of Escapement
                        0,                          // Orientation Angle
                        FW_NORMAL,                  // Font Weight
                        FALSE,                      // Italic
                        FALSE,                      // Underline
                        FALSE,                      // Strikeout
                        ANSI_CHARSET,               // Character Set Identifier
                        OUT_TT_PRECIS,              // Output Precision
                        CLIP_DEFAULT_PRECIS,        // Clipping Precision
                        ANTIALIASED_QUALITY,        // Output Quality
                        FF_DONTCARE|DEFAULT_PITCH,  // Family And Pitch
                        m_Name.c_str());             // Font Name

    oldfont = (HFONT)SelectObject(hDC, font);       // Selects The Font We Want

    // BUG: call it 2 times, cause sometimes 1 time is not enought
    //wglUseFontBitmaps(hDC, FIRST_CHAR, NUM_CHARS, m_GLFontBase); // Builds NUM_CHARS Characters Starting At Character FIRST_CHAR
    wglUseFontBitmaps(hDC, FIRST_CHAR, NUM_CHARS, m_GLFontBase); // Builds NUM_CHARS Characters Starting At Character FIRST_CHAR

    ABCFLOAT metrics[NUM_CHARS]; // Storage For Information About Our Font
    GetCharABCWidthsFloat(hDC, FIRST_CHAR, FIRST_CHAR+NUM_CHARS-1, metrics);
    for (int i = FIRST_CHAR; i != NUM_CHARS; ++i)
        LWidth[i] = metrics[i].abcfA + metrics[i].abcfB + metrics[i].abcfC;
        
/*
    GLYPHMETRICSFLOAT gmf[NUM_CHARS]; // Storage For Information About Our Font
    wglUseFontOutlines(hDC,                         // Select The Current DC
            FIRST_CHAR,                             // Starting Character
            NUM_CHARS,                              // Number Of Display Lists To Build
            m_GLFontBase3d,                         // Starting Display Lists
            0.0f,                                   // Deviation From The True Outlines
            0.2f,                                   // Font Thickness In The Z Direction
            WGL_FONT_POLYGONS,                      // Use Polygons, Not Lines
            gmf);                                   // Address Of Buffer To Recieve Data
*/
    SelectObject(hDC, oldfont);                     // Selects The Font We Want
    DeleteObject(font);                             // Delete The Font
#else
    /* load a font with a specific name in "Host Portable Character Encoding" */
/*
 *  We construct an X font-name by using the name, style and size fields.
 *  An X font name has the following structure:
 *
 *  -fndry-family-weight-slant-swdth-adstyl-pixelsize-pointsize-
 *  resx-resy-spc-avgWdth-rgstry-encdng
 *
 *  We leave as wildcards those fields we don't wish to specify.
 *  The ones which are important to use are:
 *
 *  family = { courier, fixed, helvetica, lucida, new century schoolbook,
 *      screen, times }
 *  weight = { *, bold, medium }
 *  slant  = { r, i } roman or italic
 *  swdth  = { normal, condensed, double wide, narrow, semicondensed, wide }
 *  adstyl = { *, sans, serif }
*/
    std::string sfont = "-*-helvetica-*-r-normal--" + itos(m_Size) + "-*-*-*-*-*-*-*";
    XFontStruct *font = XLoadQueryFont(hDC, sfont.c_str());
    if (font == NULL)
    {
        /* this really *should* be available on every X Window System...*/
        font = XLoadQueryFont(hDC, "fixed");
        if (font == NULL)
            printf("Problems loading fonts :-(\n");
    }
    /* build 96 display lists out of our font starting at char 32 */
    glXUseXFont(font->fid, FIRST_CHAR, NUM_CHARS, m_GLFontBase);

    char s = FIRST_CHAR;
    for (int i = 0; i < NUM_CHARS; ++i, ++s)
        LWidth[i] = XTextWidth(font, &s, 1);

    /* free our XFontStruct since we have our display lists */
    XFreeFont(hDC, font);
#endif

    glNewList(m_GLFontBase+'\t',GL_COMPILE);        // Tab is 8 spaces
        glCallLists(8, GL_UNSIGNED_BYTE, "        "); // Draws The Display List Text
    glEndList();
}

void GLFont::Print (float x, float y, float z, float maxHeight, int skipLines, const char *text) const
{
    assert(m_GLFontBase != -1);
    //if (m_GLFontBase == -1) Init();
    glRasterPos3f(x, y, z);            // Position The Text On The Screen

    if (text == NULL)                  // If There's No Text
        return;                        // Do Nothing

    const char *start = text;
    size_t len = strlen(text);
    const char *end;

    float lineH = LineH();

    glPushAttrib(GL_LIST_BIT);         // Pushes The Display List Bits
    glListBase(m_GLFontBase - FIRST_CHAR); // Sets The Base Character to FIRST_CHAR
    while ( (end = strchr(start, '\n')) )
    {
        if (skipLines)
            --skipLines;
        else
        {
            if (maxHeight <= 0) return;

            glCallLists((GLsizei)(end-start), GL_UNSIGNED_BYTE, start); // Draws The Display List Text
            y -= lineH;
            glRasterPos3f(x, y, z);        // Position The Text On The Screen
            maxHeight -= lineH;
        }
        start = end+1;
    }
    end = text + len;
    glCallLists((GLsizei)(end-start), GL_UNSIGNED_BYTE, start);     // Draws The Display List Text
    glPopAttrib();                     // Pops The Display List Bits
}

void GLFont::PrintF (float x, float y, float z, const char *fmt, ...) const
{
    assert(m_GLFontBase != -1);
    //if (m_GLFontBase == -1) Init();
    glRasterPos3f(x, y, z);            // Position The Text On The Screen

    char    text[256];                 // Holds Our String
    va_list ap;                        // Pointer To List Of Arguments
    float lineH = LineH();

    if (fmt == NULL)                   // If There's No Text
        return;                        // Do Nothing

    va_start(ap, fmt);                 // Parses The String For Variables
    vsprintf(text, fmt, ap);           // And Converts Symbols To Actual Numbers
    va_end(ap);                        // Results Are Stored In Text

    char *start = text;
    size_t len = strlen(text);
    char *end;

    glPushAttrib(GL_LIST_BIT);         // Pushes The Display List Bits
    glListBase(m_GLFontBase - FIRST_CHAR); // Sets The Base Character to FIRST_CHAR
    while ( (end = strchr(start, '\n')) )
    {
        glCallLists((GLsizei)(end-start), GL_UNSIGNED_BYTE, start); // Draws The Display List Text
        start = end+1;
        y -= lineH;
        glRasterPos3f(x, y, z);        // Position The Text On The Screen
    }
    end = text + len;
    glCallLists((GLsizei)(end-start), GL_UNSIGNED_BYTE, start);     // Draws The Display List Text
    glPopAttrib();                     // Pops The Display List Bits
}

void GLFont::Print (const char *text) const
{
    assert(m_GLFontBase != -1);
    //if (m_GLFontBase == -1) Init();

    if (text == NULL)                   // If There's No Text
        return;                        // Do Nothing

    glPushAttrib(GL_LIST_BIT);         // Pushes The Display List Bits
    glListBase(m_GLFontBase - FIRST_CHAR); // Sets The Base Character to FIRST_CHAR
    glCallLists((GLsizei)strlen(text), GL_UNSIGNED_BYTE, text);    // Draws The Display List Text
    glPopAttrib();                     // Pops The Display List Bits
}

void GLFont::PrintF (const char *fmt, ...) const
{
    assert(m_GLFontBase != -1);
    //if (m_GLFontBase == -1) Init();
    char        text[256];             // Holds Our String
    va_list        ap;                 // Pointer To List Of Arguments

    if (fmt == NULL)                   // If There's No Text
        return;                        // Do Nothing

    va_start(ap, fmt);                 // Parses The String For Variables
    vsprintf(text, fmt, ap);           // And Converts Symbols To Actual Numbers
    va_end(ap);                        // Results Are Stored In Text

    glPushAttrib(GL_LIST_BIT);         // Pushes The Display List Bits
    glListBase(m_GLFontBase - FIRST_CHAR); // Sets The Base Character to FIRST_CHAR
    glCallLists((GLsizei)strlen(text), GL_UNSIGNED_BYTE, text);    // Draws The Display List Text
    glPopAttrib();                     // Pops The Display List Bits
}

float GLFont::Length (const char *text) const
{
    assert(m_GLFontBase != -1);
    //if (m_GLFontBase == -1) Init();
    float length = 0.0f;

    for (; *text; ++text)    // Loop To Find Text Length
#if FIRST_CHAR
        if ((unsigned char)*text >= FIRST_CHAR && (unsigned char)*text < FIRST_CHAR+NUM_CHARS)
#elseif FIRST_CHAR+NUM_CHARS < 255
        if ((unsigned char)*text < FIRST_CHAR+NUM_CHARS)
#endif
            length += LWidth[(unsigned char)*text-FIRST_CHAR]; // Increase Length By Each Characters Width
    return length;
}

#ifdef WIN32
/*
void GLFont::Print3d (const char *fmt, ...) const
{
    assert(m_GLFontBase3d != -1);
    //if (m_GLFontBase3d == -1) Init();
    char        text[256];             // Holds Our String
    va_list        ap;                 // Pointer To List Of Arguments

    if (fmt == NULL)                   // If There's No Text
        return;                        // Do Nothing

    va_start(ap, fmt);                 // Parses The String For Variables
    vsprintf(text, fmt, ap);           // And Converts Symbols To Actual Numbers
    va_end(ap);                        // Results Are Stored In Text

    GLfloat length = Length3d(text);

    glTranslatef(-length/2,0.0f,0.0f);            // Center Our Text On The Screen

    size_t len = strlen(text);
    
    int ffBit;
    glGetIntegerv(GL_FRONT_FACE, &ffBit);

    glPushAttrib(GL_LIST_BIT);                    // Pushes The Display List Bits
    glListBase(m_GLFontBase3d - FIRST_CHAR);      // Sets The Base Character to FIRST_CHAR
    glCallLists((GLsizei)(len), GL_UNSIGNED_BYTE, text);     // Draws The Display List Text
    glPopAttrib();                                // Pops The Display List Bits

    glFrontFace(ffBit);

    glTranslatef(length/2,0.0f,0.0f);             // Center Our Text On The Screen
}

GLfloat GLFont::Length3d (const char *text) const
{
    assert(m_GLFontBase3d != -1);
    //if (m_GLFontBase3d == -1) Init();
    float length = 0.0f;
    size_t strl = strlen(text);

    for (; *text; ++text)    // Loop To Find Text Length
        if (*text >= FIRST_CHAR && *text < FIRST_CHAR+NUM_CHARS)
            length += gmf[*text-FIRST_CHAR].gmfCellIncX; // Increase Length By Each Characters Width
    return length;
}
*/
#endif

