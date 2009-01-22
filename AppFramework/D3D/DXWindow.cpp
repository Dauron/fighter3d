#include "DXWindow.h"
#include "../../Utils/Debug.h"
#include <cassert>

#ifdef USE_D3D

bool DXWindow::Create()
{
    assert ( !IsDestroyed() );
    assert ( IsDisposed() );

    // Register window class
    WNDCLASS wc;
    wc.style         = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
    wc.lpfnWndProc   = (WNDPROC) WindowProc;
    wc.cbClsExtra    = 0;                                        // No extra class data
    wc.cbWndExtra    = 0;                                        // No extra window data
    wc.hInstance     = hInstance = GetModuleHandle(NULL);        // Instance of our window
    wc.hIcon         = LoadIcon(NULL, IDI_WINLOGO);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;                                     // Not needed for GL and DX
    wc.lpszMenuName  = NULL;                                     // No menu
    wc.lpszClassName = CLASS_NAME;                               // Window class name

    if (!RegisterClass(&wc))                                     // Attempt To Register The Window Class
    {
        MessageBox(NULL, "Failed To Register The Window Class.", "ERROR", MB_OK|MB_ICONEXCLAMATION);
        return false;
    }

    if (FL_fullscreen)
    {
        DEVMODE dmScreenSettings;                                // Device Mode
        memset(&dmScreenSettings,0,sizeof(dmScreenSettings));    // Makes Sure Memory's Cleared
        //EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dmScreenSettings);
        dmScreenSettings.dmSize=sizeof(dmScreenSettings);        // Size Of The Devmode Structure
        dmScreenSettings.dmPelsWidth    = Width;                 // Selected Screen Width
        dmScreenSettings.dmPelsHeight   = Height;                // Selected Screen Height
        dmScreenSettings.dmBitsPerPel   = 32;                    // Selected Bits Per Pixel
        dmScreenSettings.dmFields       = DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

        // Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
        if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
        {
            // If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
            if (MessageBox(NULL,"The requested fullscreen mode is not supported by\nyour video card. Use windowed mode instead?",Title,MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
                FL_fullscreen = false;
            else
            {
                MessageBox(NULL,"Program will now close.","ERROR",MB_OK|MB_ICONSTOP);
                this->Destroy();
                return false;
            }
        }
    }

    DWORD        dwExStyle;                                      // Window Extended Style
    DWORD        dwStyle;                                        // Window Style
    if (FL_fullscreen)                                           // Are We Still In Fullscreen Mode?
    {
        dwExStyle=WS_EX_APPWINDOW;                               // Window Extended Style
        dwStyle=WS_POPUP;                                        // Windows Style
        //ShowCursor(FALSE);                                     // Hide Mouse Pointer
    }
    else
    {
        dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;            // Window Extended Style
        dwStyle=WS_OVERLAPPEDWINDOW;                             // Windows Style
    }

    RECT WindowRect;
    WindowRect.left   = 0;
    WindowRect.right  = Width;
    WindowRect.top    = 0;
    WindowRect.bottom = Height;
    AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);     // Adjust Window To True Requested Size

    // Create The Window
    if (!(hWnd=CreateWindowEx(  dwExStyle,                          // Extended Style For The Window
                                CLASS_NAME,                         // Class Name
                                Title,                              // Window Title
                                dwStyle |                           // Defined Window Style
                                WS_CLIPSIBLINGS |                   // Required Window Style
                                WS_CLIPCHILDREN,                    // Required Window Style
                                CW_USEDEFAULT, CW_USEDEFAULT,       // Window Position
                                WindowRect.right-WindowRect.left,   // Calculate Window Width
                                WindowRect.bottom-WindowRect.top,   // Calculate Window Height
                                NULL,                               // No Parent Window
                                NULL,                               // No Menu
                                hInstance,                          // Instance
                                NULL)))                             // Dont Pass Anything To WM_CREATE
    {
        this->Destroy();
        MessageBox(NULL,"Window creation error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return false;
    }

    SetLastError(0);
    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG)this);
    if (GetLastError())
    {
        this->Destroy();
        MessageBox(NULL,"Window creation error - cannot associate GLWindow with hWnd.","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return false;
    }

    if (!(hDC=GetDC(hWnd)))                            // Did We Get A Device Context?
    {
        this->Destroy();
        MessageBox(NULL,"Can't create a OpenGL device context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return false;
    }

    ZeroMemory(&pfd, sizeof(pfd));
    pfd.nSize      = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion   = 1;                                // Version Number
    pfd.dwFlags    = PFD_DRAW_TO_WINDOW |              // Format Must Support Window
                     PFD_GENERIC_ACCELERATED |         // Enable hardware acceleration
                     PFD_DOUBLEBUFFER,                 // Must Support Double Buffering
    pfd.iPixelType = PFD_TYPE_RGBA;                    // Request An RGBA Format
    pfd.cColorBits = 32;                               // Select Our Color Depth
    pfd.cDepthBits = 24;                               // 16Bit Z-Buffer (Depth Buffer)
    pfd.cStencilBits = 8;                              // Use Stencil Buffer
    pfd.iLayerType = PFD_MAIN_PLANE;                   // Main Drawing Layer

    if (FL_queryForMultisample || !FL_multisampleAviable)
        if(!(PixelFormat = ChoosePixelFormat(hDC, &pfd)))
        {
            this->Destroy();
            MessageBox(hWnd, "Can't find a suitable PixelFormat.", "ERROR", MB_OK|MB_ICONEXCLAMATION);
            return false;
        }

    if(!SetPixelFormat(hDC, PixelFormat, &pfd))
    {
        this->Destroy();
        MessageBox(hWnd, "Failed to set pixel format.", "ERROR", MB_OK|MB_ICONEXCLAMATION);
        return false;
    }

    if(!(d3d = Direct3DCreate9(D3D_SDK_VERSION))) // create the Direct3D interface
    {
        this->Destroy();
        MessageBox(hWnd, "Failed to create the Direct3D rendering context.", "ERROR", MB_OK|MB_ICONEXCLAMATION);
        return false;
    }

    D3DPRESENT_PARAMETERS d3dpp;                    // create a struct to hold various device information
    ZeroMemory(&d3dpp, sizeof(d3dpp));              // clear out the struct for use
    d3dpp.Windowed         = !FL_fullscreen;        // program windowed, not fullscreen
    d3dpp.SwapEffect       = D3DSWAPEFFECT_DISCARD; // discard old frames
    d3dpp.hDeviceWindow    = hWnd;                  // set the window to be used by Direct3D
    d3dpp.BackBufferCount  = 1;
    d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;       // set the back buffer format to 32-bit
    d3dpp.BackBufferWidth  = Width;                 // set the width of the buffer
    d3dpp.BackBufferHeight = Height;                // set the height of the buffer
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;      // depth buffer size
    //d3dpp.MultiSampleQuality;
    //d3dpp.MultiSampleType;

    // create a device class using this information and information from the d3dpp stuct
    if (D3D_OK != d3d->CreateDevice(D3DADAPTER_DEFAULT,
                                    D3DDEVTYPE_HAL,
                                    hWnd,
                                    D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                    &d3dpp,
                                    &d3ddev))
    {
        this->Destroy();
        MessageBox(hWnd, "Failed to create the Direct3D device context.", "ERROR", MB_OK|MB_ICONEXCLAMATION);
        return false;
    }

    OnCreate();
    ShowWindow(hWnd, SW_SHOW);
    SetForegroundWindow(hWnd);
    SetFocus(hWnd);
    this->Resize(Width, Height);

    return true;
}

void DXWindow::Dispose()
{
    if (d3ddev)
        d3ddev->Release();    // close and release the 3D device

    if (d3d)
        d3d->Release();    // close and release Direct3D

    if (hDC && !ReleaseDC(hWnd,hDC))                     // Are We Able To Release The DC
        MessageBox(NULL,"Release device context failed.","SHUTDOWN ERROR",MB_OK|MB_ICONINFORMATION);

    if (hWnd && !DestroyWindow(hWnd))                    // Are We Able To Destroy The Window?
        MessageBox(NULL,"Could not release hWnd.","SHUTDOWN ERROR",MB_OK|MB_ICONINFORMATION);

    if (hInstance && !UnregisterClass(CLASS_NAME,hInstance)) // Are We Able To Unregister Class
        MessageBox(NULL,"Could not unregister class.","SHUTDOWN ERROR",MB_OK|MB_ICONINFORMATION);

    d3ddev    = NULL;
    d3d       = NULL;
    hDC       = NULL;
    hWnd      = NULL;
    hInstance = NULL;

    if (FL_fullscreen)                                   // Are We In Fullscreen Mode?
    {
        ChangeDisplaySettings(NULL,0);                   // If So Switch Back To The Desktop
        ShowCursor(TRUE);                                // Show Mouse Pointer
    }
}

#endif
