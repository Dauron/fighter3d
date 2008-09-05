#include "SceneSkeleton.h"

#include "../AppFramework/Input/InputMgr.h"
#include "../Utils/Filesystem.h"
#include "../Graphics/OGL/GLShader.h"
#include "../Graphics/OGL/WorldRenderGL.h"

using namespace Scenes;

SceneSkeleton::SceneSkeleton(const char *gr_modelName, const char *ph_modelName)
{
    Name            = "[Skeleton]";

    Model.modelFile     = gr_modelName ? gr_modelName : "";
    Model.fastModelFile = ph_modelName ? ph_modelName : "";
}

bool SceneSkeleton::Create(int left, int top, unsigned int width, unsigned int height, Scene *scene)
{
    if (!Model.modelFile.size()) return false;

    Scene::Create(left, top, width, height, scene);

    InitInputMgr();

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
    InputState.String.clear();

    KeyName_Modify  = strdup(g_InputMgr.GetKeyName(g_InputMgr.Input2KeyCode(IC_BE_Modifier)).c_str());
    KeyName_Accept  = strdup(g_InputMgr.GetKeyName(g_InputMgr.Input2KeyCode(IC_Accept)).c_str());
    Font            = g_FontMgr.GetFont("Courier New", 15);
    const GLFont* pFont = g_FontMgr.GetFont(Font);

    CurrentDirectory = Filesystem::GetFullPath("Data/models");

    // Init buttons
    Buttons.resize(emLast);
    std::vector<GLButton> &menu1 = Buttons[emMain];
    menu1.push_back(GLButton("Create Skeleton", 10,              Height-20.f, pFont, IC_BE_ModeSkeletize));
    menu1.push_back(GLButton("Edit BVH",        menu1[0].X2 + 5, Height-20.f, pFont, IC_BE_ModeBVH));
    menu1.push_back(GLButton("Skinning",        menu1[1].X2 + 5, Height-20.f, pFont, IC_BE_ModeSkin));
    menu1.push_back(GLButton("Animating",       menu1[2].X2 + 5, Height-20.f, pFont, IC_BE_ModeAnimate));
    menu1.push_back(GLButton("Graph/Phys",      menu1[3].X2 + 5, Height-20.f, pFont, IC_BE_Select));
    menu1.push_back(GLButton("Save",            menu1[4].X2 + 5, Height-20.f, pFont, IC_BE_Save));

    std::vector<GLButton> &menu2 = Buttons[emCreateBone];
    menu2.push_back(GLButton("Select",        100,             Height-20.f, pFont, IC_BE_Select, true, true));
    menu2.push_back(GLButton("Create",        menu2[0].X2 + 5, Height-20.f, pFont, IC_BE_Create, true));
    menu2.push_back(GLButton("Move",          menu2[1].X2 + 5, Height-20.f, pFont, IC_BE_Move,   true));
    menu2.push_back(GLButton("Delete",        menu2[2].X2 + 5, Height-20.f, pFont, IC_BE_Delete));
    menu2.push_back(GLButton("Create constr", menu2[3].X2 + 5, Height-20.f, pFont, IC_BE_CreateConstr));
    menu2.push_back(GLButton("Delete constr", menu2[4].X2 + 5, Height-20.f, pFont, IC_BE_DeleteConstr, true));

    std::vector<GLButton> &menu3 = Buttons[emCreateConstraint_Type];
    menu3.push_back(GLButton("Max",    205,             Height-20.f, pFont, IC_BE_CreateConstrMax));
    menu3.push_back(GLButton("Min",    menu3[0].X2 + 5, Height-20.f, pFont, IC_BE_CreateConstrMin));
    menu3.push_back(GLButton("Const",  menu3[1].X2 + 5, Height-20.f, pFont, IC_BE_CreateConstrEql));
    menu3.push_back(GLButton("Ang",    menu3[2].X2 + 5, Height-20.f, pFont, IC_BE_CreateConstrAng));
    menu3.push_back(GLButton("Weight", menu3[3].X2 + 5, Height-20.f, pFont, IC_BE_CreateConstrWeight));

    std::vector<GLButton> &menu4 = Buttons[emEditBVH];
    menu4.push_back(GLButton("Create", 120,             Height-20.f, pFont, IC_BE_Create));
    menu4.push_back(GLButton("Edit",   menu4[0].X2 + 5, Height-20.f, pFont, IC_BE_Edit, true, true));
    menu4.push_back(GLButton("Clone",  menu4[1].X2 + 5, Height-20.f, pFont, IC_BE_Clone, true));
    menu4.push_back(GLButton("Delete", menu4[2].X2 + 5, Height-20.f, pFont, IC_BE_Delete, true));

    std::vector<GLButton> &menu5 = Buttons[emCreateBVH];
    menu5.push_back(GLButton("Sphere",  120,             Height-20.f, pFont, IC_BE_CreateSphere));
    menu5.push_back(GLButton("Capsule", menu5[0].X2 + 5, Height-20.f, pFont, IC_BE_CreateCapsule));
    menu5.push_back(GLButton("Box",     menu5[1].X2 + 5, Height-20.f, pFont, IC_BE_CreateBox));

    std::vector<GLButton> &menu6 = Buttons[emSelectAnimation];
    menu6.push_back(GLButton("New",  110,             Height-20.f, pFont, IC_BE_Create));
    menu6.push_back(GLButton("Load", menu6[0].X2 + 5, Height-20.f, pFont, IC_BE_Select));

    std::vector<GLButton> &menu7 = Buttons[emEditAnimation];
    menu7.push_back(GLButton("Play",      110,             Height-20.f, pFont, IC_BE_Play));
    menu7.push_back(GLButton("Insert KF", menu7[0].X2 + 5, Height-20.f, pFont, IC_BE_Create));
    menu7.push_back(GLButton("Edit KF",   menu7[1].X2 + 5, Height-20.f, pFont, IC_BE_Edit));
    menu7.push_back(GLButton("KF Time",   menu7[2].X2 + 5, Height-20.f, pFont, IC_BE_Move));
    menu7.push_back(GLButton("Delete KF", menu7[3].X2 + 5, Height-20.f, pFont, IC_BE_Delete));
    menu7.push_back(GLButton("Loop",      menu7[4].X2 + 5, Height-20.f, pFont, IC_BE_Loop));
    menu7.push_back(GLButton("Save",      menu7[5].X2 + 5, Height-20.f, pFont, IC_BE_Save));

    std::vector<GLButton> &menu8 = Buttons[emAnimateBones];
    menu8.push_back(GLButton("Select",     110,             Height-20.f, pFont, IC_BE_Select, true, true));
    menu8.push_back(GLButton("Move",       menu8[0].X2 + 5, Height-20.f, pFont, IC_BE_Move,   true));
    menu8.push_back(GLButton("Reset Bone", menu8[1].X2 + 5, Height-20.f, pFont, IC_BE_Delete));
    menu8.push_back(GLButton("Tgl.Bones",  menu8[2].X2 + 5, Height-20.f, pFont, IC_BE_ModeSkeletize));
    menu8.push_back(GLButton("Accept",     menu8[3].X2 + 5, Height-20.f, pFont, IC_BE_Save));
    menu8.push_back(GLButton("Reject",     menu8[4].X2 + 5, Height-20.f, pFont, IC_Reject));

    Buttons[emLoadAnimation].push_back(GLButton("Reject", 110, Height-20.f, pFont, IC_Reject));

    std::vector<GLButton> &menu9 = Buttons[emSaveAnimation];
    menu9.push_back(GLButton("Accept", 110,             Height-20.f, pFont, IC_Accept));
    menu9.push_back(GLButton("Reject", menu9[0].X2 + 5, Height-20.f, pFont, IC_Reject));

    std::vector<GLButton> &menu10 = Buttons[emSaveModel];
    menu10.push_back(GLButton("Accept", 110,              Height-20.f, pFont, IC_Accept));
    menu10.push_back(GLButton("Reject", menu10[0].X2 + 5, Height-20.f, pFont, IC_Reject));

    // Init cameras
    Cameras.Current = NULL;
    InitCameras(false);

    // Init model
    Model.Create(Model.modelFile.c_str(), Model.fastModelFile.size() ? Model.fastModelFile.c_str() : NULL);
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
    im.SetScene(Name);

    im.Key2InputCode_SetIfKeyFree(VK_F11,    IC_FullScreen);
    im.Key2InputCode_SetIfKeyFree(VK_RETURN, IC_Accept);
    im.Key2InputCode_SetIfKeyFree(VK_ESCAPE, IC_Reject);
    im.Key2InputCode_SetIfKeyFree(VK_BACK,   IC_Con_BackSpace);

    im.Key2InputCode_SetIfKeyFree('C',       IC_CameraChange);
    im.Key2InputCode_SetIfKeyFree(VK_TAB,    IC_CameraReset);
    im.Key2InputCode_SetIfKeyFree('1',       IC_CameraFront);
    im.Key2InputCode_SetIfKeyFree('2',       IC_CameraBack);
    im.Key2InputCode_SetIfKeyFree('3',       IC_CameraLeft);
    im.Key2InputCode_SetIfKeyFree('4',       IC_CameraRight);
    im.Key2InputCode_SetIfKeyFree('5',       IC_CameraTop);
    im.Key2InputCode_SetIfKeyFree('6',       IC_CameraBottom);
    im.Key2InputCode_SetIfKeyFree('7',       IC_CameraPerspective);

    im.Key2InputCode_SetIfKeyFree('V',       IC_PolyModeChange);
    im.Key2InputCode_SetIfKeyFree('B',       IC_ShowBonesAlways);
    im.Key2InputCode_SetIfKeyFree('F',       IC_ViewPhysicalModel);

    im.Key2InputCode_SetIfKeyFree(VK_LBUTTON, IC_LClick);
    im.Key2InputCode_SetIfKeyFree(VK_RBUTTON, IC_RClick);
    im.Key2InputCode_SetIfKeyFree(VK_SHIFT,   IC_RunModifier);
    im.Key2InputCode_SetIfKeyFree(VK_CONTROL, IC_BE_Modifier);
    im.Key2InputCode_SetIfKeyFree(VK_DELETE,  IC_BE_Delete);
    im.Key2InputCode_SetIfKeyFree('N', IC_BE_Select);
    im.Key2InputCode_SetIfKeyFree('M', IC_BE_Move);
    im.Key2InputCode_SetIfKeyFree('P', IC_BE_Play);

    im.Key2InputCode_SetIfKeyFree(VK_PRIOR,  IC_MoveForward);
    im.Key2InputCode_SetIfKeyFree(VK_NEXT,   IC_MoveBack);
    im.Key2InputCode_SetIfKeyFree(VK_LEFT,   IC_MoveLeft);
    im.Key2InputCode_SetIfKeyFree(VK_RIGHT,  IC_MoveRight);
    im.Key2InputCode_SetIfKeyFree(VK_UP,     IC_MoveUp);
    im.Key2InputCode_SetIfKeyFree(VK_DOWN,   IC_MoveDown);

    im.Key2InputCode_SetIfKeyFree('W', IC_TurnUp);
    im.Key2InputCode_SetIfKeyFree('S', IC_TurnDown);
    im.Key2InputCode_SetIfKeyFree('D', IC_TurnLeft);
    im.Key2InputCode_SetIfKeyFree('A', IC_TurnRight);
    im.Key2InputCode_SetIfKeyFree('Q', IC_RollLeft);
    im.Key2InputCode_SetIfKeyFree('E', IC_RollRight);

    im.Key2InputCode_SetIfKeyFree('U', IC_OrbitUp);
    im.Key2InputCode_SetIfKeyFree('J', IC_OrbitDown);
    im.Key2InputCode_SetIfKeyFree('H', IC_OrbitLeft);
    im.Key2InputCode_SetIfKeyFree('K', IC_OrbitRight);
}

void SceneSkeleton::Resize(int left, int top, unsigned int width, unsigned int height)
{
    Scene::Resize(left, top, width, height);
    InitCameras(false);

    const GLFont* pFont = g_FontMgr.GetFont(Font);
    for (size_t i = 0; i < Buttons.size(); ++i)
        for (size_t j = 0; j < Buttons[i].size(); ++j)
        {
            Buttons[i][j].Y = Height-20.f;
            Buttons[i][j].Y2 = Buttons[i][j].Y + pFont->LineH();
        }
}

bool SceneSkeleton::Invalidate()
{
    WorldRenderGL().Invalidate(Model);
    return Scene::Invalidate();
}

void SceneSkeleton::Destroy()
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

    g_FontMgr.DeleteReference(Font);
    Font = HFont();

    Directories.clear();
    Buttons.clear();

    WorldRenderGL().Free(Model);
    Model.Destroy();
}

#define fractf(a)    ((a)-floorf(a))

/************************** RENDER *************************************/
bool SceneSkeleton::Render()
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
    GLfloat light_amb_color[]  = { 0.1f, 0.1f, 0.1f, 1.0f };
    GLfloat light_dif_color[]  = { 0.9f, 0.9f, 0.9f, 1.0f };
    GLfloat light_spec_color[] = { 0.9f, 0.9f, 0.9f, 1.0f };
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
    Model.Render();

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
        render.RenderBVH(*model.BVHierarchy, xMatrix::Identity(), true, 0, Selection.BVHNodeID);

    GLShader::Suspend();

    glFlush();

    //////////////////////////// Overlay

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Set text output projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, Width, Height, 0, 0, 100);
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
            glVertex3i(Selection.RectStartX, Selection.RectStartY, 0);
            glVertex2i(Selection.RectStartX, y);
            glVertex2i(x, y);
            glVertex2i(x, Selection.RectStartY);
        }
        glEnd();
        glEnable(GL_CULL_FACE);
        glDisable(GL_BLEND);
    }

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
    pFont->PrintF(5.f, 20.f, 0.f, sCamera);

    if (EditMode == emCreateBone)
        pFont->PrintF(5.f, Height-5.f, 0.f, "Skeleton |");
    else if (EditMode == emCreateConstraint_Type)
        pFont->PrintF(5.f, Height-5.f, 0.f, "Skeleton constraints |");
    else if (EditMode == emCreateConstraint_Node)
    {
        if (Constraint.type == IC_BE_CreateConstrWeight)
            pFont->PrintF(5.f, Height-5.f, 0.f, "Skeleton constraints | select bone");
        else
        if (Constraint.type == IC_BE_CreateConstrAng)
            pFont->PrintF(5.f, Height-5.f, 0.f, "Skeleton constraints | select bone to constrain");
        else
        if (Constraint.boneA == xBYTE_MAX)
            pFont->PrintF(5.f, Height-5.f, 0.f, "Skeleton constraints | select first node");
        else
        if (Constraint.boneB == xBYTE_MAX)
            pFont->PrintF(5.f, Height-5.f, 0.f, "Skeleton constraints | select second node");
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
                    pFont->PrintF(5.f, Height-85.f+20.f*i, 0.f, "%s Angle: ?", label);
                else
                if (Constraint.step == i)
                    pFont->PrintF(5.f, Height-85.f+20.f*i, 0.f, "%s Angle: (%2.2f) %s", label, Constraint.angles[i], InputState.String.c_str());
                else
                    pFont->PrintF(5.f, Height-85.f+20.f*i, 0.f, "%s Angle: %2.2f", label, Constraint.angles[i]);
            }
        else
        if (Constraint.type == IC_BE_CreateConstrWeight)
            pFont->PrintF(5.f, Height-25.f, 0.f, "Weight: (%2.2f) %s", Constraint.M_weight, InputState.String.c_str());
        else
            pFont->PrintF(5.f, Height-25.f, 0.f, "Length: (%2.2f) %s", Constraint.S_length, InputState.String.c_str());
        pFont->PrintF(5.f, Height-5.f, 0.f, "Skeleton constraints | Input parameters of the constraint");
    }
    else if (EditMode == emEditBVH)
        pFont->PrintF(5.f, Height-5.f, 0.f, "BVH Editor |");
    else if (EditMode == emSelectBVHBone)
        pFont->PrintF(5.f, Height-5.f, 0.f, "BVH Editor | Click: Select Bone");
    else if (EditMode == emCreateBVH)
        pFont->PrintF(5.f, Height-5.f, 0.f, "BVH Editor |");
    else if (EditMode == emEditVolume)
        pFont->PrintF(5.f, Height-5.f, 0.f, "BVH Editor | Drag: Edit bounding volume");
    else if (EditMode == emSelectElement)
        pFont->PrintF(5.f, Height-5.f, 0.f, "Skinning | Click: Select Element");
    else if (EditMode == emSelectVertex)
    {
        pFont->PrintF(5.f, Height-5.f, 0.f,
            "Skinning | Drag: Select Vertices | %s+Drag: Unselect Vertices | %s: Assign bones to selected Vertices",
            KeyName_Modify, KeyName_Accept);
        pFont->PrintF(5.f, 40.f, 0.f, "%d vertices selected", Selection.Vertices.size());

        if (Selection.Element->FL_skeletized && HoveredVert != xDWORD_MAX)
        {
            size_t stride = Selection.Element->FL_textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel);
            xVertexSkel *vert = (xVertexSkel*)(((xBYTE*)Selection.Element->L_vertices) + stride * HoveredVert);
            for (int i=0; i < 4 && fractf(vert->bone[i]) != 0.f; ++i)
                pFont->PrintF(5.f, 20.f * (i+3), 0.f, "Bone id%d : %d", (int)floorf(vert->bone[i]), (int)(fractf(vert->bone[i])*1000));
        }
    }
    else if (EditMode == emSelectBone)
        pFont->PrintF(5.f, Height-5.f, 0.f, "Skinning | Click: Select Bone");
    else if (EditMode == emInputWght)
        pFont->PrintF(5.f, Height-5.f, 0.f, "Skinning | Type in Bone weight | %s: Accept", KeyName_Accept);
    else if (EditMode == emSaveModel)
        pFont->PrintF(5.f, Height-5.f, 0.f, "Save |");

    else if (EditMode == emSelectAnimation || EditMode == emAnimateBones
        || EditMode == emLoadAnimation || EditMode == emEditAnimation
        || EditMode == emFrameParams || EditMode == emSaveAnimation)
    {
        pFont->PrintF(5.f, Height-5.f, 0.f, "Animation |");
        if (EditMode != emSelectAnimation && EditMode != emLoadAnimation)
            if (AnimationName.size())
                pFont->PrintF(150.f, 20.f, 0.f, "Animation: %s", AnimationName.c_str());
            else
                pFont->PrintF(150.f, 20.f, 0.f, "Animation: new animation");
    }

    if (EditMode == emEditAnimation)
        RenderProgressBar();
    else if (EditMode == emFrameParams) {
        if (Animation.KeyFrame.step == 1)
            pFont->PrintF(5.f, 40.f, 0.f, "Freeze: (%d) %s", Animation.KeyFrame.freeze, InputState.String.c_str());
        else
        {
            pFont->PrintF(5.f, 40.f, 0.f, "Freeze: %d", Animation.KeyFrame.freeze);
            pFont->PrintF(5.f, 60.f, 0.f, "Duration: (%d) %s", Animation.KeyFrame.duration, InputState.String.c_str());
        }
    }
    else if (EditMode == emSaveAnimation || EditMode == emSaveModel)
        pFont->PrintF(5.f, Height-25.f, 0.f, "Filename: %s", InputState.String.c_str());

    std::vector<GLButton>::iterator begin = Buttons[EditMode].begin();
    std::vector<GLButton>::iterator end   = Buttons[EditMode].end();
    for (; begin != end; ++begin)
        begin->Render(pFont);

    if (EditMode == emSaveAnimation || EditMode == emLoadAnimation || EditMode == emSaveModel)
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
            pFont->PrintF(5.f, 20.f * (i+2), 0.f, "Bone id%d : %d", Skin.BoneWeight[i].id, Skin.BoneWeight[i].weight);
        }
        if (EditMode == emInputWght)
            pFont->PrintF(5.f, 20.f * (i+2), 0.f, "Bone id%d : (%d) %s", Selection.Bone->ID, 100 - sum, InputState.String.c_str());
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
    float y1 = 10.f;
    float y2 = y1+pHeight;

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
    pFont->PrintF(5.f, 40.f, 0.f, "Frame: %d/%d | Progress: %d/%d (%d/%d)",
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
        render.RenderBVH(*model.BVHierarchy, xMatrix::Identity(), true, 0, 0, true);
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
