#include "Scene.h"
#include "Application.h"

Scene * Scene :: SetCurrentScene(Scene* scene, bool destroyPrev)
 {
    if (!scene)
        throw "The scene cannot be null";

    scene->Initialize(0,0,g_Application.MainWindow().Width(), g_Application.MainWindow().Height());
    if (destroyPrev)
    {
        Terminate();
        delete this;
    }
    else
        scene->PrevScene = this;

    return scene;
}
