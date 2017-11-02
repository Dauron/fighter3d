#ifndef __incl_Scene_h
#define __incl_Scene_h

#ifndef NULL
#define NULL    0
#endif

#include <string>
#include "Input/InputMgr.h"

class IScene
{
public:
    const char  * Name;
    IScene      * PrevScene;

    IScene() { Name = NULL; FL_destroyed = true; PrevScene = NULL; }
    virtual ~IScene() = default;

    virtual bool Create(int left, int top, unsigned int width, unsigned int height, IScene *prevScene = NULL);
    virtual void Destroy();
    bool IsDestroyed() { return FL_destroyed; }

    virtual void Enter() { Resize (Left, Top, Width, Height); g_InputMgr.SetScene(Name); }
    virtual void Exit()  {}

    // Invalidates scene data (OpenGL data, etc.)
    // returns: true on success
    virtual bool Invalidate()
    { return PrevScene ? PrevScene->Invalidate() : true; }
    virtual void Resize(int left, int top, unsigned int width, unsigned int height);

    virtual void FrameStart()          {}
    virtual bool Update(float T_delta) = 0;
    virtual bool Render()              = 0;
    virtual void FrameEnd()            {}

    // Processes given text command
    // returns: true on correct command
    virtual bool ShellCommand(std::string &cmd, std::string &output) { return false; }

    virtual IScene &Scene_Set(IScene& scene, bool fl_destroyPrevious = true);

protected:
    signed   int Left;
    signed   int Top;
    unsigned int Width;
    unsigned int Height;
    bool         FL_destroyed;
};

#endif

