#ifndef __incl_Window_h
#define __incl_Window_h

#include "System.h"
#include "../Utils/Delegate.h"
#include <cassert>
#include <cstring>

#ifndef WIN32
#include <GL/glx.h>
#include <X11/extensions/xf86vmode.h>
typedef ::Display *HDC;
#endif

class IWindow
{
#ifdef WIN32
    friend LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#else
    friend bool WindowProc(IWindow *thisWnd, XEvent &event);
#endif

protected:
    static const char *CLASS_NAME;

    ::HDC        hDC;

    char*        Title;
    bool         FL_fullscreen;
    bool         FL_active;
    unsigned int Width, Height;
    bool         FL_destroyed;

    void Resize(unsigned int width, unsigned int height);

public:
    typedef Delegate<IWindow> WindowCreateEvent;
    typedef Delegate<IWindow, unsigned int /*Width*/, unsigned int /*Height*/> WindowResizeEvent;

    WindowCreateEvent OnCreate;
    WindowResizeEvent OnResize;

    IWindow() { Clear(); }

    virtual void Clear()
    {
        hDC          = NULL;
        Title        = NULL;
        FL_active    = false;
        FL_destroyed = true;
        OnCreate     = WindowCreateEvent(*this);
        OnResize     = WindowResizeEvent(*this);
    }

    ::HDC  HDC()                                 { return hDC; }

    virtual void PreCreate(const char *title, unsigned int width, unsigned int height, bool fl_fullscreen)
    {
        assert (IsDestroyed());
        FL_destroyed  = false;
        Title         = strdup(title);
        Width         = width;
        Height        = height;
        FL_fullscreen = fl_fullscreen;
    }
    virtual bool Create() = 0;
    bool Create(const char *title, unsigned int width, unsigned int height, bool fl_fullscreen)
    {
        PreCreate(title, width, height, fl_fullscreen);
        return Create();
    }
    virtual void Dispose() = 0;
    virtual void Destroy()
    {
        Dispose();
        if (Title) delete[] Title;
        Clear();
    }
    bool IsDisposed()                            { return !hDC; }
    bool IsDestroyed()                           { return FL_destroyed; }

    virtual bool ProcessMessages() = 0;

    bool  IsFullScreen()                         { return FL_fullscreen; }
    bool  FullScreen_Set(unsigned int width, unsigned int height, bool fl_fullscreen);
    bool  FullScreen_Switch()                    { return FullScreen_Set(Width, Height, !FL_fullscreen); }

    bool  IsActive()                             { return FL_active; }
    void  Active_Set(bool isActive)              { FL_active = isActive; }

    int   Height_Get()                           { return Height; }
    int   Width_Get()                            { return Width; }

    virtual void SwapBuffers() = 0;
};

#endif
