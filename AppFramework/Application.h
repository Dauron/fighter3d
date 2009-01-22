#ifndef __incl_Application_h
#define __incl_Application_h

#include "IWindow.h"
#include "../Utils/Singleton.h"
#include "IScene.h"

#define g_Application Application::GetSingleton()

class Application : public Singleton<Application>
{
public:
    enum AppResult { // FLAGS
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
        MainWindow              = NULL;
    }

    // Creates application with given scene... the scene should be a dynamical object
    // (it will be deleted by this class automaticaly on application termination)
    int  Create(const char* title, unsigned int width, unsigned int height,
                bool fl_openGL,    bool fl_fullscreen, IScene &scene);
    // Creates application with given precreated window and scene... both should be a dynamical object
    // (they will be deleted by this class automaticaly on application termination)
    int  Create(IWindow &window, IScene &scene);
    void Destroy();

    int  Invalidate();

    int  Run();
    bool Update(float T_delta);
    bool Render();

    IWindow&  MainWindow_Get() { return *MainWindow; }
    IScene&   Scene_Get()      { return *SceneCur; }
    bool      Scene_Set(IScene& scene, bool fl_destroyPrevious = true)
    {
        SceneCur = &SceneCur->Scene_Set(scene, fl_destroyPrevious);
        return SceneCur == &scene;
    }

    bool      IsOpenGL()       { return MainWindow->IsOpenGL(); }

    typedef Delegate<Application, bool> ApplicationEvent;
    ApplicationEvent OnApplicationCreate;
    ApplicationEvent OnApplicationInvalidate;
    ApplicationEvent OnApplicationDestroy;

private:
     // copy constructor
    Application(const Application&) {}
     // assignment operator
    Application& operator=(const Application&) { return *this; }

    friend void MainWindow_OnCreate(IWindow &window, void* receiver);
    friend void MainWindow_OnResize(IWindow &window, void* receiver, unsigned int &width, unsigned int &height);

    void MainWindow_OnCreate(IWindow &window)
    { Invalidate(); }
    void MainWindow_OnResize(IWindow &window, unsigned int width, unsigned int height)
    { if (SceneCur) SceneCur->Resize(0,0,width,height); }

    IWindow *MainWindow;
    IScene  *SceneCur;
    char    *Title;
    bool     FL_terminated;
};

#endif
