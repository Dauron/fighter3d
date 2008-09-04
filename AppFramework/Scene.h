#ifndef __incl_Scene_h
#define __incl_Scene_h

#ifndef NULL
#define NULL    0
#endif

#include <string>

class Scene
{
public:
    Scene       * PrevScene;
    const char  * SceneName;

    virtual bool Initialize(int left, int top, unsigned int width, unsigned int height)
    {
        Initialized = true;
        Resize (left, top, width, height);
        return true;
    }
    virtual bool Invalidate()
    { return PrevScene ? PrevScene->Invalidate() : true; }
    virtual void Terminate()
    {
        Initialized = false;
        if (PrevScene)
        {
            PrevScene->Terminate();
            delete PrevScene;
            PrevScene = NULL;
        }
    }

    virtual void FrameStart() {}
    virtual bool FrameUpdate(float deltaTime) = 0;
    virtual bool FrameRender() = 0;
    virtual void FrameEnd() {}

    virtual bool ShellCommand(std::string &cmd, std::string &output) { return false; }
    virtual Scene *SetCurrentScene(Scene* scene, bool destroyPrev = true);

    virtual void Resize(int left, int top, unsigned int width, unsigned int height)
    {
        Left        = left;
        Top         = top;
        Width       = width;
        Height      = height;
        AspectRatio = ((float)width)/height;
    }

    Scene() { SceneName = NULL; Initialized = false; PrevScene = NULL; }
    virtual ~Scene() {}

protected:
    int          Left;
    int          Top;
    unsigned int Width;
    unsigned int Height;
    float        AspectRatio;
    bool         Initialized;
};

#endif

