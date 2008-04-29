#include "Application.h"
#include "Scene.h"
#include "../OpenGL/Fonts/FontMgr.h"
#include "Input/InputMgr.h"
#include "../OpenGL/Textures/TextureMgr.h"
#include "../OpenGL/GLAnimSkeletal.h"
#include "../Models/ModelMgr.h"
#include "../Models/lib3dx/xAnimationMgr.h"

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
    error |= init_ARB_extensions();
    error |= Invalidate();
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
    if (!InputMgr::GetSingletonPtr())
        new InputMgr();

    if (!FontMgr::GetSingletonPtr())
        new FontMgr();
    else
        g_FontMgr.InvalidateItems();

    if (!TextureMgr::GetSingletonPtr())
        new TextureMgr();
    else
        g_TextureMgr.InvalidateItems();

    if (!GLAnimSkeletal::GetSingletonPtr())
        new GLAnimSkeletal();
    else
        g_AnimSkeletal.ReinitializeGL();

    if (!xAnimationMgr::GetSingletonPtr())
        new xAnimationMgr();
    else
        g_AnimationMgr.InvalidateItems();

    if (!ModelMgr::GetSingletonPtr())
        new ModelMgr();
    else
        g_ModelMgr.InvalidateItems();

    m_scene->Invalidate();

    return true;
}

void Application::Terminate()
{
    m_scene->Terminate();
    delete m_scene;
    m_window->Terminate();
    if (ModelMgr::GetSingletonPtr())
        delete ModelMgr::GetSingletonPtr();
    if (xAnimationMgr::GetSingletonPtr())
        delete xAnimationMgr::GetSingletonPtr();
    if (FontMgr::GetSingletonPtr())
        delete FontMgr::GetSingletonPtr();
    if (TextureMgr::GetSingletonPtr())
        delete TextureMgr::GetSingletonPtr();
    if (InputMgr::GetSingletonPtr())
        delete InputMgr::GetSingletonPtr();
    if (GLAnimSkeletal::GetSingletonPtr())
        delete GLAnimSkeletal::GetSingletonPtr();
}

int Application::Run()
{
    float prevTick, curTick = GetTick();

    while (!m_window->Terminated())
    {
        if (!m_window->ProcessMessages()) break;

        prevTick = curTick;
        curTick = GetTick();

        this->Update((curTick - prevTick)/1000.f);
        if (m_window->Terminated()) break;
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
