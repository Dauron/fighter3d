#include "Application.h"

void MainWindow_OnCreate(IWindow &window, void* reciever)
{ g_Application.MainWindow_OnCreate(window); }
void MainWindow_OnResize(IWindow &window, void* reciever, unsigned int &width, unsigned int &height)
{ g_Application.MainWindow_OnResize(window, width, height); }

int Application::Create(const char *title, unsigned int width, unsigned int height,
                                           bool fl_fullscreen, Scene &scene)
{
    assert( !MainWindow );

    if (FL_OpenGL)
        MainWindow = new GLWindow();
    //else
    //  MainWindow = new DXWindow();

    if (! MainWindow->Create(title, width, height, fl_fullscreen) )
    {
        Destroy();
        return WINDOW_ERROR;
    }
    bool res = true; OnApplicationCreate(res);
    if (!res)
    {
        Destroy();
        return EVENT_ERROR;
    }
    SceneCur = &scene;
    MainWindow->OnCreate.Set((void*)this, ::MainWindow_OnCreate);
    MainWindow->OnResize.Set((void*)this, ::MainWindow_OnResize);
    if (! SceneCur->Create(0, 0, width, height) )
    {
        Destroy();
        return SCENE_ERROR;
    }
    SceneCur->Enter();

    return SUCCESS;
}

int Application::Invalidate()
{
    if (!MainWindow || MainWindow->IsDestroyed()) return SUCCESS;

    int result = SUCCESS;

    bool res = true; OnApplicationInvalidate(res);
    if (!res)
        result |= EVENT_ERROR;

    if (! SceneCur->Invalidate() )
        result |= SCENE_ERROR;

    return result;
}

void Application::Destroy()
{
    if (MainWindow) { MainWindow->Destroy(); delete MainWindow; }
    if (SceneCur)   { SceneCur->Exit(); SceneCur->Destroy(); delete SceneCur; }
    bool res = true; OnApplicationDestroy(res);

    Clear();
}

int Application::Run()
{
    float T_currrent  = GetTick();
    float T_preRender = T_currrent;
    float T_prev;
    Performance.Clear();

    while (MainWindow && !MainWindow->IsDestroyed())
    {
        if (!MainWindow->ProcessMessages()) break;

        T_prev     = T_currrent;
        T_currrent = GetTick();
        float T_total  = T_currrent - T_prev;
        float T_render = T_currrent - T_preRender;

        SceneCur->FrameStart();

        Update(T_render * 0.001f);

        T_preRender = GetTick();
        Render();

        if (!MainWindow || MainWindow->IsDestroyed()) return 0;
        SceneCur->FrameEnd();
    }
    return 0;
}

bool Application::Update(float T_delta)
{
    if (!MainWindow || MainWindow->IsDestroyed()) return false;

    if (MainWindow->IsActive())
    {
        Performance.Update(T_delta);
        return SceneCur->Update(T_delta);
    }
    return true;
}

bool Application::Render()
{
    if (!MainWindow || MainWindow->IsDestroyed()) return false;

    if (MainWindow->IsActive())
    {
        bool res = SceneCur->Render();
        MainWindow->SwapBuffers();
        return res;
    }
    return true;
}
