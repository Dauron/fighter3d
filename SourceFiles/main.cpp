#include "../AppFramework/Application.h"
#include "SceneGame.h"
#include "../Utils/Debug.h"

int main()
{
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
    main();
}

#endif
