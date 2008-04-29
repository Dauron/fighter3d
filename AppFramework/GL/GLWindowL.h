#ifndef __incl_GLWindowL_h
#define __incl_GLWindowL_h

class GLWindow : public IWindow
{
  private:
    int                  screen;
    Window               win;
    GLXContext           glctx;
    XSetWindowAttributes attr;
    XF86VidModeModeInfo  deskMode;
    unsigned int         depth;

    int x, y;

  public:
     GLWindow() : glctx(NULL) {}
    virtual ~GLWindow() { this->Terminate(); }

    bool Initialize(const char *title, unsigned int width, unsigned int height, bool fullscreen);
    void Terminate();

    virtual void SwapBuffers(){ glXSwapBuffers(hDC,win); }

    virtual bool ProcessMessages()
    {
        XEvent event;
        while (XPending(hDC) > 0)
        {
            XNextEvent(hDC, &event);
            if (!WindowProc(this, event)) return false;
        }
        return true;
    }
};

#endif
