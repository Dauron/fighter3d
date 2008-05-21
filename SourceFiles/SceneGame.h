#ifndef __incl_SceneGame_h
#define __incl_SceneGame_h

#include "../AppFramework/Scene.h"
#include "../Math/Cameras/CameraHuman.h"
#include "../World/World.h"
#include "../OpenGL/GLShader.h"
#include "../OpenGL/Fonts/FontMgr.h"
#include "../Math/xFieldOfView.h"

class SceneGame : public Scene
{
  public:
    Camera *DefaultCamera;

    SceneGame() : DefaultCamera(NULL) { sceneName="[Game]"; shader.Load("Data/program.vert", "Data/program.frag"); };

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

  private:
    bool InitGL();
    void InitInputMgr();
    void SetLight(xLight &light);

    long  accum;
    float stepAccum;

    CameraHuman    hCamera;
    World          world;

    GLShader       shader;

    HFont          m_Font1;
    HFont          m_Font2;

    xFieldOfView   FOV;
};

#endif

