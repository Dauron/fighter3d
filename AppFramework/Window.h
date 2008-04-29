#ifndef __incl_Window_h
#define __incl_Window_h

#include "System.h"

#ifndef WIN32
#include <GL/glx.h>
#include <X11/extensions/xf86vmode.h>
typedef ::Display *HDC;
#endif

class IWindow
{
  protected:
    static const char *CLASS_NAME;

    ::HDC        hDC;

    char*        title;
    bool         fullscreen;
    bool         active;
    unsigned int width, height;
    bool         terminated;

    void OnResized(unsigned int width, unsigned int height);

  public:
    IWindow() : hDC(NULL), title(NULL), active(false), terminated(true) {}
    virtual ~IWindow() {};

    ::HDC  HDC()                                 { return hDC; }

    virtual bool Initialize(const char *title, unsigned int width, unsigned int height, bool fullscreen) = 0;
    virtual void Terminate() = 0;
    bool Terminated()                            { return terminated; }

    virtual bool ProcessMessages() = 0;

    bool  FullScreen()                           { return fullscreen; }
    bool  SetFullScreen(bool isFullScreen);

    bool  Active()                               { return active; }
    void  SetActive(bool isActive)               { active = isActive; }

    int   Height()                               { return height; }
    int   Width()                                { return width; }

    virtual void SwapBuffers() = 0;

#ifdef WIN32
    friend LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#else
    friend bool WindowProc(IWindow *thisWnd, XEvent &event);
#endif
};

#endif
