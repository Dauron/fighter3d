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

#include "../Physics/PhysicalFigure.h"

class SceneTest : public Scene, private ISelectionProvider
{
  public:
    Camera *DefaultCamera;

    SceneTest() : DefaultCamera(NULL)
        , pf_sphere1(new ::Math::Figures::xSphere())
        , pf_sphere2(new ::Math::Figures::xSphere())
        , pf_capsule1(new ::Math::Figures::xCapsule())
        , pf_capsule2(new ::Math::Figures::xCapsule())
        , pf_cube1(new ::Math::Figures::xBoxO())
        , pf_cube2(new ::Math::Figures::xBoxO())
        , pf_mesh1(new ::Math::Figures::xMesh())
        , pf_mesh2(new ::Math::Figures::xMesh())
    { sceneName="[Test]"; };

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

    ::Physics::PhysicalFigure  pf_sphere1;
    ::Physics::PhysicalFigure  pf_sphere2;
    ::Physics::PhysicalFigure  pf_capsule1;
    ::Physics::PhysicalFigure  pf_capsule2;
    ::Physics::PhysicalFigure  pf_cube1;
    ::Physics::PhysicalFigure  pf_cube2;
    ::Physics::PhysicalFigure  pf_mesh1;
    ::Physics::PhysicalFigure  pf_mesh2;
    ::Physics::PhysicalFigure *figures[10][2];

    xDWORD  selected;
    xDWORD  selectedSub;

    bool     FL_pause;
    bool     FL_mouse_down;
    xVector3 P_prevMouse;
    xVector3 P_firstMouse;
    xFLOAT   T_total;

    void InitObjects();
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

