#include "Scene.h"

bool Scene :: Create(int left, int top, unsigned int width, unsigned int height, Scene *prevScene)
{
    assert ( FL_terminated );
    FL_terminated = false;
    Left          = left;
    Top           = top;
    Width         = width;
    Height        = height;
    if (prevScene)
        PrevScene = prevScene;
    else
    if (PrevScene)
        PrevScene->Create(left, top, width, height);
    return true;
}

void Scene :: Destroy()
{
    if (FL_terminated) return;

    FL_terminated = true;
    if (PrevScene)
    {
        PrevScene->Destroy();
        delete PrevScene;
        PrevScene = NULL;
    }
}
void Scene :: Resize(int left, int top, unsigned int width, unsigned int height)
{
    Left   = left;
    Top    = top;
    Width  = width;
    Height = height;
}

Scene & Scene :: Scene_Set(Scene& scene, bool fl_destroyPrevious)
 {
     if (fl_destroyPrevious)
     {
        if (!scene.FL_terminated || scene.Create(0, 0, Width, Height))
        {
            this->Exit();
            this->Destroy();
            delete this;
            scene.Enter();
            return scene;
        }
     }
     else
     if (!scene.FL_terminated || scene.Create(0, 0, Width, Height, this))
     {
         this->Exit();
         scene.Enter();
         return scene;
     }

    return *this;
}
