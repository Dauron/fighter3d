#include "../AppFramework/Application.h"
#include "SceneGame.h"
#include "../Utils/Debug.h"
#include "../Utils/Filesystem.h"

void OnApplicationInvalidate(Application* sender);
void OnApplicationTerminate (Application* sender);

int main( int argc, char **argv )
{
    Filesystem::WorkingDirectory = Filesystem::GetParentDir(argv[0]);
    Filesystem::SetSystemWorkingDirectory(Filesystem::WorkingDirectory);

    DEB__log(false, "***********************************");
    DEB__log(true, "Game started");

    Application game;
    game.OnApplicationInvalidate = OnApplicationInvalidate;
    game.OnApplicationTerminate = OnApplicationTerminate;
    if (!game.Initialize("Camera Fighter", 800, 600, false, new SceneGame()))
        return 1;
    int res = game.Run();
    game.Terminate();

    DEB__log(true, "Game finished");
    DEB__log(false, "***********************************");

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
    main(1, &buff);
    delete[] buff;
}

#include "../AppFramework/Input/InputMgr.h"
#include "../MotionCapture/CaptureInput.h"
#include "../OpenGL/Textures/TextureMgr.h"
#include "../OpenGL/Fonts/FontMgr.h"
#include "../OpenGL/GLAnimSkeletal.h"
#include "../Models/ModelMgr.h"
#include "../Models/lib3dx/xAnimationMgr.h"

void OnApplicationInvalidate(Application* sender)
{
    GL_Init_Extensions();

    if (!InputMgr::GetSingletonPtr())
        new InputMgr();

    if (!CaptureInput::GetSingletonPtr())
        new CaptureInput();

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
}

#endif
