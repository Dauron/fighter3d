#ifndef __incl_DXWindow_h
#define __incl_DXWindow_h

#ifdef USE_D3D

#include "../IWindow.h"

#include <d3d9.h>
// include the Direct3D Library file
#pragma comment (lib, "d3d9.lib")

class DXWindow : public IWindow
{
private:
    HWND       hWnd;
    HINSTANCE  hInstance;

    LPDIRECT3D9       d3d;
    LPDIRECT3DDEVICE9 d3ddev;

    int                   PixelFormat;
    PIXELFORMATDESCRIPTOR pfd;
    bool                  FL_queryForMultisample;
    bool                  FL_multisampleAviable;

public:
    ::HWND            HWND()   { return hWnd; }
    LPDIRECT3DDEVICE9 D3DDEV() { return d3ddev; }

    bool IsOpenGL() { return false; }

    DXWindow() { Clear(); }

    void Clear()
    {
        IWindow::Clear();
        d3ddev    = NULL;
        d3d       = NULL;
        hWnd      = NULL;
        hInstance = NULL;
        FL_queryForMultisample = true;
        FL_multisampleAviable  = false;
    }

    virtual bool Create();
    virtual void Dispose();

    virtual void SwapBuffers() { ::SwapBuffers(hDC); }

    virtual bool ProcessMessages()
    {
        MSG msg;
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
