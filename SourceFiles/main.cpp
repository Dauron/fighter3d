#include "../AppFramework/Application.h"
#include "SceneGame.h"
#include "../Utils/Debug.h"
#include "../Utils/Filesystem.h"

int main( int argc, char **argv )
{
    Filesystem::WorkingDirectory = Filesystem::GetParentDir(argv[0]);
    Filesystem::SetSystemWorkingDirectory(Filesystem::WorkingDirectory);

    DEB__log(false, "***********************************");
    DEB__log(true, "Game started");

    Application game;
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

    int lenA = ::WideCharToMultiByte(CP_ACP, 0, argv[0], -1, 0, 0, NULL, NULL);
    if (lenA)
    {
        char *buff = new char[lenA];

        ::WideCharToMultiByte(CP_ACP, 0, argv[0], -1, buff, lenA, NULL, NULL);

        main(1, &buff);

        delete[] buff;
    }
}

#endif
