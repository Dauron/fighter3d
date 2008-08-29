#include "SceneMenu.h"

#include "../App Framework/Application.h"
#include "../App Framework/Input/InputMgr.h"
#include "../Graphics/OGL/GLAnimSkeletal.h"

#include "MenuStates/MenuStates.h"

BaseState* BaseState::Current = NULL;

bool SceneMenu :: Initialize(int left, int top, unsigned int width, unsigned int height)
{
    Scene::Initialize(left, top, width, height);
    InitInputMgr();

    if (!root)
    {
        root = new MainState();
        root->Init(NULL);
        BaseState::SwitchState(*root);
    }
    
    return true;
}

void SceneMenu :: Resize(int left, int top, unsigned int width, unsigned int height)
{
	if (Initialized)
	{
		Scene::Resize(left, top, width, height);

        g_FontMgr.DeleteFont(font03);
        font03 = g_FontMgr.GetFont("Courier New", (int)(Height * 0.03f));
        g_FontMgr.DeleteFont(font04);
        font04 = g_FontMgr.GetFont("Courier New", (int)(Height * 0.04f));
        g_FontMgr.DeleteFont(font05);
		font05 = g_FontMgr.GetFont("Courier New", (int)(Height * 0.05f));
        g_FontMgr.DeleteFont(font10);
		font10 = g_FontMgr.GetFont("Courier New", (int)(Height * 0.10f));
	}
}

void SceneMenu :: InitInputMgr()
{
    InputMgr &im = g_InputMgr;
    im.SetScene(sceneName);

    im.SetInputCodeIfKeyIsFree(VK_RETURN, IC_Accept);
    im.SetInputCodeIfKeyIsFree(VK_ESCAPE, IC_Reject);
    im.SetInputCodeIfKeyIsFree(VK_F11,    IC_FullScreen);
    im.SetInputCodeIfKeyIsFree(VK_BACK,   IC_Con_BackSpace);
#ifdef WIN32
    im.SetInputCodeIfKeyIsFree(VK_OEM_3,  IC_Console);
#else
    im.SetInputCodeIfKeyIsFree('`',       IC_Console);
#endif
    im.SetInputCodeIfKeyIsFree(VK_UP,    IC_MoveUp);
    im.SetInputCodeIfKeyIsFree(VK_DOWN,  IC_MoveDown);
    im.SetInputCodeIfKeyIsFree(VK_LEFT,  IC_MoveLeft);
    im.SetInputCodeIfKeyIsFree(VK_RIGHT, IC_MoveRight);

    im.SetInputCodeIfKeyIsFree(VK_LBUTTON, IC_LClick);
    im.SetInputCodeIfKeyIsFree(VK_RBUTTON, IC_RClick);

    im.SetInputCodeIfKeyIsFree(VK_F11,   IC_FullScreen);
}

void SceneMenu :: Terminate()
{
    g_FontMgr.DeleteFont(font03);
    g_FontMgr.DeleteFont(font04);
    g_FontMgr.DeleteFont(font05);
    g_FontMgr.DeleteFont(font10);
    font03 = font04 = font05 = font10 = HFont();

    if (root)
    {
        root->Clear();
        delete root;
        root = NULL;
    }

    Scene::Terminate();
}
    
bool SceneMenu :: Invalidate()
{
    if (BaseState::Current_Get())
        BaseState::Current_Get()->Invalidate();
    return true; 
}
    
bool SceneMenu :: FrameUpdate(float deltaTime)
{
    if (g_InputMgr.GetInputStateAndClear(IC_FullScreen))
    {
        g_Application.MainWindow().SetFullScreen(!g_Application.MainWindow().FullScreen());
        return true;
    }

    return BaseState::Current_Get()->Update(deltaTime);
}
    
bool SceneMenu :: FrameRender()
{
    glClearColor( 0.f, 0.f, 0.f, 0.f );
    glColorMask(1,1,1,1);
    glClear(GL_COLOR_BUFFER_BIT);

    glViewport(Left, Top, Width, Height);
    glDisable(GL_DEPTH_TEST);
    GLShader::SetLightType(xLight_NONE);
    GLShader::EnableTexturing(xState_Off);
    glDisable (GL_POLYGON_SMOOTH);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable (GL_BLEND);                    // Enable blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Set projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, Width, 0, Height, 0, 100);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Draw backgroud
    glColor4f( 0.7f, 0.7f, 1.f, 0.6f );
    glBegin(GL_QUADS);
        glVertex2f(0.f, 0.f);
        glVertex2f((GLfloat)Width, 0.f);
        glVertex2f((GLfloat)Width, (GLfloat)Height);
        glVertex2f(0.f, (GLfloat)Height);
    glEnd();

    const GLFont* pFont03 = g_FontMgr.GetFont(font03);
    const GLFont* pFont04 = g_FontMgr.GetFont(font04);
    const GLFont* pFont05 = g_FontMgr.GetFont(font05);
    const GLFont* pFont10 = g_FontMgr.GetFont(font10);

    BaseState::Current_Get()->Render(pFont03, pFont04, pFont05, pFont10, Width, Height);

    glDisable(GL_BLEND);

    //////////////////// WORLD - END
    glFlush(); //glFinish();
    return true;
}
