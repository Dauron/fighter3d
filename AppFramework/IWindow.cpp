#include "IWindow.h"
#include <cassert>

const char *IWindow::CLASS_NAME = "OMFW_GL";

void IWindow::Resize(unsigned int width, unsigned int height)
{
    assert(!FL_destroyed);

    if (height==0) height=1;

    Width  = width;
    Height = height;

    OnResize(Width, Height);
}

bool IWindow::FullScreen_Set(unsigned width, unsigned height, bool fl_fullscreen)
{
    assert(!FL_destroyed);

    char *title = Title;
    Title = NULL;
    Destroy();
    if (!Create(title, width, height, fl_fullscreen))
    {
        delete[] title;
        return false;
    }
    delete[] title;
    return true;
}
