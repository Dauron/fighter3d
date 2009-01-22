#include "Font.h"
#include "../../AppFramework/Application.h"

#ifdef USE_D3D

using namespace Graphics::D3D;

const float Graphics::D3D::Font::INTERLINE = 0.2f;

bool Graphics::D3D::Font :: Create()
{
    assert (!dxFont);
    assert (Name.size());

    HDC hDC = g_Application.MainWindow_Get().HDC();
    if (!hDC) return false;

    D3DXCreateFont(d3ddev,                      // the D3D Device
                   -Size,                       // font height of 30
                   0,                           // default font width
                   FW_NORMAL,                   // font weight
                   1,                           // not using MipLevels
                   false,                       // italic font
                   DEFAULT_CHARSET,             // default character set
                   OUT_DEFAULT_PRECIS,          // default OutputPrecision,
                   DEFAULT_QUALITY,             // default quality
                   DEFAULT_PITCH | FF_DONTCARE, // default pitch and family
                   Name.c_str(),                // facename
                   &dxFont);                    // the font object


    // get metrics
    HFONT    font;                                  // Windows Font ID
    HFONT    oldfont;                               // Used For Good House Keeping
    font = CreateFont( -Size,                       // Height Of Font
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
                        Name.c_str());              // Font Name
    oldfont = (HFONT)SelectObject(hDC, font);       // Selects The Font We Want

    ABCFLOAT metrics[NUM_CHARS]; // Storage For Information About Our Font
    GetCharABCWidthsFloat(hDC, 0, NUM_CHARS-1, metrics);
    for (int i = 0; i != NUM_CHARS; ++i)
        LWidth[i] = metrics[i].abcfA + metrics[i].abcfB + metrics[i].abcfC;

    SelectObject(hDC, oldfont);                     // Selects The Font We Want
    DeleteObject(font);                             // Delete The Font

    return true;
}

void Graphics::D3D::Font :: Dispose()
{
    if (dxFont)
    {
        dxFont->Release();
        dxFont = NULL;
    }
}

void  Graphics::D3D::Font :: Print  (float x, float y, D3DCOLOR color, float maxHeight, int skipLines, const char *text) const
{
    assert(dxFont);
    assert(text);

    RECT textbox; SetRect(&textbox, (int)x, (int)y, (int)x, (int)y);
    size_t len = strlen(text);
    const char *start = text, *end;
    float lineH = LineH();

    while ( (end = strchr(start, '\n')) )
    {
        if (skipLines)
            --skipLines;
        else
        {
            if (maxHeight <= 0)
                return;
            dxFont->DrawTextA(NULL,
                              start,
                              (size_t)(end-start),
                              &textbox,
                              DT_LEFT | DT_TOP,
                              color);
            textbox.top = (textbox.bottom -= (long)lineH);
            maxHeight -= lineH;

        }
        start = end+1;
    }
    end = text + len;
    dxFont->DrawTextA(NULL,
                      start,
                      (size_t)(end-start),
                      &textbox,
                      DT_LEFT | DT_TOP,
                      color);
}

void  Graphics::D3D::Font :: Print  (float x, float y, D3DCOLOR color, const char *text) const
{
    assert(dxFont);
    assert(text);

    RECT textbox; SetRect(&textbox, (int)x, (int)y, (int)x, (int)y);
    size_t len = strlen(text);
    dxFont->DrawTextA(NULL,
                      text,
                      len,
                      &textbox,
                      DT_LEFT | DT_TOP,
                      color);

    /*
    const char *start = text, *end;
    float lineH = LineH();

    while ( (end = strchr(start, '\n')) )
    {
        if (maxHeight <= 0)
            return;
        dxFont->DrawTextA(NULL,
                          start,
                          (size_t)(end-start),
                          &textbox,
                          DT_LEFT | DT_TOP,
                          color);
        textbox.top = (textbox.bottom -= lineH);
        maxHeight -= lineH;
        start = end+1;
    }
    end = text + len;
    dxFont->DrawTextA(NULL,
                      start,
                      (size_t)(end-start),
                      &textbox,
                      DT_LEFT | DT_TOP,
                      color);
    */
}

void  Graphics::D3D::Font :: PrintF (float x, float y, D3DCOLOR color, const char *fmt, ...) const
{
    assert(dxFont);
    assert(fmt);

    char    text[256];                 // Holds Our String
    va_list ap;                        // Pointer To List Of Arguments
    va_start(ap, fmt);                 // Parses The String For Variables
    vsprintf(text, fmt, ap);           // And Converts Symbols To Actual Numbers
    va_end(ap);                        // Results Are Stored In Text

    RECT textbox; SetRect(&textbox, (int)x, (int)y, (int)x, (int)y);
    size_t len = strlen(text);
    dxFont->DrawTextA(NULL,
                      text,
                      len,
                      &textbox,
                      DT_LEFT | DT_TOP,
                      color);
}

float Graphics::D3D::Font :: Length (const char *text) const
{
    float length = 0.f;
    float maxLen = 0.f;

    for (; *text; ++text)    // Loop To Find Text Length
#if FIRST_CHAR
        if ((byte)*text >= FIRST_CHAR && (byte)*text < FIRST_CHAR+NUM_CHARS)
#elseif FIRST_CHAR+NUM_CHARS < 255
        if ((byte)*text < FIRST_CHAR+NUM_CHARS)
#endif
        {
            if (*text == '\n')
            {
                if (length > maxLen) maxLen = length;
                length = 0.f;
                continue;
            }
            length += LWidth[(byte)*text]; // Increase Length By Each Characters Width
        }
    if (length > maxLen) maxLen = length;
    return maxLen;
}

#endif
