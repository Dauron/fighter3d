#include "SceneSkeleton.h"

#include "../App Framework/Input/InputMgr.h"
#include "../Utils/Filesystem.h"
#include "../Graphics/OGL/GLShader.h"
#include "../Graphics/OGL/WorldRenderGL.h"

SceneSkeleton::SceneSkeleton(Scene *prevScene, const char *gr_modelName, const char *ph_modelName)
{
    PrevScene = prevScene;

    EditMode = emMain;

    State.CurrentAction   = 0;
    State.DisplayPhysical = false;
    State.HideBonesOnAnim = false;
    State.ShowBonesAlways = false;
    State.PlayAnimation   = false;

    Animation.Instance    = NULL;

    Selection.Bone        = NULL;
    Selection.Element     = NULL;
    Selection.ElementId   = xWORD_MAX;
    Selection.Vertices.clear();
    HoveredVert           = xDWORD_MAX;

    InputState.MouseLIsDown = false;
    InputState.MouseRIsDown = false;
    InputState.String.clear();
    KeyName_Modify = NULL;
    KeyName_Accept = NULL;

    sceneName = "[Skeleton]";
    Font      = HFont();
    Cameras.Current = NULL;

    Buttons.resize(emLast);
    Buttons[emMain].push_back(GLButton("Create Skeleton",  10, 2, 145, 15, IC_BE_ModeSkeletize));
    Buttons[emMain].push_back(GLButton("Edit BVH",        160, 2,  80, 15, IC_BE_ModeBVH));
    Buttons[emMain].push_back(GLButton("Skinning",        245, 2,  80, 15, IC_BE_ModeSkin));
    Buttons[emMain].push_back(GLButton("Animating",       330, 2,  90, 15, IC_BE_ModeAnimate));
    Buttons[emMain].push_back(GLButton("Graph/Phys",      425, 2, 100, 15, IC_BE_Select));
    Buttons[emMain].push_back(GLButton("Save",            530, 2,  45, 15, IC_BE_Save));

    Buttons[emCreateBone].push_back(GLButton("Select", 100, 2, 65, 15, IC_BE_Select, true, true));
    Buttons[emCreateBone].push_back(GLButton("Create", 170, 2, 65, 15, IC_BE_Create, true));
    Buttons[emCreateBone].push_back(GLButton("Move",   240, 2, 45, 15, IC_BE_Move,   true));
    Buttons[emCreateBone].push_back(GLButton("Delete", 290, 2, 65, 15, IC_BE_Delete));
    Buttons[emCreateBone].push_back(GLButton("Create constr", 360, 2, 130, 15, IC_BE_CreateConstr));
    Buttons[emCreateBone].push_back(GLButton("Delete constr", 495, 2, 130, 15, IC_BE_DeleteConstr, true));

    Buttons[emCreateConstraint_Type].push_back(GLButton("Max",    205, 2, 35, 15, IC_BE_CreateConstrMax));
    Buttons[emCreateConstraint_Type].push_back(GLButton("Min",    245, 2, 35, 15, IC_BE_CreateConstrMin));
    Buttons[emCreateConstraint_Type].push_back(GLButton("Const",  285, 2, 55, 15, IC_BE_CreateConstrEql));
    Buttons[emCreateConstraint_Type].push_back(GLButton("Ang",    345, 2, 35, 15, IC_BE_CreateConstrAng));
    Buttons[emCreateConstraint_Type].push_back(GLButton("Weight", 385, 2, 65, 15, IC_BE_CreateConstrWeight));

    Buttons[emEditBVH].push_back(GLButton("Create", 120, 2, 65, 15, IC_BE_Create));
    Buttons[emEditBVH].push_back(GLButton("Edit",   190, 2, 45, 15, IC_BE_Edit, true, true));
    Buttons[emEditBVH].push_back(GLButton("Clone",  240, 2, 55, 15, IC_BE_Clone, true));
    Buttons[emEditBVH].push_back(GLButton("Delete", 300, 2, 65, 15, IC_BE_Delete, true));

    Buttons[emCreateBVH].push_back(GLButton("Sphere",  120, 2, 65, 15, IC_BE_CreateSphere));
    Buttons[emCreateBVH].push_back(GLButton("Capsule", 190, 2, 75, 15, IC_BE_CreateCapsule));
    Buttons[emCreateBVH].push_back(GLButton("Box",     270, 2, 35, 15, IC_BE_CreateBox));

    Buttons[emSelectAnimation].push_back(GLButton("New",  110, 2, 35, 15, IC_BE_Create));
    Buttons[emSelectAnimation].push_back(GLButton("Load", 150, 2, 45, 15, IC_BE_Select));

    Buttons[emEditAnimation].push_back(GLButton("Play",      110, 2, 45, 15, IC_BE_Play));
    Buttons[emEditAnimation].push_back(GLButton("Insert KF", 160, 2, 90, 15, IC_BE_Create));
    Buttons[emEditAnimation].push_back(GLButton("Edit KF",   255, 2, 70, 15, IC_BE_Edit));
    Buttons[emEditAnimation].push_back(GLButton("KF Time",   330, 2, 70, 15, IC_BE_Move));
    Buttons[emEditAnimation].push_back(GLButton("Delete KF", 405, 2, 90, 15, IC_BE_Delete));
    Buttons[emEditAnimation].push_back(GLButton("Loop",      500, 2, 45, 15, IC_BE_Loop));
    Buttons[emEditAnimation].push_back(GLButton("Save",      550, 2, 45, 15, IC_BE_Save));

    Buttons[emAnimateBones].push_back(GLButton("Select",     110, 2, 65, 15, IC_BE_Select, true, true));
    Buttons[emAnimateBones].push_back(GLButton("Move",       180, 2, 45, 15, IC_BE_Move,   true));
    Buttons[emAnimateBones].push_back(GLButton("Reset Bone", 230, 2, 95, 15, IC_BE_Delete));
    Buttons[emAnimateBones].push_back(GLButton("Tgl.Bones",  330, 2, 90, 15, IC_BE_ModeSkeletize));
    Buttons[emAnimateBones].push_back(GLButton("Accept",     425, 2, 65, 15, IC_BE_Save));
    Buttons[emAnimateBones].push_back(GLButton("Reject",     495, 2, 65, 15, IC_Reject));

    Buttons[emLoadAnimation].push_back(GLButton("Reject",    110, 2, 65, 15, IC_Reject));

    Buttons[emSaveAnimation].push_back(GLButton("Accept",    110, 2, 65, 15, IC_Accept));
    Buttons[emSaveAnimation].push_back(GLButton("Reject",    180, 2, 65, 15, IC_Reject));

    Model.Initialize(gr_modelName, ph_modelName);

    xModel *modelGr = Model.ModelGr_Get().xModelP;
    if (Model.ModelPh)
    {
        xModel *modelPh = Model.ModelPh_Get().xModelP;

        if (modelPh->Spine.I_bones && !modelGr->Spine.I_bones)
        {
            modelGr->SkeletonAdd(); //   add skeleton to model
            RigidObj::CopySpine(modelPh->Spine, modelGr->Spine);
        }
        else
        if (!modelGr->Spine.I_bones && modelPh->Spine.I_bones)
        {
            modelPh->SkeletonAdd(); //   add skeleton to model
            RigidObj::CopySpine(modelGr->Spine, modelPh->Spine);
        }
        else
            RigidObj::CopySpine(modelPh->Spine, modelGr->Spine);
    }
    modelGr->Spine.ResetQ();
    Model.CalculateSkeleton();

    CurrentDirectory = Filesystem::GetFullPath("Data/models");
}
bool SceneSkeleton::Initialize(int left, int top, unsigned int width, unsigned int height)
{
    Scene::Initialize(left, top, width, height);
    InitInputMgr();
    g_InputMgr.mouseWheel = 0;
    if (!KeyName_Modify) {
        KeyName_Modify = strdup(g_InputMgr.GetKeyName(g_InputMgr.GetKeyCode(IC_BE_Modifier)).c_str());
        KeyName_Accept = strdup(g_InputMgr.GetKeyName(g_InputMgr.GetKeyCode(IC_Accept)).c_str());
    }
    InitCameras(!Cameras.Current);

    return true;
}
void SceneSkeleton::InitCameras(bool FL_reposition)
{
    if (!Cameras.Current)
    {
        Cameras.Current = &Cameras.Front;
        Cameras.Front.Init(0.0f, -5.0f, 1.7f, 0.0f, 0.0f, 1.7f, 0.0f, 0.0f, 1.0f);
        Cameras.Back.Init(0.0f, +5.0f, 1.7f, 0.0f, 0.0f, 1.7f, 0.0f, 0.0f, 1.0f);
        Cameras.Right.Init(-5.0f, 0.0f, 1.7f, 0.0f, 0.0f, 1.7f, 0.0f, 0.0f, 1.0f);
        Cameras.Left.Init(+5.0f, 0.0f, 1.7f, 0.0f, 0.0f, 1.7f, 0.0f, 0.0f, 1.0f);
        Cameras.Top.Init(0.0f, 0.0f,  5.0f, 0.0f, 0.0f, 1.7f, 0.0f, -1.0f, 0.0f);
        Cameras.Bottom.Init(0.0f, 0.0f, -5.0f, 0.0f, 0.0f, 1.7f, 0.0f, -1.0f, 0.0f);
        Cameras.Perspective.Init(-5.0f, -5.0f, 1.7f, 0.0f, 0.0f, 1.7f, 0.0f, 0.0f, 1.0f);
    }
    else
    if (FL_reposition)
    {
        if (Cameras.Current == &Cameras.Front)
            Cameras.Front.Init(0.0f, -5.0f, 1.7f, 0.0f, 0.0f, 1.7f, 0.0f, 0.0f, 1.0f);
        if (Cameras.Current == &Cameras.Right)
            Cameras.Right.Init(-5.0f, 0.0f, 1.7f, 0.0f, 0.0f, 1.7f, 0.0f, 0.0f, 1.0f);
        if (Cameras.Current == &Cameras.Back)
            Cameras.Back.Init(0.0f, +5.0f, 1.7f, 0.0f, 0.0f, 1.7f, 0.0f, 0.0f, 1.0f);
        if (Cameras.Current == &Cameras.Left)
            Cameras.Left.Init(+5.0f, 0.0f, 1.7f, 0.0f, 0.0f, 1.7f, 0.0f, 0.0f, 1.0f);
        if (Cameras.Current == &Cameras.Top)
            Cameras.Top.Init(0.0f, 0.0f,  5.0f, 0.0f, 0.0f, 1.7f, 0.0f, -1.0f, 0.0f);
        if (Cameras.Current == &Cameras.Bottom)
            Cameras.Bottom.Init(0.0f, 0.0f, -5.0f, 0.0f, 0.0f, 1.7f, 0.0f, -1.0f, 0.0f);
        if (Cameras.Current == &Cameras.Perspective)
            Cameras.Perspective.Init(-5.0f, -5.0f, 1.7f, 0.0f, 0.0f, 1.7f, 0.0f, 0.0f, 1.0f);
    }

    Cameras.Front.FOV.InitOrthogonal();
    Cameras.Back.FOV.InitOrthogonal();
    Cameras.Right.FOV.InitOrthogonal();
    Cameras.Left.FOV.InitOrthogonal();
    Cameras.Top.FOV.InitOrthogonal();
    Cameras.Bottom.FOV.InitOrthogonal();
    Cameras.Perspective.FOV.InitPerspective();

    Cameras.Front.FOV.InitViewport(Left,Top,Width,Height);
    Cameras.Back.FOV.InitViewport(Left,Top,Width,Height);
    Cameras.Right.FOV.InitViewport(Left,Top,Width,Height);
    Cameras.Left.FOV.InitViewport(Left,Top,Width,Height);
    Cameras.Top.FOV.InitViewport(Left,Top,Width,Height);
    Cameras.Bottom.FOV.InitViewport(Left,Top,Width,Height);
    Cameras.Perspective.FOV.InitViewport(Left,Top,Width,Height);
}
void SceneSkeleton::InitInputMgr()
{
    InputMgr &im = g_InputMgr;
    im.SetScene(sceneName);

    im.SetInputCodeIfKeyIsFree(VK_RETURN, IC_Accept);
    im.SetInputCodeIfKeyIsFree(VK_ESCAPE, IC_Reject);
    im.SetInputCodeIfKeyIsFree(VK_BACK,   IC_Con_BackSpace);
    im.SetInputCodeIfKeyIsFree(VK_F11,    IC_FullScreen);
#ifdef WIN32
    im.SetInputCodeIfKeyIsFree(VK_OEM_3,  IC_Console);
#else
    im.SetInputCodeIfKeyIsFree('`',       IC_Console);
#endif

    im.SetInputCodeIfKeyIsFree('C',       IC_CameraChange);
    im.SetInputCodeIfKeyIsFree(VK_TAB,    IC_CameraReset);
    im.SetInputCodeIfKeyIsFree('1',       IC_CameraFront);
    im.SetInputCodeIfKeyIsFree('2',       IC_CameraBack);
    im.SetInputCodeIfKeyIsFree('3',       IC_CameraLeft);
    im.SetInputCodeIfKeyIsFree('4',       IC_CameraRight);
    im.SetInputCodeIfKeyIsFree('5',       IC_CameraTop);
    im.SetInputCodeIfKeyIsFree('6',       IC_CameraBottom);
    im.SetInputCodeIfKeyIsFree('7',       IC_CameraPerspective);

    im.SetInputCodeIfKeyIsFree('V',       IC_PolyModeChange);
    im.SetInputCodeIfKeyIsFree('B',       IC_ShowBonesAlways);
    im.SetInputCodeIfKeyIsFree('F',       IC_ViewPhysicalModel);

    im.SetInputCodeIfKeyIsFree(VK_LBUTTON, IC_LClick);
    im.SetInputCodeIfKeyIsFree(VK_RBUTTON, IC_RClick);
    im.SetInputCodeIfKeyIsFree(VK_SHIFT,   IC_RunModifier);
    im.SetInputCodeIfKeyIsFree(VK_CONTROL, IC_BE_Modifier);
    im.SetInputCodeIfKeyIsFree(VK_DELETE,  IC_BE_Delete);
    im.SetInputCodeIfKeyIsFree('N', IC_BE_Select);
    im.SetInputCodeIfKeyIsFree('M', IC_BE_Move);
    im.SetInputCodeIfKeyIsFree('P', IC_BE_Play);

    im.SetInputCodeIfKeyIsFree(VK_PRIOR,  IC_MoveForward);
    im.SetInputCodeIfKeyIsFree(VK_NEXT,   IC_MoveBack);
    im.SetInputCodeIfKeyIsFree(VK_LEFT,   IC_MoveLeft);
    im.SetInputCodeIfKeyIsFree(VK_RIGHT,  IC_MoveRight);
    im.SetInputCodeIfKeyIsFree(VK_UP,     IC_MoveUp);
    im.SetInputCodeIfKeyIsFree(VK_DOWN,   IC_MoveDown);

    im.SetInputCodeIfKeyIsFree('W', IC_TurnUp);
    im.SetInputCodeIfKeyIsFree('S', IC_TurnDown);
    im.SetInputCodeIfKeyIsFree('D', IC_TurnLeft);
    im.SetInputCodeIfKeyIsFree('A', IC_TurnRight);
    im.SetInputCodeIfKeyIsFree('Q', IC_RollLeft);
    im.SetInputCodeIfKeyIsFree('E', IC_RollRight);

    im.SetInputCodeIfKeyIsFree('U', IC_OrbitUp);
    im.SetInputCodeIfKeyIsFree('J', IC_OrbitDown);
    im.SetInputCodeIfKeyIsFree('H', IC_OrbitLeft);
    im.SetInputCodeIfKeyIsFree('K', IC_OrbitRight);
}

bool SceneSkeleton::Invalidate()
{
    WorldRenderGL renderer;
    renderer.Invalidate(Model);
    return Scene::Invalidate();
}

SceneSkeleton::~SceneSkeleton()
{
    WorldRenderGL renderer;
    renderer.Free(Model);
    Model.Finalize();
}
void SceneSkeleton::Terminate()
{
    Cameras.Current = NULL;
    if (KeyName_Modify) {
        delete[] KeyName_Modify; KeyName_Modify = NULL;
        delete[] KeyName_Accept; KeyName_Accept = NULL;
    }
    if (EditMode == emEditAnimation || EditMode == emAnimateBones || EditMode == emFrameParams)
        if (Animation.Instance) {
            Animation.Instance->Unload();
            delete Animation.Instance;
            Animation.Instance = NULL;
        }

    g_FontMgr.DeleteFont(Font);
    Font = HFont();

    if (PrevScene) {
        PrevScene->Terminate();
        delete PrevScene;
        PrevScene = NULL;
    }

    Directories.clear();
}

#define fractf(a)    ((a)-floorf(a))

/************************** RENDER *************************************/
bool SceneSkeleton::FrameRender()
{
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); // Nice perspective calculations

    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE); // GL_FALSE = infinite viewpoint, GL_TRUE = locale viewpoint
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE); // GL_TRUE=two, GL_FALSE=one
    glDepthFunc(GL_LEQUAL);                 // Depth testing function

    glEnable(GL_CULL_FACE);                 // Do not draw hidden faces
    glCullFace (GL_BACK);                   // Hide back faces
    glFrontFace(GL_CCW);                    // Front faces are drawn in counter-clockwise direction

    glShadeModel(GL_SMOOTH);                // GL_SMOOTH - enable smooth shading, GL_FLAT - no gradient on faces
    glDisable (GL_POINT_SMOOTH);
    glDisable (GL_LINE_SMOOTH);
    glDisable (GL_POLYGON_SMOOTH);          // produces errors on many cards... use FSAA!
    glDisable(GL_LIGHT0); glDisable(GL_LIGHT1); glDisable(GL_LIGHT2); glDisable(GL_LIGHT3);
    glDisable(GL_LIGHT4); glDisable(GL_LIGHT5); glDisable(GL_LIGHT6); glDisable(GL_LIGHT7);

    // Clear surface
    glClearColor( 0.5f, 0.5f, 0.5f, 0.f );  // Background color
    glClearDepth( 100.0f );                 // Draw distance
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT/* | GL_STENCIL_BUFFER_BIT*/);

    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, Config::PolygonMode);

    glViewport(Cameras.Current->FOV.ViewportLeft, Cameras.Current->FOV.ViewportTop,
               Cameras.Current->FOV.ViewportWidth, Cameras.Current->FOV.ViewportHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(&Cameras.Current->FOV.MX_Projection_Get().x0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // lights at viewer position
    GLfloat light_global_amb_color[]  = { 0.2f, 0.2f, 0.2f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_global_amb_color);
    GLfloat light_position[]   = { 0.0f, 1.0f, 1.0f, 0.0f };
    GLfloat light_amb_color[]  = { 0.09f, 0.07f, 0.0f, 1.0f };
    GLfloat light_dif_color[]  = { 0.9f, 0.7f, 0.0f, 1.0f };
    GLfloat light_spec_color[] = { 0.9f, 0.7f, 0.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT,  light_amb_color);  // environment
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_dif_color);  // direct light
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_spec_color); // light on mirrors/metal
    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION,  1.0f);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.f);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.f);
    glEnable(GL_LIGHT0);
    GLShader::SetLightType(xLight_INFINITE);
    GLShader::EnableTexturing(xState_Enable);

    glMultMatrixf(&Cameras.Current->MX_WorldToView_Get().x0);

    WorldRenderGL wRender;
    RendererGL   &render = wRender.renderModel;

    if (Model.FL_renderNeedsUpdate) wRender.Free(Model);
    Model.FrameRender();

    xModel         &model         = *Model.ModelGr->xModelP;
    xModelInstance &modelInstance = Model.ModelGr->instance;

    render.RenderModel(model, modelInstance, false, Cameras.Current->FOV);
    render.RenderModel(model, modelInstance, true, Cameras.Current->FOV);
    GLShader::Suspend();

    GLShader::EnableTexturing(xState_Disable);
    GLShader::SetLightType(xLight_NONE);
    GLShader::Start();

    if (EditMode == emSelectVertex)
        render.RenderVertices(model, modelInstance, Renderer::smNone, Selection.ElementId, &Selection.Vertices);
    if (State.ShowBonesAlways || EditMode == emSelectBone || EditMode == emCreateBone ||
        EditMode == emCreateConstraint_Node || EditMode == emCreateConstraint_Params ||
        EditMode == emInputWght  || (EditMode == emAnimateBones && !State.HideBonesOnAnim) ||
        EditMode == emSelectBVHBone || EditMode == emEditVolume)
        render.RenderSkeleton(model, modelInstance, Selection.Bone ? Selection.Bone->ID : xWORD_MAX);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    if (EditMode == emAnimateBones)
    {
        glColor3f(0.2f, 0.5f, 0.2f);
        glBegin(GL_QUADS);
        {
            glVertex3f(Animation.Skeleton.Bounds.P_min.x, Animation.Skeleton.Bounds.P_max.y, Animation.Skeleton.Bounds.P_min.z);
            glVertex3f(Animation.Skeleton.Bounds.P_min.x, Animation.Skeleton.Bounds.P_min.y, Animation.Skeleton.Bounds.P_min.z);
            glVertex3f(Animation.Skeleton.Bounds.P_max.x, Animation.Skeleton.Bounds.P_min.y, Animation.Skeleton.Bounds.P_min.z);
            glVertex3f(Animation.Skeleton.Bounds.P_max.x, Animation.Skeleton.Bounds.P_max.y, Animation.Skeleton.Bounds.P_min.z);
        }
        glEnd();
        glBegin(GL_LINES);
        {
            glVertex3f(Animation.Skeleton.Bounds.P_min.x, Animation.Skeleton.Bounds.P_max.y, Animation.Skeleton.Bounds.P_min.z);
            glVertex3f(Animation.Skeleton.Bounds.P_min.x, Animation.Skeleton.Bounds.P_min.y, Animation.Skeleton.Bounds.P_min.z);
            glVertex3f(Animation.Skeleton.Bounds.P_max.x, Animation.Skeleton.Bounds.P_min.y, Animation.Skeleton.Bounds.P_min.z);
            glVertex3f(Animation.Skeleton.Bounds.P_max.x, Animation.Skeleton.Bounds.P_max.y, Animation.Skeleton.Bounds.P_min.z);
        }
        glEnd();
    }

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    if ((EditMode == emEditBVH || EditMode == emEditVolume) && model.BVHierarchy)
        render.RenderBVH(*model.BVHierarchy, xMatrix::Identity(), 0, Selection.BVHNodeID);

    GLShader::Suspend();

    glFlush();

    //////////////////////////// Overlay

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Set text output projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, Width, 0, Height, 0, 100);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (InputState.MouseLIsDown && EditMode == emSelectVertex)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_CULL_FACE);
        glColor4f(0.3f, 0.3f, 0.9f, 0.3f);
        glBegin(GL_QUADS);
        {
            int x = g_InputMgr.mouseX, y = g_InputMgr.mouseY;
            glVertex3i(Selection.RectStartX, Height - Selection.RectStartY, 0);
            glVertex2i(Selection.RectStartX, Height - y);
            glVertex2i(x, Height - y);
            glVertex2i(x, Height - Selection.RectStartY);
        }
        glEnd();
        glEnable(GL_CULL_FACE);
        glDisable(GL_BLEND);
    }

    if (!g_FontMgr.IsHandleValid(Font))
        Font = g_FontMgr.GetFont("Courier New", 15);
    const GLFont* pFont = g_FontMgr.GetFont(Font);

    glColor4f( 1.f, 1.f, 1.f, 1.f );

    const char* sCamera = "NULL";
    if (Cameras.Current == &Cameras.Front)
        sCamera = "Front";
    if (Cameras.Current == &Cameras.Right)
        sCamera = "Right";
    if (Cameras.Current == &Cameras.Back)
        sCamera = "Back";
    if (Cameras.Current == &Cameras.Left)
        sCamera = "Left";
    if (Cameras.Current == &Cameras.Top)
        sCamera = "Top";
    if (Cameras.Current == &Cameras.Bottom)
        sCamera = "Bottom";
    if (Cameras.Current == &Cameras.Perspective)
        sCamera = "Perspective";
    pFont->PrintF(5.f, Height - 20.f, 0.f, sCamera);

    if (EditMode == emCreateBone)
        pFont->PrintF(5.f, 5.f, 0.f, "Skeleton |");
    else if (EditMode == emCreateConstraint_Type)
        pFont->PrintF(5.f, 5.f, 0.f, "Skeleton constraints |");
    else if (EditMode == emCreateConstraint_Node)
    {
        if (Constraint.type == IC_BE_CreateConstrWeight)
            pFont->PrintF(5.f, 5.f, 0.f, "Skeleton constraints | select bone");
        else
        if (Constraint.type == IC_BE_CreateConstrAng)
            pFont->PrintF(5.f, 5.f, 0.f, "Skeleton constraints | select bone to constrain");
        else
        if (Constraint.boneA == xBYTE_MAX)
            pFont->PrintF(5.f, 5.f, 0.f, "Skeleton constraints | select first node");
        else
        if (Constraint.boneB == xBYTE_MAX)
            pFont->PrintF(5.f, 5.f, 0.f, "Skeleton constraints | select second node");
    }
    else if (EditMode == emCreateConstraint_Params)
    {
        if (Constraint.type == IC_BE_CreateConstrAng)
            for (int i = 0; i < 4; ++i)
            {
                const char *label = "";
                if (i == 0) label = "Max X";
                if (i == 1) label = "Min X";
                if (i == 2) label = "Max Y";
                if (i == 3) label = "Min Y";
                if (Constraint.step < i)
                    pFont->PrintF(5.f, 85.f-20.f*i, 0.f, "%s Angle: ?", label);
                else
                if (Constraint.step == i)
                    pFont->PrintF(5.f, 85.f-20.f*i, 0.f, "%s Angle: (%2.2f) %s", label, Constraint.angles[i], InputState.String.c_str());
                else
                    pFont->PrintF(5.f, 85.f-20.f*i, 0.f, "%s Angle: %2.2f", label, Constraint.angles[i]);
            }
        else
        if (Constraint.type == IC_BE_CreateConstrWeight)
            pFont->PrintF(5.f, 25, 0.f, "Weight: (%2.2f) %s", Constraint.M_weight, InputState.String.c_str());
        else
            pFont->PrintF(5.f, 25, 0.f, "Length: (%2.2f) %s", Constraint.S_length, InputState.String.c_str());
        pFont->PrintF(5.f, 5.f, 0.f, "Skeleton constraints | Input parameters of the constraint");
    }
    else if (EditMode == emEditBVH)
        pFont->PrintF(5.f, 5.f, 0.f, "BVH Editor |");
    else if (EditMode == emSelectBVHBone)
        pFont->PrintF(5.f, 5.f, 0.f, "BVH Editor | Click: Select Bone");
    else if (EditMode == emCreateBVH)
        pFont->PrintF(5.f, 5.f, 0.f, "BVH Editor |");
    else if (EditMode == emEditVolume)
        pFont->PrintF(5.f, 5.f, 0.f, "BVH Editor | Drag: Edit bounding volume");
    else if (EditMode == emSelectElement)
        pFont->PrintF(5.f, 5.f, 0.f, "Skinning | Click: Select Element");
    else if (EditMode == emSelectVertex)
    {
        pFont->PrintF(5.f, 5.f, 0.f,
            "Skinning | Drag: Select Vertices | %s+Drag: Unselect Vertices | %s: Assign bones to selected Vertices",
            KeyName_Modify, KeyName_Accept);
        pFont->PrintF(5.f, Height-40.f, 0.f, "%d vertices selected", Selection.Vertices.size());

        if (Selection.Element->FL_skeletized && HoveredVert != xDWORD_MAX)
        {
            size_t stride = Selection.Element->FL_textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel);
            xVertexSkel *vert = (xVertexSkel*)(((xBYTE*)Selection.Element->L_vertices) + stride * HoveredVert);
            for (int i=0; i < 4 && fractf(vert->bone[i]) != 0.f; ++i)
                pFont->PrintF(5.f, Height - 20.f * (i+3), 0.f, "Bone id%d : %d", (int)floorf(vert->bone[i]), (int)(fractf(vert->bone[i])*1000));
        }
    }
    else if (EditMode == emSelectBone)
        pFont->PrintF(5.f, 5.f, 0.f, "Skinning | Click: Select Bone");
    else if (EditMode == emInputWght)
        pFont->PrintF(5.f, 5.f, 0.f, "Skinning | Type in Bone weight | %s: Accept", KeyName_Accept);

    else if (EditMode == emSelectAnimation || EditMode == emAnimateBones
        || EditMode == emLoadAnimation || EditMode == emEditAnimation
		|| EditMode == emFrameParams || EditMode == emSaveAnimation)
	{
        pFont->PrintF(5.f, 5.f, 0.f, "Animation |");
		if (EditMode != emSelectAnimation && EditMode != emLoadAnimation)
			if (AnimationName.size())
				pFont->PrintF(150.f, Height - 20.f, 0.f, "Animation: %s", AnimationName.c_str());
			else
				pFont->PrintF(150.f, Height - 20.f, 0.f, "Animation: new animation");
	}

	if (EditMode == emEditAnimation)
        RenderProgressBar();
	else if (EditMode == emFrameParams) {
        if (Animation.KeyFrame.step == 1)
            pFont->PrintF(5.f, Height - 40.f, 0.f, "Freeze: (%d) %s", Animation.KeyFrame.freeze, InputState.String.c_str());
        else
        {
            pFont->PrintF(5.f, Height - 40.f, 0.f, "Freeze: %d", Animation.KeyFrame.freeze);
            pFont->PrintF(5.f, Height - 60.f, 0.f, "Duration: (%d) %s", Animation.KeyFrame.duration, InputState.String.c_str());
        }
    }
    else if (EditMode == emSaveAnimation)
    {
        if (AnimationName.size())
            pFont->PrintF(5.f, 25, 0.f, "Filename: (%s) %s", AnimationName.c_str(), InputState.String.c_str());
        else
            pFont->PrintF(5.f, 25, 0.f, "Filename: %s", InputState.String.c_str());
    }

    std::vector<GLButton>::iterator begin = Buttons[EditMode].begin();
    std::vector<GLButton>::iterator end   = Buttons[EditMode].end();
    for (; begin != end; ++begin)
        begin->Render(pFont);

    if (EditMode == emSaveAnimation || EditMode == emLoadAnimation)
    {
        begin = Directories.begin();
        end   = Directories.end();
        for (; begin != end; ++begin)
            begin->Render(pFont);
    }

    if (EditMode == emSelectBone || EditMode == emInputWght) {
        int i;
        int sum = 0;
        for (i=0; i < 4 && Skin.BoneWeight[i].weight != 0; ++i) {
            sum += Skin.BoneWeight[i].weight;
            pFont->PrintF(5.f, Height - 20.f * (i+2), 0.f, "Bone id%d : %d", Skin.BoneWeight[i].id, Skin.BoneWeight[i].weight);
        }
        if (EditMode == emInputWght)
            pFont->PrintF(5.f, Height - 20.f * (i+2), 0.f, "Bone id%d : (%d) %s", Selection.Bone->ID, 100 - sum, InputState.String.c_str());
    }

    glFlush();
    return true;
}
void SceneSkeleton::RenderProgressBar()
{
    unsigned int pWidth = Width / 2;
    unsigned int pHeight = 10;

    float x  = (float)Width - pWidth - 10;
    float x1, x2, x3 = x;
    float y1 = (float)Height - 10;
    float y2 = y1-pHeight;

    xAnimationInfo info = Animation.Instance->GetInfo();
    float scale = ((float)pWidth) / info.T_duration;

    xKeyFrame *frame = Animation.Instance->L_frames;
    xWORD cnt = Animation.Instance->I_frames;
    for(; frame && cnt; frame = frame->Next, --cnt)
    {
        x1 = x3;
        x2 = x1 + frame->T_freeze*scale;
        x3 = x2 + frame->T_duration*scale;

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glColor3f(0.4f, 0.4f, 1.f);
        glRectf(x1, y2, x2, y1);

        glColor3f(0.f, 0.7f, 0.f);
        glRectf(x2, y2, x3, y1);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glColor3f(0.f, 0.f, 0.f);
        glRectf(x1, y2, x3, y1);

    }

    glColor3f(1.f, 1.f, 1.f);
    glBegin(GL_LINES),
        glVertex2f (x+info.T_progress*scale, y1+2),
        glVertex2f (x+info.T_progress*scale, y2-3);
    glEnd();

    const GLFont* pFont = g_FontMgr.GetFont(Font);
    pFont->PrintF(5.f, Height-40.f, 0.f, "Frame: %d/%d | Progress: %d/%d (%d/%d)",
        info.I_frameNo, Animation.Instance->I_frames,
        info.T_progress, info.T_duration,
        Animation.Instance->T_progress,
        Animation.Instance->CurrentFrame->T_freeze+Animation.Instance->CurrentFrame->T_duration);
}

/************************** SELECTIONS *************************************/
xBYTE SceneSkeleton::GetBVH_Count (xBVHierarchy &bvhNode)
{
    xBYTE res = 1;

    for (int i = 0; i < bvhNode.I_items; ++i)
        res += GetBVH_Count(bvhNode.L_items[i]);

    return res;
}

xBYTE SceneSkeleton::GetBVH_ID (xBVHierarchy &bvhNode, xBVHierarchy *selected, xBYTE &ID)
{
    if (&bvhNode == selected) return ID;

    for (int i = 0; i < bvhNode.I_items; ++i)
    {
        xBYTE id = GetBVH_ID(bvhNode.L_items[i], selected, ++ID);
        if (id != xBYTE_MAX) return id;
    }
    return xBYTE_MAX;
}

xBVHierarchy *SceneSkeleton::GetBVH_byID (xBVHierarchy &bvhNode, xBYTE ID_selected, xBYTE &ID)
{
    if (ID == ID_selected) return &bvhNode;

    for (int i = 0; i < bvhNode.I_items; ++i)
    {
        xBVHierarchy *bvh = GetBVH_byID(bvhNode.L_items[i], ID_selected, ++ID);
        if (bvh) return bvh;
    }
    return NULL;
}

void SceneSkeleton::RenderSelect(const Math::Cameras::FieldOfView &FOV)
{
    xModel         &model         = *Model.ModelGr->xModelP;
    xModelInstance &modelInstance = Model.ModelGr->instance;
    RendererGL      render;

    if (EditMode == emCreateBone || EditMode == emCreateConstraint_Node ||
        EditMode == emSelectBone || EditMode == emAnimateBones ||
        EditMode == emSelectBVHBone)
        if (EditMode == emCreateBone && State.CurrentAction == IC_BE_DeleteConstr)
            render.RenderSkeletonSelection(model, modelInstance, true);
        else
            render.RenderSkeletonSelection(model, modelInstance, false);
    else
    if (EditMode == emEditBVH && model.BVHierarchy)
        render.RenderBVH(*model.BVHierarchy, xMatrix::Identity(), 0, 0, true);
    else
    if (EditMode == emSelectElement)
        render.RenderVertices(model, modelInstance, Renderer::smElement);
    else
    if (EditMode == emSelectVertex)
        render.RenderVertices(model, modelInstance, Renderer::smVertex, Selection.ElementId);
}
unsigned int SceneSkeleton::CountSelectable()
{
    xModel &model = *Model.ModelGr->xModelP;

    if (EditMode == emCreateBone || EditMode == emCreateConstraint_Node ||
        EditMode == emSelectBone || EditMode == emAnimateBones ||
        EditMode == emSelectBVHBone)
        if (EditMode == emCreateBone && State.CurrentAction == IC_BE_DeleteConstr)
            return model.Spine.I_constraints;
        else
            return model.Spine.I_bones;
    else
    if (EditMode == emEditBVH)
        if (model.BVHierarchy)
            return GetBVH_Count(*model.BVHierarchy);
        else
            return 0;
    else
    if (EditMode == emSelectElement)
            return model.I_elements;
    else
    if (EditMode == emSelectVertex)
        return Selection.Element->I_vertices;
    return 0;
}
std::vector<xDWORD> *SceneSkeleton::SelectCommon(int X, int Y, int W, int H)
{
    return ISelectionProvider::Select(*Cameras.Current, X, Height-Y, W, H);
}

xBone *SceneSkeleton::SelectBone(int X, int Y)
{
    std::vector<xDWORD> *sel = SelectCommon(X, Y);

    if (sel)
    {
        if (sel->size()) {
            GLuint id = sel->back();
            delete sel;
            return Model.ModelGr_Get().xModelP->Spine.L_bones + id;
        }
        delete sel;
    }
    return NULL;
}

xBVHierarchy *SceneSkeleton::SelectBVH(int X, int Y)
{
    std::vector<xDWORD> *sel = SelectCommon(X, Y);

    if (sel)
    {
        if (sel->size()) {
            GLuint id = sel->back();
            delete sel;
            if (Model.ModelGr_Get().xModelP->BVHierarchy)
            {
                xBYTE cid = 0;
                return GetBVH_byID(*Model.ModelGr_Get().xModelP->BVHierarchy, id, cid);
            }
            return NULL;
        }
        delete sel;
    }
    return NULL;
}

xWORD SceneSkeleton::SelectElement(int X, int Y)
{
    std::vector<xDWORD> *sel = SelectCommon(X, Y);

    if (sel)
    {
        if (sel->size()) {
            GLuint id = sel->back();
            delete sel;
            return (xWORD)id;
        }
        delete sel;
    }
    return xWORD_MAX;
}
