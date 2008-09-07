#ifndef __incl_SceneConsole_h
#define __incl_SceneConsole_h

#include <string>
#include "../Math/xMath.h"
#include "../AppFramework/IScene.h"
#include "../Graphics/OGL/Fonts/FontMgr.h"

namespace Scenes {

    class SceneConsole : public IScene
    {
      public:
        SceneConsole() { Name = "[Console]"; Clear(); }

        void Clear()
        {
            T_carretTick = 0;
            font = font15 = HFont();
            FL_overlayInput = false;
            FL_overlayClock = false;
        }

        virtual bool Create(int left, int top, unsigned int width, unsigned int height, IScene *prevScene = NULL);
        virtual void Destroy();

        virtual void Enter();
        virtual void Exit();
        virtual void Resize(int left, int top, unsigned int width, unsigned int height);

        virtual void FrameStart() { if (PrevScene) PrevScene->FrameStart(); }
        virtual bool Update(float T_delta);
        virtual bool Render();
        virtual void FrameEnd()   { if (PrevScene) PrevScene->FrameEnd(); }

        virtual bool   ShellCommand(std::string &cmd, std::string &output);

        virtual IScene &Scene_Set(IScene& scene, bool fl_destroyPrevious = true);

      private:
        void InitInputMgr();
        void AppendConsole(std::string text);

        HFont    font;
        HFont    font15;

        float    T_carretTick;
        bool     FL_carretVisible;

        int      scroll_v;
        int      pageSize;

        bool     FL_justOpened;   // skip the key that has opened the console
        bool     FL_visible;
        bool     FL_overlayInput; // send keys to subscene
        bool     FL_overlayClock; // send ticks to subscene

        std::string history;    // console history
        int         histLines;  // no of lines in history
        std::string currCmd;    // currently edited command
    };

} // namespace Scenes

#endif

