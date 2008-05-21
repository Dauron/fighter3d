#include "Application.h"

bool Application::Initialize(char *title, unsigned int width, unsigned int height,
                             bool fullscreen, Scene* scene)
{
    if (scene != NULL)
    {
        if (m_scene) m_scene->Terminate();
        m_scene = scene;
    }
    if (!m_scene)
        throw "The scene cannot be null";

    bool error = m_window->Initialize(title, width, height, fullscreen);
    if (OnApplicationInitialize) OnApplicationInitialize(this);
    error |= m_scene->Initialize(0, 0, width, height);
    return error;
}

bool Application::SetCurrentScene(Scene* scene, bool destroyPrev)
{
    if (!scene)
        throw "The scene cannot be null";
    Scene* oldScene = m_scene;
    m_scene = scene;
    bool res = m_scene->Initialize(0,0,m_window->Width(), m_window->Height());
    if (oldScene && destroyPrev)
    {
        oldScene->Terminate();
        delete oldScene;
    }

    return res;
}

bool Application::Invalidate()
{
    m_scene->Invalidate();
    if (OnApplicationInvalidate) OnApplicationInvalidate(this);
    return true;
}

void Application::Terminate()
{
    m_window->Terminate();
    m_scene->Terminate();
    delete m_scene;
    if (OnApplicationTerminate) OnApplicationTerminate(this);
}

int Application::Run()
{
    float preRenderTick, prevTick, curTick = GetTick();
    preRenderTick = curTick;
    Performance.Reset();

    while (!m_window->Terminated())
    {
        if (!m_window->ProcessMessages()) break;

        prevTick = curTick;
        curTick = GetTick();
        float realTicks   = curTick - prevTick;
        float renderTicks = curTick - preRenderTick;

        Performance.NextFrame(realTicks);
        this->Update(renderTicks * 0.001f);
        if (m_window->Terminated()) break;

        preRenderTick = GetTick();
        this->Render();
    }
    return 0;
}

bool Application::Update(float deltaTime)
{
    if (m_window->Terminated()) return false;

    if (m_window->Active())
        return m_scene->Update(deltaTime);

    return true;
}

bool Application::Render()
{
    if (m_window->Terminated()) return false;

    if (m_window->Active())
    {
        bool res = m_scene->Render();
        m_window->SwapBuffers();
        return res;
    }
    return true;
}
