#ifndef __incl_SceneMenu_h
#define __incl_SceneMenu_h

#include "../AppFramework/Scene.h"
#include "../Graphics/OGL/Fonts/FontMgr.h"
#include "MenuStates/BaseState.h"

namespace Scenes {

    class SceneMenu : public Scene
    {
    public:
        SceneMenu() {
            SceneName="[Menu]";
            font03 = font04 = font05 = font10 = HFont();
            root = NULL;
        };

        virtual bool Initialize(int left, int top, unsigned int width, unsigned int height);
        virtual void Resize(int left, int top, unsigned int width, unsigned int height);
        virtual bool Invalidate();
        virtual void Terminate();

        virtual bool FrameUpdate(float deltaTime);
        virtual bool FrameRender();

        HFont    font03; // 0.03 Window Height
        HFont    font04; // 0.04 Window Height
        HFont    font05; // 0.05 Window Height
        HFont    font10; // 0.10 Window Height

    private:
        void InitInputMgr();

        Menu::BaseState *root;
    };

} // namespace Scenes

#endif
