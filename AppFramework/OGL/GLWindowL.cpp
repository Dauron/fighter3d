#ifndef WIN32

#include "GLWindow.h"
#include <cstdio>
#include <cstring>
#include <cassert>

/* attributes for a single buffered visual in RGBA format with at least
 *  * 4 bits per color and a 16 bit depth buffer */
static int attrListSgl[] = {GLX_RGBA,
    GLX_RED_SIZE, 8,
    GLX_GREEN_SIZE, 8,
    GLX_BLUE_SIZE, 8,
    GLX_DEPTH_SIZE, 24,
    GLX_STENCIL_SIZE, 8,
    None};

/* attributes for a double buffered visual in RGBA format with at least
 *  * 4 bits per color and a 16 bit depth buffer */
static int attrListDbl[] = { GLX_RGBA, GLX_DOUBLEBUFFER,
    GLX_RED_SIZE, 8,
    GLX_GREEN_SIZE, 8,
    GLX_BLUE_SIZE, 8,
    GLX_DEPTH_SIZE, 24,
    GLX_STENCIL_SIZE, 8,
    None };

bool GLWindow::Create()
{
    assert ( !IsDestroyed() );
    assert ( IsDisposed() );

    XVisualInfo *vi;
    Colormap cmap;
    int dpyWidth, dpyHeight;
    int i;
    int glxMajorVersion, glxMinorVersion;
    int vidModeMajorVersion, vidModeMinorVersion;
    XF86VidModeModeInfo **modes;
    int modeNum;
    int bestMode;
    Atom wmDelete;
    Window winDummy;
    unsigned int borderDummy;

    /* set best mode to current */
    bestMode = 0;
    /* get a connection */
    hDC = XOpenDisplay(0);
    screen = DefaultScreen(hDC);
    XF86VidModeQueryVersion(hDC, &vidModeMajorVersion, &vidModeMinorVersion);
    printf("XF86VidModeExtension-Version %d.%d\n", vidModeMajorVersion,
        vidModeMinorVersion);
    XF86VidModeGetAllModeLines(hDC, screen, &modeNum, &modes);
    /* save desktop-resolution before switching modes */
    deskMode = *modes[0];
    /* look for mode with requested resolution */
    for (i = 0; i < modeNum; i++)
        if ((modes[i]->hdisplay == Width) && (modes[i]->vdisplay == Height))
            bestMode = i;
    /* get an appropriate visual */
    vi = glXChooseVisual(hDC, screen, attrListDbl);
    if (vi == NULL)
    {
        vi = glXChooseVisual(hDC, screen, attrListSgl);
        printf("Only Singlebuffered Visual!\n");
    }
    else
        printf("Got Doublebuffered Visual!\n");
    glXQueryVersion(hDC, &glxMajorVersion, &glxMinorVersion);
    printf("glX-Version %d.%d\n", glxMajorVersion, glxMinorVersion);
    /* create a GLX context */
    glctx = glXCreateContext(hDC, vi, 0, GL_TRUE);
    /* create a color map */
    cmap = XCreateColormap(hDC, RootWindow(hDC, vi->screen),
        vi->visual, AllocNone);
    attr.colormap = cmap;
    attr.border_pixel = 0;

    if (FL_fullscreen)
    {
        XF86VidModeSwitchToMode(hDC, screen, modes[bestMode]);
        XF86VidModeSetViewPort(hDC, screen, 0, 0);
        dpyWidth = modes[bestMode]->hdisplay;
        dpyHeight = modes[bestMode]->vdisplay;
        printf("Resolution %dx%d\n", dpyWidth, dpyHeight);
        XFree(modes);

        /* create a fullscreen window */
        attr.override_redirect = True;
        attr.event_mask = ExposureMask | KeyPressMask | ButtonPressMask |
            FocusChangeMask | EnterWindowMask | StructureNotifyMask;
        win = XCreateWindow(hDC, RootWindow(hDC, vi->screen),
            0, 0, dpyWidth, dpyHeight, 0, vi->depth, InputOutput, vi->visual,
            CWBorderPixel | CWColormap | CWEventMask | CWOverrideRedirect,
            &attr);
        XWarpPointer(hDC, None, win, 0, 0, 0, 0, 0, 0);
        XMapRaised(hDC, win);
        XGrabKeyboard(hDC, win, True, GrabModeAsync,
            GrabModeAsync, CurrentTime);
        XGrabPointer(hDC, win, True, ButtonPressMask,
            GrabModeAsync, GrabModeAsync, win, None, CurrentTime);
    }
    else
    {
        /* create a window in window mode*/
        attr.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask |
            PointerMotionMask | FocusChangeMask | StructureNotifyMask;
        win = XCreateWindow(hDC, RootWindow(hDC, vi->screen),
            0, 0, Width, Height, 0, vi->depth, InputOutput, vi->visual,
            CWBorderPixel | CWColormap | CWEventMask, &attr);
        /* only set window title and handle wm_delete_events if in windowed mode */
        wmDelete = XInternAtom(hDC, "WM_DELETE_WINDOW", True);
        XSetWMProtocols(hDC, win, &wmDelete, 1);
        XSetStandardProperties(hDC, win, Title,
            Title, None, NULL, 0, NULL);
        XMapRaised(hDC, win);
    }
    /* connect the glx-context to the window */
    glXMakeCurrent(hDC, win, glctx);
    XGetGeometry(hDC, win, &winDummy, &x, &y,
        &Width, &Height, &borderDummy, &depth);
    printf("Depth %d\n", depth);
    if (glXIsDirect(hDC, glctx))
        printf("Congrats, you have Direct Rendering!\n");
    else
        printf("Sorry, no Direct Rendering possible!\n");

    OnCreate();
    this->Resize(Width, Height);

    return true;
}

void GLWindow::Dispose()
{
    if (glctx)
    {
        if (!glXMakeCurrent(hDC, None, NULL))
            printf("Could not release drawing context.\n");
        glXDestroyContext(hDC, glctx);
    }
    /* switch back to original desktop resolution if we were in fs */
    if (FL_fullscreen)
    {
        XF86VidModeSwitchToMode(hDC, screen, &deskMode);
        XF86VidModeSetViewPort(hDC, screen, 0, 0);
    }
    if (hDC) XCloseDisplay(hDC);

    glctx = NULL;
    hDC   = NULL;
}

#endif
