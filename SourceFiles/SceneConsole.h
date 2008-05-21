#ifndef __incl_SceneConsole_h
#define __incl_SceneConsole_h

#include <string>
#include "../Math/xMath.h"
#include "../AppFramework/Scene.h"
#include "../OpenGL/Fonts/FontMgr.h"

class SceneConsole : public Scene
{
  public:
    SceneConsole(Scene *prevScene)
    {
        this->prevScene = prevScene;
        prevTick = 0;
        minFPS   = xDWORD_MAX; // max uint
        meanFPS  = 50.0f;
        maxFPS   = 0;
        carretTick = 0;
        font = HFont();
        overlayInput = false;
        overlayClock = false;
        sceneName = "[Console]";
    }

    virtual bool Initialize(int left, int top, unsigned int width, unsigned int height);
    virtual bool Invalidate() {
        return prevScene->Invalidate();
    }
    virtual void Resize(int left, int top, unsigned int width, unsigned int height);
    virtual void Terminate();
    virtual bool Update(float deltaTime);
    virtual bool Render();

    void AppendConsole(std::string text);
    bool ProcessCmd(std::string cmd);

  private:
    void InitInputMgr();

    Scene   *prevScene;
    HFont    font;

    float    prevTick;
    xDWORD   curFPS;
    xDWORD   minFPS;
    float    meanFPS;
    xDWORD   maxFPS;

    float    carretTick;
    bool     carretVisible;

    int      scroll_v;
    int      pageSize;

    bool     justOpened; // skip the key that has opened the console
    bool     overlayInput;
    bool     overlayClock;

    std::string history;    // console history
    int         histLines;  // no of lines in history
    std::string currCmd;    // currently edited command
};

#endif

