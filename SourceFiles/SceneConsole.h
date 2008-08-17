#ifndef __incl_SceneConsole_h
#define __incl_SceneConsole_h

#include <string>
#include "../Math/xMath.h"
#include "../AppFramework/Scene.h"
#include "../Graphics/OGL/Fonts/FontMgr.h"

class SceneConsole : public Scene
{
  public:
    SceneConsole(Scene *prevScene)
    {
        PrevScene = prevScene;
        carretTick = 0;
        font = HFont();
        overlayInput = false;
        overlayClock = false;
        visible      = !PrevScene;
        sceneName    = "[Console]";
    }

    virtual bool Initialize(int left, int top, unsigned int width, unsigned int height);
    virtual void Resize(int left, int top, unsigned int width, unsigned int height);
    virtual void Terminate();

    virtual void FrameStart() { if (PrevScene) PrevScene->FrameStart(); }
    virtual bool FrameUpdate(float deltaTime);
    virtual bool FrameRender();
    virtual void FrameEnd() { if (PrevScene) PrevScene->FrameEnd(); }

    Scene * SceneConsole :: SetCurrentScene(Scene* scene, bool destroyPrev = true);

    void AppendConsole(std::string text);
    virtual bool ShellCommand(std::string &cmd, std::string &output);

  private:
    void InitInputMgr();

    HFont    font;
    HFont    font15;

    float    carretTick;
    bool     carretVisible;

    int      scroll_v;
    int      pageSize;

    bool     justOpened; // skip the key that has opened the console
    bool     visible;
    bool     overlayInput;
    bool     overlayClock;

    std::string history;    // console history
    int         histLines;  // no of lines in history
    std::string currCmd;    // currently edited command
};

#endif

