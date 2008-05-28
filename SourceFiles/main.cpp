#include "../AppFramework/Application.h"
#include "../Utils/Debug.h"
#include "../Utils/Filesystem.h"
#include "../Graphics/OGL/GLShader.h"

#include "SceneGame.h"

void OnApplicationInitialize(Application* sender);
void OnApplicationInvalidate(Application* sender);
void OnApplicationTerminate (Application* sender);

int main( int argc, char **argv )
{
    Filesystem::WorkingDirectory = Filesystem::GetParentDir(argv[0]);
    Filesystem::SetSystemWorkingDirectory(Filesystem::WorkingDirectory);

    logEx(0, false, "***********************************");
    logEx(0, true, "Game started");

    GLShader::Load();

    Application game;
    game.OnApplicationInitialize = OnApplicationInitialize;
    game.OnApplicationInvalidate = OnApplicationInvalidate;
    game.OnApplicationTerminate  = OnApplicationTerminate;
    if (!game.Initialize("Camera Fighter", 800, 600, false, new SceneGame()))
        return 1;
    int res = game.Run();
    game.Terminate();

    GLShader::Unload();

    logEx(0, true, "Game finished");
    logEx(0, false, "***********************************");

    return res;
}

#ifdef WIN32

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nShowCmd)
{
    int argc;
    WCHAR EMPTY_WSTR = 0;
    LPWSTR *argv = ::CommandLineToArgvW(&EMPTY_WSTR, &argc);

    int   lenJ = 0;
    char *buff = new char[1000];
    buff[0] = 0;

    for (int i = 0; i < argc; ++i)
    {
        int lenA = ::WideCharToMultiByte(CP_ACP, 0, argv[i], -1, 0, 0, NULL, NULL);
        if (lenA)
        {
            char *part = new char[lenA];

            ::WideCharToMultiByte(CP_ACP, 0, argv[i], -1, part, lenA, NULL, NULL);
            memcpy (buff+lenJ, part, lenA);

            delete[] part;
        }
        lenJ += lenA;
        buff[lenJ-1] = ' ';
        buff[lenJ]   = 0;
    }
    int res = main(1, &buff);
    delete[] buff;
    return res;
}

#endif

#include "../AppFramework/Input/InputMgr.h"
#include "../MotionCapture/CaptureInput.h"
#include "../Graphics/OGL/Textures/TextureMgr.h"
#include "../Graphics/OGL/Fonts/FontMgr.h"
#include "../Graphics/OGL/GLAnimSkeletal.h"
#include "../Models/ModelMgr.h"
#include "../Models/lib3dx/xAnimationMgr.h"

void OnApplicationInitialize(Application* sender)
{
    GLExtensions::Init();

    if (!InputMgr::GetSingletonPtr())
        new InputMgr();

    if (!CaptureInput::GetSingletonPtr())
        new CaptureInput();

    if (!FontMgr::GetSingletonPtr())
        new FontMgr();

    if (!TextureMgr::GetSingletonPtr())
        new TextureMgr();

    GLShader::Initialize();

    if (!GLAnimSkeletal::GetSingletonPtr())
        new GLAnimSkeletal();

    if (!xAnimationMgr::GetSingletonPtr())
        new xAnimationMgr();

    if (!ModelMgr::GetSingletonPtr())
        new ModelMgr();
}

void OnApplicationInvalidate(Application* sender)
{
    if (FontMgr::GetSingletonPtr())
        g_FontMgr.InvalidateItems();

    if (TextureMgr::GetSingletonPtr())
        g_TextureMgr.InvalidateItems();

    GLShader::Invalidate();

    if (xAnimationMgr::GetSingletonPtr())
        g_AnimationMgr.InvalidateItems();

    if (ModelMgr::GetSingletonPtr())
        g_ModelMgr.InvalidateItems();
}

void OnApplicationTerminate(Application* sender)
{
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
    if (CaptureInput::GetSingletonPtr())
        delete CaptureInput::GetSingletonPtr();
    if (GLAnimSkeletal::GetSingletonPtr())
        delete GLAnimSkeletal::GetSingletonPtr();
    GLShader::Terminate();
}
