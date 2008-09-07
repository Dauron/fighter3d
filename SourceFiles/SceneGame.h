#ifndef __incl_SceneGame_h
#define __incl_SceneGame_h

#include "../AppFramework/IScene.h"
#include "../Math/Cameras/CameraSet.h"
#include "../Graphics/OGL/Fonts/FontMgr.h"
#include "../Graphics/OGL/ISelectionProvider.h"
#include "../World/World.h"
#include "../World/SkeletizedObj.h"

namespace Scenes {

    class SceneGame : public IScene, private ISelectionProvider
    {
    public:
        SkeletizedObj              *Player1;
        SkeletizedObj              *Player2;
        std::string                 MapFileName;

        SceneGame() { Name="[Game]"; Clear(); };

        void Clear()
        {
            MainCamera = NULL;
            Player1    = NULL;
            Player2    = NULL;
            MapFileName.clear();
            Targets.L_objects.clear();
        }

        virtual bool Create(int left, int top, unsigned int width, unsigned int height, IScene *prevScene = NULL);
        virtual void Destroy();

        virtual bool Invalidate();
        virtual void Resize(int left, int top, unsigned int width, unsigned int height)
        {
            IScene::Resize(left, top, width, height);
            InitCameras();
        }

        virtual void FrameStart()             { Map.FrameStart(); }
        virtual bool Update(float T_delta);
        virtual bool Render();
        virtual void FrameEnd()               { Map.FrameEnd(); }

        virtual bool ShellCommand(std::string &cmd, std::string &output);

    private:
        bool InitMap();
        void FreeMap();
        void InitCameras();
        void InitInputMgr();

        World                       Map;
        Math::Cameras::Camera      *MainCamera;
        Math::Cameras::CameraSet    Cameras;
        Math::Tracking::TrackingSet Targets;

        virtual void RenderSelect(const Math::Cameras::FieldOfView &FOV);
        unsigned int CountSelectable()
        { return Map.objects.size(); }
        RigidObj *Select(int X, int Y);
    };

} // namespace Scenes

#endif
