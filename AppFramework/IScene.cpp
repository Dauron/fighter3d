#include "IScene.h"

bool IScene :: Create(int left, int top, unsigned int width, unsigned int height, IScene *prevScene)
{
    assert ( FL_destroyed );
    FL_destroyed = false;
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

void IScene :: Destroy()
{
    if (FL_destroyed) return;

    FL_destroyed = true;
    if (PrevScene)
    {
        PrevScene->Destroy();
        delete PrevScene;
        PrevScene = NULL;
    }
}
void IScene :: Resize(int left, int top, unsigned int width, unsigned int height)
{
    Left   = left;
    Top    = top;
    Width  = width;
    Height = height;
    if (PrevScene) PrevScene->Resize(left, top, width, height);
}

IScene & IScene :: Scene_Set(IScene& scene, bool fl_destroyPrevious)
 {
     if (fl_destroyPrevious)
     {
        if (!scene.FL_destroyed || scene.Create(0, 0, Width, Height))
        {
            this->Exit();
            this->Destroy();
            delete this;
            scene.Enter();
            return scene;
        }
     }
     else
     if (!scene.FL_destroyed || scene.Create(0, 0, Width, Height, this))
     {
         this->Exit();
         scene.Enter();
         return scene;
     }

    return *this;
}
