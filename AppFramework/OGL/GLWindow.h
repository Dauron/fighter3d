#ifndef __incl_GLWindow_h
#define __incl_GLWindow_h

#include "../IWindow.h"
#include <GL/gl.h>

#ifndef WIN32

#include "GLWindowL.h"

#else

class GLWindow : public IWindow
{
private:
    HWND       hWnd;
    HGLRC      hRC;
    HINSTANCE  hInstance;
    MSG        msg;

    int                   PixelFormat;
    PIXELFORMATDESCRIPTOR pfd;
    bool                  FL_queryForMultisample;
    bool                  FL_multisampleAviable;

public:
    GLWindow() { Clear(); }

    void Clear()
    {
        IWindow::Clear();
        hWnd      = NULL;
        hRC       = NULL;
        hInstance = NULL;
        FL_queryForMultisample = true;
        FL_multisampleAviable  = false;
    }

    virtual bool Create();
    virtual void Dispose();

    virtual void SwapBuffers() { ::SwapBuffers(hDC); }

    virtual bool ProcessMessages()
    {
        if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if(msg.message==WM_QUIT) return false;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        return true;
    }
};

#endif

#endif
