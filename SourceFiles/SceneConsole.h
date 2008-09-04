#ifndef __incl_SceneConsole_h
#define __incl_SceneConsole_h

#include <string>
#include "../Math/xMath.h"
#include "../AppFramework/Scene.h"
#include "../Graphics/OGL/Fonts/FontMgr.h"

namespace Scenes {

    class SceneConsole : public Scene
    {
      public:
        SceneConsole(Scene *prevScene)
        {
            PrevScene = prevScene;
            carretTick = 0;
            font = font15 = HFont();
            overlayInput = false;
            overlayClock = false;
            visible      = !PrevScene;
            SceneName    = "[Console]";
        }

        virtual bool Initialize(int left, int top, unsigned int width, unsigned int height);
        virtual void Resize(int left, int top, unsigned int width, unsigned int height);
        virtual void Terminate();

        virtual void FrameStart() { if (PrevScene) PrevScene->FrameStart(); }
        virtual bool FrameUpdate(float deltaTime);
        virtual bool FrameRender();
        virtual void FrameEnd() { if (PrevScene) PrevScene->FrameEnd(); }

        virtual Scene *SetCurrentScene(Scene* scene, bool destroyPrev = true);
        virtual bool   ShellCommand(std::string &cmd, std::string &output);

      private:
        void InitInputMgr();
        void AppendConsole(std::string text);

        HFont    font;
        HFont    font15;

        float    carretTick;
        bool     carretVisible;

        int      scroll_v;
        int      pageSize;

        bool     justOpened;   // skip the key that has opened the console
        bool     visible;
        bool     overlayInput; // send keys to subscene
        bool     overlayClock; // send ticks to subscene

        std::string history;    // console history
        int         histLines;  // no of lines in history
        std::string currCmd;    // currently edited command
    };

} // namespace Scenes

#endif

