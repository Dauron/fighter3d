#ifndef __incl_SceneGame_h
#define __incl_SceneGame_h

#include "../App Framework/Scene.h"
#include "../Math/Cameras/CameraHuman.h"
#include "../World/World.h"
#include "../OpenGL/GLShader.h"
#include "../OpenGL/Fonts/FontMgr.h"

class SceneGame : public Scene
{
  public:
    Camera *DefaultCamera;
    
    SceneGame() : DefaultCamera(NULL) { sceneName="[Game]"; shader.Load("Data/program.vert", "Data/program.frag"); };
    
    virtual bool Initialize(int left, int top, unsigned int width, unsigned int height);
    virtual bool Invalidate() {
        shader.Invalidate(); return true;
    }
    virtual void Terminate();
    virtual bool Update(float deltaTime);
    virtual bool Render();

  private:
    bool InitGL();
    void InitInputMgr();

    long  accum;
    float stepAccum;

    CameraHuman  hCamera;
    World          world;
    
    GLShader       shader;

    HFont          m_Font1;
    HFont          m_Font2;
};

#endif

