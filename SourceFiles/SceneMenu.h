#ifndef __incl_SceneMenu_h
#define __incl_SceneMenu_h

#include "../AppFramework/IScene.h"
#include "../Graphics/FontMgr.h"
#include "MenuStates/BaseState.h"

namespace Scenes {

    class SceneMenu : public IScene
    {
    public:
        SceneMenu() {
            Name="[Menu]";
            font03 = font04 = HFont();
            root = NULL;
        };

        virtual bool Create(int left, int top, unsigned int width, unsigned int height, IScene *prevScene = NULL);
        virtual void Destroy();

        virtual bool Invalidate();
        virtual void Resize(int left, int top, unsigned int width, unsigned int height);

        virtual bool Update(float deltaTime);
        virtual bool Render();

        HFont    font03; // 0.03 Window Height
        HFont    font04; // 0.04 Window Height

    private:
        void InitInputMgr();

        Menu::BaseState *root;
    };

} // namespace Scenes

#endif
