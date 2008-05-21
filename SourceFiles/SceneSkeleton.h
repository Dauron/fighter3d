#ifndef __incl_SceneSkeleton_h
#define __incl_SceneSkeleton_h

#include <string>
#include "../AppFramework/Scene.h"
#include "../OpenGL/ISelectionProvider.h"

#include "../OpenGL/Fonts/FontMgr.h"
#include "../World/ModelObj.h"
#include "../Math/Cameras/CameraHuman.h"
#include "../Models/lib3dx/xSkeleton.h"

#include "../AppFramework/Input/InputCodes.h"
#include "../OpenGL/GLButton.h"
#include "../Models/lib3dx/xAnimation.h"
#include "../Utils/Filesystem.h"

class SceneSkeleton : public Scene, public ISelectionProvider
{
  public:
    SceneSkeleton(Scene *prevScene, const char *gr_modelName, const char *ph_modelName);
    ~SceneSkeleton() { m_Model.Finalize(); }

    virtual bool Initialize(int left, int top, unsigned int width, unsigned int height);
    virtual bool Invalidate() {
        m_Model.Invalidate();
        return m_PrevScene->Invalidate();
    }
    virtual void Terminate();
    virtual bool Update(float deltaTime);
    virtual bool Render();

  private:
    bool        InitGL();
    void        InitInputMgr();

    struct
    {
        CameraHuman Front, Back, Top, Bottom, Left, Right, Perspective;
        Camera *Current;
    } m_Cameras;

    enum
    {
        emMain, emCreateBone,
        emSelectElement, emSelectVertex, emSelectBone, emInputWght,
        emSelectAnimation, emEditAnimation, emAnimateBones, emFrameParams,
        emLoadAnimation, emSaveAnimation, emLast
    } m_EditMode;

    Scene     * m_PrevScene;
    HFont       m_Font;
    ModelObj    m_Model;
    bool        m_EditGraphical;
    char      * modifyButton, * acceptButton;

    std::vector<std::vector<GLButton> > m_Buttons;
    std::vector<GLButton>               m_Directories;
    std::string                         m_CurrentDirectory;
    std::string                         m_AnimationName;

    int                  lastX, lastY;
    bool                 mouseLIsDown, mouseRIsDown;
    bool                 showBonesOnAnim;
    bool                 play;
    int                  currentAction;
    xBYTE                boneIds[4];
    xBYTE                boneWghts[4];
    xLONG                frameParams[2];
    xBYTE                param;
    xBone               *selectedBone;
    xElement            *selectedElement;
    xWORD                selectedElemID;
    std::vector<xDWORD>  selectedVert;
    xDWORD               hoveredVert;
    int                  selStartX, selStartY;
    xVector4             lastBoneQuaternion;
    std::string          command;
    xAnimation          *currentAnimation;

    void        RenderProgressBar();

    /* INPUT & CAMERAS */
    bool        UpdateButton(GLButton &button);
    void        UpdateDisplay(float deltaTime);
    /* MOUSE */
    void        UpdateMouse(float deltaTime);
    void        MouseLDown(int X, int Y);
    void        MouseLUp  (int X, int Y);
    void        MouseRDown(int X, int Y);
    void        MouseRUp  (int X, int Y);
    void        MouseMove (int X, int Y);
    /* 3D */
    xVector3    Get3dPos  (int X, int Y, xVector3 planeP);
    xVector3    CastPoint(xVector3 rayPos, xVector3 planeP);
    /* TEXT INPUT */
    void        GetCommand();
    void        GetInputWeight();
    void        SetVertexWghts();
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

