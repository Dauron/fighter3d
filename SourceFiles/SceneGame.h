#ifndef __incl_SceneGame_h
#define __incl_SceneGame_h

#include "../AppFramework/Scene.h"
#include "../Math/xFieldOfView.h"
#include "../Math/Cameras/CameraHuman.h"
#include "../Graphics/OGL/Fonts/FontMgr.h"
#include "../Graphics/OGL/ISelectionProvider.h"
#include "../World/World.h"

class SceneGame : public Scene, private ISelectionProvider
{
  public:
    Camera *DefaultCamera;

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
        FOV.init(45.0f, AspectRatio, 0.1f, 1000.0f);
    }

    virtual bool ShellCommand(std::string &cmd, std::string &output);

    World          world;

  private:
    bool InitGL();
    void InitInputMgr();
    void SetLight(xLight &light, bool t_Ambient, bool t_Diffuse, bool t_Specular);

    long  accum;
    float stepAccum;

    CameraHuman    hCamera;

    xFieldOfView   FOV;

    virtual void RenderSelect(const xFieldOfView *FOV);
    virtual unsigned int CountSelectable();

    RigidObj *Select(int X, int Y)
    {
        std::vector<xDWORD> *objectIDs = ISelectionProvider::Select(&FOV, X, Y);
        return objectIDs == NULL ? NULL : (RigidObj*) world.objects[objectIDs->back()];
    }


};

#endif

