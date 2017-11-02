#include "Application.h"
#include "OGL/GLWindow.h"
#include "../Utils/Profiler.h"

void MainWindow_OnCreate(IWindow &window, void* receiver)
{ g_Application.MainWindow_OnCreate(window); }
void MainWindow_OnResize(IWindow &window, void* receiver, unsigned int &width, unsigned int &height)
{ g_Application.MainWindow_OnResize(window, width, height); }

int Application::Create(const char *title, unsigned int width, unsigned int height,
                                           bool fl_fullscreen, IScene &scene)
{
    assert( !MainWindow );

    if (FL_OpenGL)
        MainWindow = new GLWindow();
    //else
    //  MainWindow = new DXWindow();

    MainWindow->PreCreate(title, width, height, fl_fullscreen);

    return Create( *MainWindow, scene );
}

int Application::Create(IWindow &window, IScene &scene)
{
    assert( !MainWindow || MainWindow == &window );

    MainWindow = &window;

    // Create window - assume that window has been precreated
    if (! window.Create() )
    {
        Destroy();
        return WINDOW_ERROR;
    }

    // Call event
    bool res = true; OnApplicationCreate(res);
    if (!res)
    {
        Destroy();
        return EVENT_ERROR;
    }

    // Init window events
    SceneCur = &scene;
    window.OnCreate.Set(this, ::MainWindow_OnCreate);
    window.OnResize.Set(this, ::MainWindow_OnResize);

    // Create scene
    if (! SceneCur->Create(0, 0, window.Width_Get(), window.Height_Get()) )
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
    //float T_preRender = T_currrent;
    float T_prev;

    g_Profiler.ClearSamples();

    while (MainWindow && !MainWindow->IsDestroyed())
    {
        {
            Profile("Main Loop");

            if (!MainWindow->ProcessMessages()) break;

            T_prev     = T_currrent;
            T_currrent = GetTick();
            float T_total  = T_currrent - T_prev;
            //float T_render = T_currrent - T_preRender;

            SceneCur->FrameStart();

            Update(T_total * 0.001f);

            //T_preRender = GetTick();
            Render();

            if (!MainWindow || MainWindow->IsDestroyed()) return 0;
            SceneCur->FrameEnd();
        }
        g_Profiler.FrameEnd();
    }
    return 0;
}

bool Application::Update(float T_delta)
{
    if (!MainWindow || MainWindow->IsDestroyed()) return false;

    if (MainWindow->IsActive())
    {
        Profile("Update Frame");
        return SceneCur->Update(T_delta);
    }
    return true;
}

bool Application::Render()
{
    if (!MainWindow || MainWindow->IsDisposed()) return false;

    if (MainWindow->IsActive())
    {
        Profile("Render Frame");

        bool res = SceneCur->Render();

        {
            Profile("Swap buffers");
            MainWindow->SwapBuffers();
        }
        return res;
    }
    return true;
}
