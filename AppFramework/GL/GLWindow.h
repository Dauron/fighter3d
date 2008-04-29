#ifndef __incl_GLWindow_h
#define __incl_GLWindow_h

#include "../Window.h"
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

  public:
     GLWindow() : hWnd(NULL), hRC(NULL), hInstance(NULL) {}
    virtual ~GLWindow() { this->Terminate(); }

    virtual bool Initialize(const char *title, unsigned int width, unsigned int height, bool fullscreen);
    virtual void Terminate();

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
