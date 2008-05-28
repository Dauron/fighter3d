#include "SceneSkeleton.h"

#include "../AppFramework/Input/InputMgr.h"
#include "../Utils/Filesystem.h"
#include "../Graphics/OGL/GLShader.h"

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
    Buttons[emMain].push_back(GLButton("Skinning",        160, 2,  80, 15, IC_BE_ModeSkin));
    Buttons[emMain].push_back(GLButton("Animating",       245, 2,  90, 15, IC_BE_ModeAnimate));
    Buttons[emMain].push_back(GLButton("Graph/Phys",      340, 2, 100, 15, IC_BE_Select));
    Buttons[emMain].push_back(GLButton("Save",            445, 2,  45, 15, IC_BE_Save));

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

    xModel *modelGr = Model.GetModelGr();
    xModel *modelPh = Model.GetModelPh();
    if (!modelPh->spine.I_bones && modelGr->spine.I_bones)
    {
        modelPh->SkeletonAdd(); //   add skeleton to model
        Model.CopySpineToPhysical();
    }
    else
    if (!modelGr->spine.I_bones && modelPh->spine.I_bones)
    {
        modelGr->SkeletonAdd(); //   add skeleton to model
        Model.CopySpineToGraphics();
    }
    else
        Model.CopySpineToGraphics();
    modelGr->spine.ResetQ();
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
    if (!Cameras.Current)
    {
        Cameras.Front.SetCamera(0.0f, -5.0f, 1.7f, 0.0f, 0.0f, 1.7f, 0.0f, 0.0f, 1.0f);
        Cameras.Back.SetCamera(0.0f, +5.0f, 1.7f, 0.0f, 0.0f, 1.7f, 0.0f, 0.0f, 1.0f);
        Cameras.Right.SetCamera(-5.0f, 0.0f, 1.7f, 0.0f, 0.0f, 1.7f, 0.0f, 0.0f, 1.0f);
        Cameras.Left.SetCamera(+5.0f, 0.0f, 1.7f, 0.0f, 0.0f, 1.7f, 0.0f, 0.0f, 1.0f);
        Cameras.Top.SetCamera(0.0f, 0.0f,  5.0f, 0.0f, 0.0f, 1.7f, 0.0f, -1.0f, 0.0f);
        Cameras.Bottom.SetCamera(0.0f, 0.0f, -5.0f, 0.0f, 0.0f, 1.7f, 0.0f, -1.0f, 0.0f);
        Cameras.Perspective.SetCamera(-5.0f, -5.0f, 1.7f, 0.0f, 0.0f, 1.7f, 0.0f, 0.0f, 1.0f);
        Cameras.Current = &Cameras.Front;
    }
    return true;
}

void SceneSkeleton::InitInputMgr()
{
    InputMgr &im = g_InputMgr;
    im.SetScene(sceneName);

    im.SetInputCode(VK_RETURN, IC_Accept);
    im.SetInputCode(VK_ESCAPE, IC_Reject);
    im.SetInputCode(VK_BACK,   IC_Con_BackSpace);
    im.SetInputCode(VK_F11,    IC_FullScreen);
#ifdef WIN32
    im.SetInputCode(VK_OEM_3,  IC_Console);
#else
    im.SetInputCode('`',       IC_Console);
#endif

    im.SetInputCode('C',       IC_CameraChange);
    im.SetInputCode(VK_TAB,    IC_CameraReset);
    im.SetInputCode('1',       IC_CameraFront);
    im.SetInputCode('2',       IC_CameraBack);
    im.SetInputCode('3',       IC_CameraLeft);
    im.SetInputCode('4',       IC_CameraRight);
    im.SetInputCode('5',       IC_CameraTop);
    im.SetInputCode('6',       IC_CameraBottom);
    im.SetInputCode('7',       IC_CameraPerspective);

    im.SetInputCode('V',       IC_PolyModeChange);
    im.SetInputCode('B',       IC_ShowBonesAlways);
    im.SetInputCode('F',       IC_ViewPhysicalModel);

    im.SetInputCode(VK_LBUTTON, IC_LClick);
    im.SetInputCode(VK_RBUTTON, IC_RClick);
    im.SetInputCode(VK_SHIFT,   IC_RunModifier);
    im.SetInputCode(VK_CONTROL, IC_BE_Modifier);
    im.SetInputCode(VK_DELETE,  IC_BE_Delete);
    im.SetInputCode('N', IC_BE_Select);
    im.SetInputCode('M', IC_BE_Move);
    im.SetInputCode('P', IC_BE_Play);

    im.SetInputCode(VK_PRIOR,  IC_MoveForward);
    im.SetInputCode(VK_NEXT,   IC_MoveBack);
    im.SetInputCode(VK_LEFT,   IC_MoveLeft);
    im.SetInputCode(VK_RIGHT,  IC_MoveRight);
    im.SetInputCode(VK_UP,     IC_MoveUp);
    im.SetInputCode(VK_DOWN,   IC_MoveDown);

    im.SetInputCode('W', IC_TurnUp);
    im.SetInputCode('S', IC_TurnDown);
    im.SetInputCode('D', IC_TurnLeft);
    im.SetInputCode('A', IC_TurnRight);
    im.SetInputCode('Q', IC_RollLeft);
    im.SetInputCode('E', IC_RollRight);

    im.SetInputCode('U', IC_OrbitUp);
    im.SetInputCode('J', IC_OrbitDown);
    im.SetInputCode('H', IC_OrbitLeft);
    im.SetInputCode('K', IC_OrbitRight);
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

    // Set projection
    glViewport(Left, Top, Width, Height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (Cameras.Current == &Cameras.Perspective)
        xglPerspective(45, AspectRatio, 0.1, 1000);
    else
    {
        double scale = fabs((Cameras.Current->eye - Cameras.Current->center).length());
        glOrtho( -scale*AspectRatio, scale*AspectRatio, -scale, scale, 0.1, 1000 );
    }
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

    xFieldOfView FOV; FOV.Empty = true;
    Cameras.Current->LookAtMatrix(FOV.ViewTransform);
    glLoadMatrixf(&FOV.ViewTransform.x0); //Camera_Aim_GL(*Cameras.Current);
    //FOV.update();

    xModel         &model         = (State.DisplayPhysical) ? *Model.GetModelPh() : *Model.GetModelGr();
    xModelInstance &modelInstance = (State.DisplayPhysical) ? Model.modelInstancePh : Model.modelInstanceGr;
    Renderer       &render        = Model.renderer;

    render.RenderModel(model, modelInstance, false, FOV);
    render.RenderModel(model, modelInstance, true, FOV);
    GLShader::Suspend();

    GLShader::EnableTexturing(xState_Disable);
    GLShader::SetLightType(xLight_NONE);
    GLShader::Start();

    if (EditMode == emSelectVertex)
        render.RenderVertices(model, modelInstance, Renderer::smNone, Selection.ElementId, &Selection.Vertices);
    if (State.ShowBonesAlways || EditMode == emSelectBone || EditMode == emCreateBone ||
        EditMode == emCreateConstraint_Node || EditMode == emCreateConstraint_Params ||
        EditMode == emInputWght  || (EditMode == emAnimateBones && !State.HideBonesOnAnim))
        render.RenderSkeleton(model, modelInstance, Selection.Bone ? Selection.Bone->ID : xWORD_MAX);

    GLShader::Suspend();

    glFlush();

    //////////////////////////// Overlay

    glDisable(GL_DEPTH_TEST);
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
                char *label;
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
    else if (EditMode == emSelectElement)
        pFont->PrintF(5.f, 5.f, 0.f, "Skinning | Click: Select Element");
    else if (EditMode == emSelectVertex)
    {
        pFont->PrintF(5.f, 5.f, 0.f,
            "Skinning | Drag: Select Vertices | %s+Drag: Unselect Vertices | %s: Assign bones to selected Vertices",
            KeyName_Modify, KeyName_Accept);
        pFont->PrintF(5.f, Height-40.f, 0.f, "%d vertices selected", Selection.Vertices.size());

        if (Selection.Element->skeletized && HoveredVert != xDWORD_MAX)
        {
            size_t stride = Selection.Element->textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel);
            xVertexSkel *vert = (xVertexSkel*)(((xBYTE*)Selection.Element->verticesP) + stride * HoveredVert);
            for (int i=0; i < 4 && fractf(vert->bone[i]) != 0.f; ++i)
                pFont->PrintF(5.f, Height - 20.f * (i+3), 0.f, "Bone id%d : %d", (int)floorf(vert->bone[i]), (int)(fractf(vert->bone[i])*1000));
        }
    }
    else if (EditMode == emSelectBone)
        pFont->PrintF(5.f, 5.f, 0.f, "Skinning | Click: Select Bone");
    else if (EditMode == emInputWght)
        pFont->PrintF(5.f, 5.f, 0.f, "Skinning | Type in Bone weight | %s: Accept", KeyName_Accept);
    else if (EditMode == emSelectAnimation || EditMode == emAnimateBones
        || EditMode == emLoadAnimation)
        pFont->PrintF(5.f, 5.f, 0.f, "Animation |");
    else if (EditMode == emEditAnimation)
    {
        pFont->PrintF(5.f, 5.f, 0.f, "Animation |");
        RenderProgressBar();
    }
    else if (EditMode == emFrameParams) {
        pFont->PrintF(5.f, 5.f, 0.f, "Animation |");
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
        pFont->PrintF(5.f, 5.f, 0.f, "Animation |");
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
    float scale = ((float)pWidth) / info.Duration;

    xKeyFrame *frame = Animation.Instance->frameP;
    xWORD cnt = Animation.Instance->frameC;
    for(; frame && cnt; frame = frame->next, --cnt)
    {
        x1 = x3;
        x2 = x1 + frame->freeze*scale;
        x3 = x2 + frame->duration*scale;

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
        glVertex2f (x+info.Progress*scale, y1+2),
        glVertex2f (x+info.Progress*scale, y2-3);
    glEnd();

    const GLFont* pFont = g_FontMgr.GetFont(Font);
    pFont->PrintF(5.f, Height-40.f, 0.f, "Frame: %d/%d | Progress: %d/%d (%d/%d)",
        info.FrameNo, Animation.Instance->frameC,
        info.Progress, info.Duration,
        Animation.Instance->progress,
        Animation.Instance->frameCurrent->freeze+Animation.Instance->frameCurrent->duration);
}

/************************** SELECTIONS *************************************/
void SceneSkeleton::RenderSelect(const xFieldOfView *FOV)
{
    xModel         &model         = (State.DisplayPhysical) ? *Model.GetModelPh() : *Model.GetModelGr();
    xModelInstance &modelInstance = (State.DisplayPhysical) ? Model.modelInstancePh : Model.modelInstanceGr;
    Renderer       &render        = Model.renderer;

    if (EditMode == emCreateBone || EditMode == emCreateConstraint_Node ||
        EditMode == emSelectBone || EditMode == emAnimateBones)
        if (EditMode == emCreateBone && State.CurrentAction == IC_BE_DeleteConstr)
            render.RenderSkeletonSelection(model, modelInstance, true);
        else
            render.RenderSkeletonSelection(model, modelInstance, false);
    else
    if (EditMode == emSelectElement)
        render.RenderVertices(model, modelInstance, Renderer::smElement);
    else
    if (EditMode == emSelectVertex)
        render.RenderVertices(model, modelInstance, Renderer::smVertex, Selection.ElementId);
}
unsigned int SceneSkeleton::CountSelectable()
{
    if (EditMode == emCreateBone || EditMode == emCreateConstraint_Node ||
        EditMode == emSelectBone || EditMode == emAnimateBones)
        if (EditMode == emCreateBone && State.CurrentAction == IC_BE_DeleteConstr)
            return Model.GetModelGr()->spine.I_constraints;
        else
            return Model.GetModelGr()->spine.I_bones;
    else
    if (EditMode == emSelectElement)
        if (State.DisplayPhysical)
            return Model.GetModelPh()->elementC;
        else
            return Model.GetModelGr()->elementC;
    else
    if (EditMode == emSelectVertex)
        return Selection.Element->verticesC;
    return 0;
}
std::vector<xDWORD> *SceneSkeleton::SelectCommon(int X, int Y, int W, int H)
{
    glViewport(Left, Top, Width, Height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (Cameras.Current == &Cameras.Perspective)
        xglPerspective(45, AspectRatio, 0.1, 1000);
    else
    {
        double scale = fabs((Cameras.Current->eye - Cameras.Current->center).length());
        glOrtho( -scale*AspectRatio, scale*AspectRatio, -scale, scale, 0.1, 1000 );
    }
    glMatrixMode(GL_MODELVIEW);
    Camera_Aim_GL(*Cameras.Current);

    return ISelectionProvider::Select(NULL, X, Y, W, H);
}

xBone *SceneSkeleton::SelectBone(int X, int Y)
{
    std::vector<xDWORD> *sel = SelectCommon(X, Y);

    if (sel && sel->size()) {
        GLuint id = sel->back();
        delete sel;
        return Model.GetModelGr()->spine.L_bones + id;
    }
    delete sel;
    return NULL;
}

xWORD SceneSkeleton::SelectElement(int X, int Y)
{
    std::vector<xDWORD> *sel = SelectCommon(X, Y);

    if (sel && sel->size()) {
        GLuint id = sel->back();
        delete sel;
        return (xWORD)id;
    }
    delete sel;
    return xWORD_MAX;
}
