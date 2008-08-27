#ifndef __incl_SceneGame_h
#define __incl_SceneGame_h

#include "../AppFramework/Scene.h"
#include "../Math/Cameras/CameraSet.h"
#include "../Graphics/OGL/Fonts/FontMgr.h"
#include "../Graphics/OGL/ISelectionProvider.h"
#include "../World/World.h"

class SceneGame : public Scene, private ISelectionProvider
{
public:
    Math::Cameras::Camera *DefaultCamera;

    SceneGame() : DefaultCamera(NULL) { sceneName="[Game]"; };

    virtual bool Initialize(int left, int top, unsigned int width, unsigned int height);
    virtual bool Invalidate();
    virtual void Terminate();

    virtual void FrameStart() {  world.FrameStart(); }
    virtual bool FrameUpdate(float deltaTime);
    virtual bool FrameRender();
    virtual void FrameEnd() { world.FrameEnd(); }

    virtual void Resize(int left, int top, unsigned int width, unsigned int height)
    {
        Scene::Resize(left, top, width, height);
        InitCameras();
    }

    virtual bool ShellCommand(std::string &cmd, std::string &output);

private:
    bool InitGL();
    bool InitWorld();
    void FreeWorld();
    void InitCameras();
    void InitInputMgr();

    long  accum;
    float stepAccum;

    World world;
    Math::Cameras::CameraSet    Cameras;
    Math::Tracking::TrackingSet Targets;

    virtual void RenderSelect(const Math::Cameras::FieldOfView &FOV);
    unsigned int CountSelectable()
    { return world.objects.size(); }
    RigidObj *Select(int X, int Y);
};

#endif
