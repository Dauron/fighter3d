#include "GLWindow.h"
#include "../Application.h"
#include "../../Utils/Debug.h"

#ifdef WIN32

#pragma warning(disable : 4996) // deprecated

#include "../../Graphics/OGL/Extensions/wglext.h"

bool GLWindow::Initialize(const char *title, unsigned int width, unsigned int height, bool fullscreen)
{
    if (!terminated) this->Terminate();

    // Register window class
    WNDCLASS wc;
    wc.style         = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
    wc.lpfnWndProc   = (WNDPROC) WindowProc;
    wc.cbClsExtra    = 0;                                        // No extra class data
    wc.cbWndExtra    = 0;                                        // No extra window data
    wc.hInstance     = hInstance = GetModuleHandle(NULL);        // Instance of our window
    wc.hIcon         = LoadIcon(NULL, IDI_WINLOGO);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;                                     // Not needed for GL
    wc.lpszMenuName  = NULL;                                     // No menu
    wc.lpszClassName = CLASS_NAME;                               // Window class name

    if (!RegisterClass(&wc))                                     // Attempt To Register The Window Class
    {
        MessageBox(NULL, "Failed To Register The Window Class.", "ERROR", MB_OK|MB_ICONEXCLAMATION);
        return false;
    }

    if (fullscreen)
    {
        DEVMODE dmScreenSettings;                                // Device Mode
        memset(&dmScreenSettings,0,sizeof(dmScreenSettings));    // Makes Sure Memory's Cleared
        //EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dmScreenSettings);
        dmScreenSettings.dmSize=sizeof(dmScreenSettings);        // Size Of The Devmode Structure
        dmScreenSettings.dmPelsWidth    = width;                 // Selected Screen Width
        dmScreenSettings.dmPelsHeight   = height;                // Selected Screen Height
        dmScreenSettings.dmBitsPerPel   = 32;                    // Selected Bits Per Pixel
        dmScreenSettings.dmFields       = DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

        // Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
        if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
        {
            // If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
            if (MessageBox(NULL,"The requested fullscreen mode is not supported by\nyour video card. Use windowed mode instead?",title,MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
                fullscreen = false;
            else
            {
                MessageBox(NULL,"Program will now close.","ERROR",MB_OK|MB_ICONSTOP);
                this->Terminate();
                return false;
            }
        }
    }

    DWORD        dwExStyle;                                      // Window Extended Style
    DWORD        dwStyle;                                        // Window Style
    if (fullscreen)                                              // Are We Still In Fullscreen Mode?
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
    WindowRect.right  = width;
    WindowRect.top    = 0;
    WindowRect.bottom = height;
    AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);     // Adjust Window To True Requested Size

    // Create The Window
    if (!(hWnd=CreateWindowEx(  dwExStyle,                          // Extended Style For The Window
                                CLASS_NAME,                         // Class Name
                                title,                              // Window Title
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
        this->Terminate();
        MessageBox(NULL,"Window creation error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return false;
    }

    SetLastError(0);
    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG)this);
    if (GetLastError())
    {
        this->Terminate();
        MessageBox(NULL,"Window creation error - cannot associate GLWindow with hWnd.","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return false;
    }

    if (!(hDC=GetDC(hWnd)))                            // Did We Get A Device Context?
    {
        this->Terminate();
        MessageBox(NULL,"Can't create a OpenGL device context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
        return false;
    }

    ZeroMemory(&pfd, sizeof(pfd));
    pfd.nSize      = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion   = 1;                                // Version Number
    pfd.dwFlags    = PFD_DRAW_TO_WINDOW |              // Format Must Support Window
                     PFD_SUPPORT_OPENGL |              // Format Must Support OpenGL
                     PFD_GENERIC_ACCELERATED |         // Enable hardware acceleration
                     PFD_DOUBLEBUFFER,                 // Must Support Double Buffering
    pfd.iPixelType = PFD_TYPE_RGBA;                    // Request An RGBA Format
    pfd.cColorBits = 32;                               // Select Our Color Depth
    pfd.cDepthBits = 24;                               // 16Bit Z-Buffer (Depth Buffer)
    pfd.cStencilBits = 8;                              // Use Stencil Buffer
    pfd.iLayerType = PFD_MAIN_PLANE;                   // Main Drawing Layer

    if (queryForMultisample || !multisampleAviable)
        if(!(PixelFormat = ChoosePixelFormat(hDC, &pfd)))
        {
            this->Terminate();
            MessageBox(hWnd, "Can't find a suitable PixelFormat.", "ERROR", MB_OK|MB_ICONEXCLAMATION);
            return false;
        }

    if(!SetPixelFormat(hDC, PixelFormat, &pfd))
    {
        this->Terminate();
        MessageBox(hWnd, "Failed to set pixel format.", "ERROR", MB_OK|MB_ICONEXCLAMATION);
        return false;
    }

    if(!(hRC = wglCreateContext(hDC)))
    {
        this->Terminate();
        MessageBox(hWnd, "Failed to create the OpenGL rendering context.", "ERROR", MB_OK|MB_ICONEXCLAMATION);
        return false;
    }

    if(!wglMakeCurrent(hDC, hRC))
    {
        this->Terminate();
        MessageBox(hWnd, "Failed to make current the OpenGL rendering context.", "ERROR", MB_OK|MB_ICONEXCLAMATION);
        return false;
    }
    CheckForGLError("wglMakeCurrent");

    terminated = false;

    if (queryForMultisample && Config::MultisamplingLevel > 0)
    {
        PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB =
            (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
        multisampleAviable = false;
        if (wglChoosePixelFormatARB)
        {
            multisampleAviable = true;
            int iAttributes[] = { WGL_DRAW_TO_WINDOW_ARB,GL_TRUE,
                WGL_SUPPORT_OPENGL_ARB,GL_TRUE,
                WGL_ACCELERATION_ARB,WGL_FULL_ACCELERATION_ARB,
                WGL_COLOR_BITS_ARB,24,
                WGL_ALPHA_BITS_ARB,8,
                WGL_DEPTH_BITS_ARB,24,
                WGL_STENCIL_BITS_ARB,8,
                WGL_DOUBLE_BUFFER_ARB,GL_TRUE,
                WGL_SAMPLE_BUFFERS_ARB,GL_TRUE,
                WGL_SAMPLES_ARB, 4 ,                        // Check For 4x Multisampling
                0,0};
            float fAttributes[] = {0,0};
            unsigned int NumFormats;


            // First We Check To See If We Can Get A Pixel Format For 4 Samples
            iAttributes[19] = Config::MultisamplingLevel*2;
            while (Config::MultisamplingLevel &&
                   !wglChoosePixelFormatARB(hDC, iAttributes, fAttributes, 1, &PixelFormat, &NumFormats) || NumFormats == 0)
            {
                --Config::MultisamplingLevel;
                iAttributes[19] = Config::MultisamplingLevel*2;
            }

            if (Config::MultisamplingLevel == 0)
                multisampleAviable = false;
        }
        queryForMultisample = false;
        if (multisampleAviable)
            return Initialize(title, width, height, fullscreen);
    }

    ShowWindow(hWnd, SW_SHOW);
    SetForegroundWindow(hWnd);
    SetFocus(hWnd);
    Config::FullScreen = this->fullscreen = fullscreen;
    this->OnResized(width, height);

    if (this->title != title)
    {
        if (this->title) delete[] this->title;
        this->title = new char[strlen(title)+1];
        strcpy (this->title, title);
    }

    return true;
}

void GLWindow::Terminate()
{
    g_Application.Invalidate();

    if (hRC)                                             // Do We Have A Rendering Context?
    {
        if (!wglMakeCurrent(NULL,NULL))                  // Are We Able To Release The DC And RC Contexts?
            MessageBox(NULL,"Release of DC and RC failed.","SHUTDOWN ERROR", MB_OK|MB_ICONINFORMATION);

        if (!wglDeleteContext(hRC))                      // Are We Able To Delete The RC?
            MessageBox(NULL,"Release rendering context failed.","SHUTDOWN ERROR",MB_OK|MB_ICONINFORMATION);
    }
    hRC = NULL;                                          // Set RC To NULL

    if (hDC && !ReleaseDC(hWnd,hDC))                     // Are We Able To Release The DC
        MessageBox(NULL,"Release device context failed.","SHUTDOWN ERROR",MB_OK|MB_ICONINFORMATION);
    hDC = NULL;                                          // Set DC To NULL

    if (hWnd && !DestroyWindow(hWnd))                    // Are We Able To Destroy The Window?
        MessageBox(NULL,"Could not release hWnd.","SHUTDOWN ERROR",MB_OK|MB_ICONINFORMATION);
    hWnd = NULL;                                         // Set hWnd To NULL

    if (hInstance && !UnregisterClass(CLASS_NAME,hInstance)) // Are We Able To Unregister Class
        MessageBox(NULL,"Could not unregister class.","SHUTDOWN ERROR",MB_OK|MB_ICONINFORMATION);
    hInstance = NULL;                                    // Set hInstance To NULL

    if (fullscreen)                                      // Are We In Fullscreen Mode?
    {
        ChangeDisplaySettings(NULL,0);                   // If So Switch Back To The Desktop
        ShowCursor(TRUE);                                // Show Mouse Pointer
    }

    terminated = true;
}

#endif
