#ifndef __incl_Scene_h
#define __incl_Scene_h

#ifndef NULL
#define NULL    0
#endif

#include <string>
#include "Input/InputMgr.h"

class Scene
{
public:
    const char  * Name;
    Scene       * PrevScene;

    Scene() { Name = NULL; FL_terminated = true; PrevScene = NULL; }

    virtual bool Create(int left, int top, unsigned int width, unsigned int height, Scene *prevScene = NULL);
    virtual void Destroy();

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

    virtual Scene &Scene_Set(Scene& scene, bool fl_destroyPrevious = true);

    bool IsTerminated() { return FL_terminated; }

protected:
    signed   int Left;
    signed   int Top;
    unsigned int Width;
    unsigned int Height;
    bool         FL_terminated;
};

#endif

