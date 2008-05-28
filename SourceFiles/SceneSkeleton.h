#ifndef __incl_SceneSkeleton_h
#define __incl_SceneSkeleton_h

#include "../AppFramework/Scene.h"

#include "../OGL/ISelectionProvider.h"
#include "../OGL/GLButton.h"
#include "../OGL/Fonts/FontMgr.h"

#include "../Math/Cameras/CameraHuman.h"
#include "../Math/Cameras/CameraFree.h"

#include "../World/ModelObj.h"
#include "../Models/lib3dx/xAnimation.h"

#include <string>

class SceneSkeleton : public Scene, public ISelectionProvider
{
  public:
    SceneSkeleton(Scene *prevScene, const char *gr_modelName, const char *ph_modelName);
    ~SceneSkeleton() { Model.Finalize(); }

    virtual bool Initialize(int left, int top, unsigned int width, unsigned int height);
    virtual bool Invalidate() {
        Model.Invalidate();
        return PrevScene->Invalidate();
    }
    virtual void Terminate();
    virtual bool Update(float deltaTime);
    virtual bool Render();

  private:
    void         InitInputMgr();

    struct _Cameras
    {
        CameraHuman Front, Back, Left, Right, Perspective;
        CameraFree  Top, Bottom;
        Camera *Current;
    } Cameras;

    enum
    {
        emMain, emCreateBone,
        emCreateConstraint_Type, emCreateConstraint_Node, emCreateConstraint_Params,
        emSelectElement, emSelectVertex, emSelectBone, emInputWght,
        emSelectAnimation, emEditAnimation, emAnimateBones, emFrameParams,
        emLoadAnimation, emSaveAnimation, emLast
    } EditMode;

    Scene     * PrevScene;
    HFont       Font;
    ModelObj    Model;

    xVerletSystem vSystem;
    xVerletSolver vEngine;

    char                              * KeyName_Accept;
    char                              * KeyName_Modify;
    std::vector<std::vector<GLButton> > Buttons;
    std::vector<GLButton>               Directories;
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
    union {
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
                xVector4 PreviousQuaternion;
            };
            xAnimation * Instance;
        } Animation;
    };

    // Selected objects
    struct _Selection {
        xBone             * Bone;
        xElement          * Element;
        xWORD               ElementId;
        std::vector<xDWORD> Vertices;

        int                 RectStartX, RectStartY;
    } Selection;
    xDWORD          HoveredVert;

    void        RenderProgressBar();

    /* INPUT & CAMERAS */
    bool        UpdateButton(GLButton &button);
    void        UpdateDisplay(float deltaTime);
    /* MOUSE */
    void        UpdateMouse(float deltaTime);
    void        MouseLDown (int X, int Y);
    void        MouseLUp   (int X, int Y);
    void        MouseRDown (int X, int Y);
    void        MouseRUp   (int X, int Y);
    void        MouseMove  (int X, int Y);
    /* 3D */
    xVector3    Get3dPos  (int X, int Y, xVector3 planeP);
    xVector3    CastPoint (xVector3 rayPos, xVector3 planeP);
    /* TEXT INPUT */
    void        GetCommand();
    void        GetConstraintParams();
    void        GetBoneIdAndWeight();
    void        GetFrameParams();
    /* FILES */
    void        FillDirectoryBtns(bool files = false, const char *mask = NULL);

    /* SELECT */
    virtual void         RenderSelect   (const xFieldOfView *FOV);
    virtual unsigned int CountSelectable();
    std::vector<xDWORD> *SelectCommon  (int X, int Y, int W = 1, int H = 1);
    xBone               *SelectBone    (int X, int Y);
    xWORD                SelectElement (int X, int Y);

};

#endif

