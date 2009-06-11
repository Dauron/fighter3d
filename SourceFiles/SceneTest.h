#ifndef __incl_SceneTest_h
#define __incl_SceneTest_h

#include "../AppFramework/IScene.h"
#include "../Math/Cameras/CameraFree.h"
#include "../Math/Cameras/CameraHuman.h"
#include "../Graphics/FontMgr.h"
#include "../Graphics/OGL/ISelectionProvider.h"

#include "../Graphics/OGL/GuiManager.h"
#include "../Graphics/OGL/GuiWindow.h"
#include "../Graphics/OGL/GuiButton.h"
#include "../Graphics/OGL/GuiCheckBox.h"
#include "../Graphics/OGL/GuiSlider.h"
#include "../Graphics/OGL/GuiLabel.h"

#include "../Math/Figures/xSphere.h"
#include "../Math/Figures/xCapsule.h"
#include "../Math/Figures/xBoxO.h"
#include "../Math/Figures/xCylinder.h"
#include "../Math/Figures/xMesh.h"

#include "../Physics/PhysicalFigure.h"
#include "../Physics/PhysicalWorld.h"

namespace Scenes {

    class SceneTest : public IScene, private ISelectionProvider
    {
    public:
        Math::Cameras::Camera *DefaultCamera;

        SceneTest() { Name="[Test]"; }

        virtual bool Create(int left, int top, unsigned int width, unsigned int height, IScene *scene = NULL);
        virtual void Destroy();

        virtual void Resize(int left, int top, unsigned int width, unsigned int height)
        {
            IScene::Resize(left, top, width, height);
            Camera.FOV.InitPerspective();
            Camera.FOV.InitViewport(left,top,width,height);

            m_cGuiManager.Resize(left, top, width, height);
        }

        virtual bool Update(float deltaTime);
        virtual bool Render();

        virtual bool ShellCommand(std::string &cmd, std::string &output);

    private:

        ::Graphics::OGL::CGuiManager m_cGuiManager;

        ::Graphics::OGL::CGuiLabel    *pcLabel;
        ::Graphics::OGL::CGuiButton   *pcBtn1;
        ::Graphics::OGL::CGuiButton   *pcBtn2;
        ::Graphics::OGL::CGuiSlider   *pcSlider;
        ::Graphics::OGL::CGuiCheckBox *pcCheckThisNot;
        ::Graphics::OGL::CGuiCheckBox *pcCheckThisOut;

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
        void InitInputMgr();

        Math::Cameras::CameraHuman Camera;

        virtual void RenderSelect(const Math::Cameras::FieldOfView &FOV);
        virtual unsigned int CountSelectable();
        xDWORD Select(int X, int Y);

    public:
        EVENT_HANDLER0(CheckThisOut_CheckedChanged, ::Graphics::OGL::CGuiControl, SceneTest);
    };

} // namespace Scenes

#endif
