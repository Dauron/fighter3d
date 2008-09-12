#ifndef __incl_SceneSkeleton_h
#define __incl_SceneSkeleton_h

#include "../AppFramework/IScene.h"

#include "../Graphics/OGL/ISelectionProvider.h"
#include "../Graphics/OGL/Button.h"
#include "../Graphics/FontMgr.h"

#include "../Math/Cameras/CameraHuman.h"
#include "../Math/Cameras/CameraFree.h"

#include "../World/RigidObj.h"
#include "../Models/lib3dx/xAnimation.h"

#include <string>

namespace Scenes {

    class SceneSkeleton : public IScene, public ISelectionProvider
    {
      public:
        SceneSkeleton(const char *gr_modelName, const char *ph_modelName);

        virtual bool Create(int left, int top, unsigned int width, unsigned int height, IScene *scene = NULL);
        virtual void Destroy();

        virtual void Enter()
        { IScene::Enter(); g_InputMgr.mouseWheel = 0; InputState.MouseLIsDown = false; InputState.MouseRIsDown = false; }

        virtual bool Invalidate();
        virtual void Resize(int left, int top, unsigned int width, unsigned int height);

        virtual bool Update(float deltaTime);
        virtual bool Render();

      private:
        void InitInputMgr();
        void InitCameras(bool FL_reposition = false);

        struct _Cameras
        {
            Math::Cameras::CameraHuman Front, Back, Left, Right, Perspective;
            Math::Cameras::CameraFree  Top, Bottom;
            Math::Cameras::Camera *Current;
        } Cameras;

        enum
        {
            emMain, emCreateBone,
            emCreateConstraint_Type, emCreateConstraint_Node, emCreateConstraint_Params,
            emEditBVH, emSelectBVHBone, emCreateBVH, emEditVolume,
            emSelectElement, emSelectVertex, emSelectBone, emInputWght,
            emSelectAnimation, emEditAnimation, emAnimateBones, emFrameParams,
            emLoadAnimation, emSaveAnimation, emSaveModel,
            emLast
        } EditMode;

        HFont       Font;
        RigidObj    Model;

        VerletSystem vSystem;
        VerletSolver vEngine;

        char                              * KeyName_Accept;
        char                              * KeyName_Modify;
        std::vector<std::vector<Graphics::OGL::Button> > Buttons;         // Buttons[EditMode][ButtonNo]
        std::vector<Graphics::OGL::Button>               Directories;
        std::string                         CurrentDirectory;
        std::string                         AnimationName;

        // Input states and key buffer
        struct _InputState {
            int         LastX, LastY;
            bool        MouseLIsDown;
            bool        MouseRIsDown;
            std::string String;
        } InputState;

        // Scene state properties
        struct _State {
            bool        DisplayPhysical;
            bool        HideBonesOnAnim;
            bool        ShowBonesAlways;
            bool        PlayAnimation;
            int         CurrentAction;
        } State;

        // Specific Edit Mode properties
    #ifdef WIN32
        union { // Save some memory... not much, but always :)
    #endif
            // Constraint
            struct _Constraint {
                xBYTE  boneA;
                xBYTE  boneB;
                xLONG  type;
                union {
                    xFLOAT M_weight;
                    xFLOAT S_length;
                    struct {
                        xFLOAT maxX;
                        xFLOAT minX;
                        xFLOAT maxY;
                        xFLOAT minY;
                        xBYTE  step;
                    };
                    xFLOAT4    angles;
                };
            } Constraint;
            // BVH
            struct _BVH {
                xPoint3 P_prevMouse;
            } BVH;
            // Skinning
            struct _Skin {
                struct {
                    xBYTE id;
                    xBYTE weight;
                } BoneWeight[4];
            } Skin;
            // Animation
            struct _Animation {
                union {
                    // Key Frame parameters
                    struct {
                        xBYTE step;
                        xLONG freeze;
                        xLONG duration;
                    } KeyFrame;
                    // Bone rotation
                    struct {
                        Math::Figures::xBoxA Bounds;
                        xQuaternion          PreviousQuaternion;
                    } Skeleton;
                };
                xAnimation * Instance;
            } Animation;
            // Save model
            struct _SaveModel {
                bool FL_save_physical;
            } SaveModel;
    #ifdef WIN32
        };
    #endif

        // Selected objects
        struct _Selection {
            xBone             * Bone;
            xBVHierarchy      * BVHNode;
            xBYTE               BVHNodeID;
            xBYTE               FigureDim;
            xElement          * Element;
            xWORD               ElementId;
            std::vector<xDWORD> Vertices;

            int                 RectStartX, RectStartY;
        } Selection;
        xDWORD          HoveredVert;

        void          RenderProgressBar();

        void          SwitchDisplayedModel();

        /* BVH */
        xBYTE         GetBVH_Count (xBVHierarchy &bvhNode);
        xBYTE         GetBVH_ID (xBVHierarchy &bvhNode, xBVHierarchy *selected, xBYTE &ID);
        xBVHierarchy *GetBVH_byID (xBVHierarchy &bvhNode, xBYTE ID_selected, xBYTE &ID);
        void          UpdateCustomBVH();
        void          UpdateBBox();

        /* INPUT & CAMERAS */
        bool        UpdateButton(Graphics::OGL::Button &button);
        void        UpdateDisplay(float deltaTime);
        /* MOUSE */
        void        UpdateMouse(float deltaTime);
        void        MouseLDown (int X, int Y);
        void        MouseLUp   (int X, int Y);
        void        MouseRDown (int X, int Y);
        void        MouseRUp   (int X, int Y);
        void        MouseMove  (int X, int Y);
        void        MouseLDown_BVH(int X, int Y);
        void        MouseLMove_BVH(int X, int Y);
        /* 3D */
        xVector3    CastPoint (xVector3 rayPos, xVector3 planeP);
        /* TEXT INPUT */
        void        GetCommand();
        void        GetConstraintParams();
        void        GetBoneIdAndWeight();
        void        GetFrameParams();
        /* FILES */
        void        FillDirectoryBtns(bool files = false, const char *mask = NULL);

        /* SELECT */
        virtual void         RenderSelect   (const Math::Cameras::FieldOfView &FOV);
        virtual unsigned int CountSelectable();
        std::vector<xDWORD> *SelectCommon  (int X, int Y, int W = 1, int H = 1);
        xBone               *SelectBone    (int X, int Y);
        xBVHierarchy        *SelectBVH     (int X, int Y);
        xWORD                SelectElement (int X, int Y);


    };

} // namespace Scenes

#endif
