#include "SceneMenu.h"

#include "../AppFramework/Application.h"
#include "../AppFramework/Input/InputMgr.h"
#include "../Graphics/OGL/AnimSkeletal.h"

#include "MenuStates/MenuStates.h"

using namespace Scenes;

Menu::BaseState* Menu::BaseState::Current = NULL;

bool SceneMenu :: Create(int left, int top, unsigned int width, unsigned int height, IScene *prevScene)
{
    IScene::Create(left, top, width, height, prevScene);
    InitInputMgr();

    root = new Menu::MainState();
    root->Init(NULL);
    Menu::BaseState::SwitchState(*root);

    GLExtensions::SetVSync(Config::VSync);

    return true;
}

void SceneMenu :: InitInputMgr()
{
    InputMgr &im = g_InputMgr;
    im.SetScene(Name);

    im.Key2InputCode_SetIfKeyFree(VK_RETURN,  IC_Accept);
    im.Key2InputCode_SetIfKeyFree(VK_ESCAPE,  IC_Reject);
    im.Key2InputCode_SetIfKeyFree(VK_F11,     IC_FullScreen);
    im.Key2InputCode_SetIfKeyFree(VK_UP,      IC_MoveUp);
    im.Key2InputCode_SetIfKeyFree(VK_DOWN,    IC_MoveDown);
    im.Key2InputCode_SetIfKeyFree(VK_LBUTTON, IC_LClick);
    im.Key2InputCode_SetIfKeyFree(VK_F11,     IC_FullScreen);
}

void SceneMenu :: Destroy()
{
    g_FontMgr.Release(font03);
    g_FontMgr.Release(font04);
    font03 = font04 = HFont();

    if (root)
    {
        root->Clear();
        delete root;
        root = NULL;
    }

    IScene::Destroy();
}

bool SceneMenu :: Invalidate()
{
    if (Menu::BaseState::Current_Get())
        Menu::BaseState::Current_Get()->Invalidate();
    return true;
}

void SceneMenu :: Resize(int left, int top, unsigned int width, unsigned int height)
{
    IScene::Resize(left, top, width, height);

    g_FontMgr.Release(font03);
    font03 = g_FontMgr.GetFont("Arial", (int)(Height * 0.025f));
    g_FontMgr.Release(font04);
    font04 = g_FontMgr.GetFont("Arial", (int)(Height * 0.035f));
}

bool SceneMenu :: Update(float deltaTime)
{
    if (g_InputMgr.InputDown_GetAndRaise(IC_FullScreen))
    {
        if (g_Application.MainWindow_Get().IsFullScreen())
            g_Application.MainWindow_Get().FullScreen_Set(Config::WindowX, Config::WindowY, false);
        else
            g_Application.MainWindow_Get().FullScreen_Set(Config::FullScreenX, Config::FullScreenY, true);
        return true;
    }

    return Menu::BaseState::Current_Get()->Update(deltaTime);
}

bool SceneMenu :: Render()
{
    glClearColor( 0.f, 0.f, 0.f, 0.f );
    glColorMask(1,1,1,1);
    glClear(GL_COLOR_BUFFER_BIT);

    glViewport(Left, Top, Width, Height);
    glDisable(GL_DEPTH_TEST);
    Graphics::OGL::Shader::SetLightType(xLight_NONE);
    Graphics::OGL::Shader::EnableTexturing(Graphics::OGL::xState_Off);
    glDisable (GL_POLYGON_SMOOTH);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    //glEnable (GL_BLEND);                    // Enable blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Set projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, Width, Height, 0, 0, 100);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Draw backgroud
    glColor4f( 1.f, 1.f, 1.f, 1.f );
    glBegin(GL_QUADS);
        glVertex2f(0.f, 0.f);
        glVertex2f((GLfloat)Width, 0.f);
        glVertex2f((GLfloat)Width, (GLfloat)Height);
        glVertex2f(0.f, (GLfloat)Height);
    glEnd();

    //glDisable(GL_BLEND);

    const Graphics::OGL::Font* pFont03 = g_FontMgr.GetFont(font03);
    const Graphics::OGL::Font* pFont04 = g_FontMgr.GetFont(font04);

    Menu::BaseState::Current_Get()->Render(pFont03, pFont04, Width, Height);


    //////////////////// WORLD - END
    glFlush(); //glFinish();
    return true;
}
