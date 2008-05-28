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
    virtual bool Update(float deltaTime);
    virtual bool Render();

    virtual void Resize(int left, int top, unsigned int width, unsigned int height)
    {
        Scene::Resize(left, top, width, height);
        FOV.init(45.0f, AspectRatio, 0.1f, 1000.0f);
    }

    World          world;

  private:
    bool InitGL();
    void InitInputMgr();
    void SetLight(xLight &light, bool t_Ambient, bool t_Diffuse, bool t_Specular);

    long  accum;
    float stepAccum;

    CameraHuman    hCamera;

    HFont          m_Font1;
    HFont          m_Font2;

    xFieldOfView   FOV;

    virtual void RenderSelect(const xFieldOfView *FOV);
    virtual unsigned int CountSelectable();

    RigidObj *Select(int X, int Y)
    {
        std::vector<xDWORD> *objectIDs = ISelectionProvider::Select(&FOV, X, Y);
        return objectIDs == NULL ? NULL : world.objects[objectIDs->back()];
    }


};

#endif

