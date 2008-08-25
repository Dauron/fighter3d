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
#include "../Physics/PhysicalWorld.h"

class SceneTest : public Scene, private ISelectionProvider
{
public:
    Math::Cameras::Camera *DefaultCamera;

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
    virtual bool FrameUpdate(float deltaTime);
    virtual bool FrameRender();

    virtual bool ShellCommand(std::string &cmd, std::string &output);

    virtual void Resize(int left, int top, unsigned int width, unsigned int height)
    {
        Scene::Resize(left, top, width, height);
        Camera.FOV.InitPerspective();
        Camera.FOV.InitViewport(left,top,width,height);
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
    ::Physics::PhysicalWorld::Vec_Object figures[10];

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

    Math::Cameras::CameraHuman Camera;

    virtual void RenderSelect(const Math::Cameras::FieldOfView &FOV);
    virtual unsigned int CountSelectable();
    xDWORD Select(int X, int Y);
};

#endif
