#include "Window.h"
#include "Application.h"

const char *IWindow::CLASS_NAME = "OMFW_GL";

void IWindow::OnResized(unsigned int width, unsigned int height)
{
    assert(!terminated);

    if(height==0) height=1;

    this->width = width;
    this->height = height;

    g_Application.CurrentScene().Resize(0, 0, width, height);
}

bool IWindow::SetFullScreen(bool fullscreen)
{
    assert(!terminated);

//#ifndef NDEBUG
    if (fullscreen)
    {
        width  = Config::FullScreenX;
        height = Config::FullScreenY;
    }
    else
    {
        width  = Config::WindowX;
        height = Config::WindowY;
    }
//#endif

    return Initialize(title, width, height, fullscreen)
        && g_Application.CurrentScene().Initialize(0, 0, width, height);
}
