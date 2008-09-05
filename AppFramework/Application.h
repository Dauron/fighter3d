#ifndef __incl_Application_h
#define __incl_Application_h

#include "OGL/GLWindow.h"
#include "../Utils/Singleton.h"
#include "Scene.h"

class Application;

#define g_Application Application::GetSingleton()
class Application : public Singleton<Application>
{
public:
    enum AppResult {
        SUCCESS      = 0,
        WINDOW_ERROR = 1,
        EVENT_ERROR  = 2,
        SCENE_ERROR  = 4
    };

    Application()  { Clear(); }

    void Clear()
    {
        FL_terminated           = true;
        OnApplicationCreate     = ApplicationEvent(*this);
        OnApplicationInvalidate = ApplicationEvent(*this);
        OnApplicationDestroy    = ApplicationEvent(*this);
        SceneCur                = NULL;
        Title                   = NULL;
        FL_OpenGL               = true;
        MainWindow              = NULL;
    }

    // Creates application with given scene... the scene should be a dynamical object
    // (it will be deleted by this class automaticaly on application termination)
    int  Create(const char* title, unsigned int width, unsigned int height,
                     bool fl_fullscreen, Scene &scene);
    void Destroy();

    int  Invalidate();

    int  Run();
    bool Update(float T_delta);
    bool Render();

    IWindow&  MainWindow_Get() { return *MainWindow; }
    Scene&    Scene_Get()      { return *SceneCur; }
    bool      Scene_Set(Scene& scene, bool fl_destroyPrevious = true)
    {
        SceneCur = &SceneCur->Scene_Set(scene, fl_destroyPrevious);
        return SceneCur == &scene;
    }

    bool IsOpenGL()  { return FL_OpenGL; }
    bool IsDirectX() { return !FL_OpenGL; }

    typedef Delegate<Application, bool> ApplicationEvent;
    ApplicationEvent OnApplicationCreate;
    ApplicationEvent OnApplicationInvalidate;
    ApplicationEvent OnApplicationDestroy;

    void MainWindow_OnCreate(IWindow &window)
    { Invalidate(); }
    void MainWindow_OnResize(IWindow &window, unsigned int width, unsigned int height)
    { if (SceneCur) SceneCur->Resize(0,0,width,height); }

private:
     // copy constructor
    Application(const Application&) {}
     // assignment operator
    Application& operator=(const Application&) { return *this; }

    IWindow *MainWindow;
    Scene   *SceneCur;
    char    *Title;
    bool     FL_OpenGL;
    bool     FL_terminated;
};

#endif

