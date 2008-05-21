#ifndef __incl_Application_h
#define __incl_Application_h

#include "GL/GLWindow.h"
#include "../Utils/Singleton.h"
#include "Scene.h"

#define g_Application Application::GetSingleton()

class Application : public Singleton<Application>
{
  public:
    // Initializes application with given scene... the scene should be a dynamical object 
    // (it will be deleted by this class automaticaly on application termination)
    bool Initialize(char* title, unsigned int width, unsigned int height,
                    bool fullscreen, Scene* scene = NULL);
    bool Invalidate();
    void Terminate();

    int  Run();
    bool Update(float deltaTime);
    bool Render();

    IWindow&  MainWindow()   { return *m_window; }
    Scene&    CurrentScene() { return *m_scene; }
    bool      SetCurrentScene(Scene* scene, bool destroyPrev = true);

    Application() : m_scene(NULL), m_title(NULL), m_OpenGL(true), OnApplicationInvalidate(NULL), OnApplicationTerminate(NULL)
    {
        if (m_OpenGL)
            m_window = new GLWindow();
        //else
        //  m_window = new DXWindow();
    }
    ~Application() { delete m_window; }

    bool OpenGL()  { return m_OpenGL; }
    bool DirectX() { return !m_OpenGL; }

    void (*OnApplicationInvalidate)(Application* sender);
    void (*OnApplicationTerminate) (Application* sender);

  private:
     // copy constructor 
    Application(const Application&) {}
     // assignment operator 
    Application& operator=(const Application&) { return *this; }

    IWindow *m_window;
    Scene   *m_scene;
    char    *m_title;
    bool     m_OpenGL;
};

#endif

