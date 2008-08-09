#ifndef __incl_SceneTest_h
#define __incl_SceneTest_h

#include "../AppFramework/Scene.h"
#include "../Math/Cameras/CameraFree.h"
#include "../Math/Cameras/CameraHuman.h"
#include "../Graphics/OGL/Fonts/FontMgr.h"
#include "../Graphics/OGL/ISelectionProvider.h"

#include "../Math/Figures/xSphere.h"
#include "../Math/Figures/xCapsule.h"
#include "../Math/Figures/xBoxO.h"
#include "../Math/Figures/xCylinder.h"
#include "../Math/Figures/xMesh.h"

class SceneTest : public Scene, private ISelectionProvider
{
  public:
    Camera *DefaultCamera;

    SceneTest() : DefaultCamera(NULL) { sceneName="[Test]"; };

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

    ::Math::Figures::xIFigure3d *figures[7][2];
    ::Math::Figures::xSphere     sphere1;
    ::Math::Figures::xSphere     sphere2;
    ::Math::Figures::xCapsule    capsule1;
    ::Math::Figures::xCapsule    capsule2;
    ::Math::Figures::xBoxO       cube1;
    ::Math::Figures::xBoxO       cube2;
    ::Math::Figures::xMesh       mesh1;

    xDWORD  selected;
    xDWORD  selectedSub;

    bool     FL_mouse_down;
    xVector3 P_prevMouse;

    bool InitGL();
    void InitInputMgr();

    CameraHuman  fCamera;
    xFieldOfView FOV;

    virtual void RenderSelect(const xFieldOfView *FOV);
    virtual unsigned int CountSelectable();

    xVector3 Get3dPos(int X, int Y, xVector3 P_plane);

    xDWORD Select(int X, int Y)
    {
        std::vector<xDWORD> *objectIDs = ISelectionProvider::Select(&FOV, X, Y);
        if (objectIDs == NULL) return -1;
        xDWORD res = objectIDs->back();
        delete objectIDs;
        return res;
    }
};

#endif

