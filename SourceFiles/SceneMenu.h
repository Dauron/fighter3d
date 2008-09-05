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
            Name="[Menu]";
            font03 = font04 = font05 = font10 = HFont();
            root = NULL;
        };

        virtual bool Create(int left, int top, unsigned int width, unsigned int height, Scene *prevScene = NULL);
        virtual void Destroy();

        virtual bool Invalidate();
        virtual void Resize(int left, int top, unsigned int width, unsigned int height);

        virtual bool Update(float deltaTime);
        virtual bool Render();

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
