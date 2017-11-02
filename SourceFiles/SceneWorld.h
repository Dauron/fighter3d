#ifndef __incl_SceneWorld_h
#define __incl_SceneWorld_h

#include "../AppFramework/IScene.h"
#include "../Graphics/FontMgr.h"
#include "../Math/Cameras/CameraHuman.h"
#include "../World/Map.h"
#include "../Graphics/OGL/MapRender.h"
#include "../Graphics/OGL/DeferredFBO.h"
#include "../Math/xLight.h"
#include "../Utils/Stat.h"

namespace Scenes {

    class SceneWorld : public IScene
    {
    public:
        SceneWorld() {
            Clear();
        }

        void Clear()
        {
            Name      = "[World]";
            font02 = font04 = HFont();
            mouseX = mouseY = -1;
            FL_followHawk = false;
        }

        virtual bool Create(int left, int top, unsigned int width, unsigned int height, IScene *prevScene = NULL);
        virtual void Destroy();

        virtual bool Invalidate();
        virtual void Resize(int left, int top, unsigned int width, unsigned int height);

        virtual bool Update(float deltaTime);
        virtual bool Render();

    private:
        HFont font02, font04;                 // Fonts

        World::Map                     map;
        Graphics::OGL::MapRender       mapRender;
        Graphics::OGL::DeferredFBO     deferred;

        void InitInputMgr();

        xLight sun, torch;                    // Sun and Torch lights

        Math::Cameras::CameraHuman Camera;    // Player camera
        Math::Cameras::CameraHuman CamFollow; // Camera used for following
        bool FL_followHawk;                   // Are we in follow mode?

        int mouseX, mouseY;

        // Responce for hawk to bird conversion event (replace CamFollow target)
        friend void Map_OnHawkReplace(World::Map &map, void* receiver, World::Hawk *&hawk, World::Bird *&bird);
        void HawkReplace(World::Hawk *hawk, World::Bird *bird);
        // Responce for bird eaten by hawk event (replace CamFollow target)
        friend void Map_OnBirdRemove(World::Map &map, void* receiver, World::Hawk *&hawk, World::Bird *&bird);
        void BirdRemove(World::Hawk *hawk, World::Bird *bird);
    };

} // namespace Scenes

#endif
