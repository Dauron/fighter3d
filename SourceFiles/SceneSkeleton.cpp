#include "SceneSkeleton.h"
#include "SceneConsole.h"

#include "../AppFramework/Application.h"
#include "../AppFramework/Input/InputMgr.h"
#include "../OpenGL/Textures/TextureMgr.h"

#include "../OpenGL/GLAnimSkeletal.h"
#include "../Models/lib3dx/xRender.h"

#include "LightsAndMaterials.h"
#include <algorithm>

SceneSkeleton::SceneSkeleton(Scene *prevScene, const char* modelName)
        : m_EditMode(emMain), m_PrevScene(prevScene),
          modifyButton(NULL), acceptButton(NULL), mouseLIsDown(false),
          play(false), currentAction(0), selectedBone(NULL), selectedElemID(-1),
          currentAnimation(NULL)

{
    sceneName = "[Skeleton]";
    m_Font = HFont();
    m_Cameras.Current = NULL;

    m_Buttons.resize(emLast);
    m_Buttons[emMain].push_back(GLButton("Create Skeleton",  10, 2, 145, 15, IC_BE_ModeSkeletize));
    m_Buttons[emMain].push_back(GLButton("Skinning",        160, 2,  80, 15, IC_BE_ModeSkin));
    m_Buttons[emMain].push_back(GLButton("Animating",       245, 2,  90, 15, IC_BE_ModeAnimate));
    m_Buttons[emMain].push_back(GLButton("Save",            340, 2,  45, 15, IC_BE_Save));

    m_Buttons[emCreateBone].push_back(GLButton("Select", 100, 2, 65, 15, IC_BE_Select, true, true));
    m_Buttons[emCreateBone].push_back(GLButton("Create", 170, 2, 65, 15, IC_BE_Create, true));
    m_Buttons[emCreateBone].push_back(GLButton("Move",   240, 2, 45, 15, IC_BE_Move,   true));
    m_Buttons[emCreateBone].push_back(GLButton("Delete", 290, 2, 65, 15, IC_BE_Delete));

    m_Buttons[emSelectAnimation].push_back(GLButton("New",  110, 2, 35, 15, IC_BE_Create));
    m_Buttons[emSelectAnimation].push_back(GLButton("Load", 150, 2, 45, 15, IC_BE_Select));

    m_Buttons[emEditAnimation].push_back(GLButton("Play",      110, 2, 45, 15, IC_BE_Play));
    m_Buttons[emEditAnimation].push_back(GLButton("Insert KF", 160, 2, 90, 15, IC_BE_Create));
    m_Buttons[emEditAnimation].push_back(GLButton("Edit KF",   255, 2, 70, 15, IC_BE_Edit));
    m_Buttons[emEditAnimation].push_back(GLButton("KF Time",   330, 2, 70, 15, IC_BE_Move));
    m_Buttons[emEditAnimation].push_back(GLButton("Delete KF", 405, 2, 90, 15, IC_BE_Delete));
    m_Buttons[emEditAnimation].push_back(GLButton("Loop",      500, 2, 45, 15, IC_BE_Loop));
    m_Buttons[emEditAnimation].push_back(GLButton("Save",      550, 2, 45, 15, IC_BE_Save));

    m_Buttons[emAnimateBones].push_back(GLButton("Select",     110, 2, 65, 15, IC_BE_Select, true, true));
    m_Buttons[emAnimateBones].push_back(GLButton("Move",       180, 2, 45, 15, IC_BE_Move,   true));
    m_Buttons[emAnimateBones].push_back(GLButton("Reset Bone", 230, 2, 95, 15, IC_BE_Delete));
    m_Buttons[emAnimateBones].push_back(GLButton("Tgl.Bones",  330, 2, 90, 15, IC_BE_ModeSkeletize));
    m_Buttons[emAnimateBones].push_back(GLButton("Accept",     425, 2, 65, 15, IC_BE_Save));
    m_Buttons[emAnimateBones].push_back(GLButton("Reject",     495, 2, 65, 15, IC_Reject));

    m_Buttons[emLoadAnimation].push_back(GLButton("Reject",    110, 2, 65, 15, IC_Reject));

    m_Buttons[emSaveAnimation].push_back(GLButton("Accept",    110, 2, 65, 15, IC_Accept));
    m_Buttons[emSaveAnimation].push_back(GLButton("Reject",    180, 2, 65, 15, IC_Reject));

    m_Model.Initialize(modelName);

    xRender *renderer = m_Model.GetRenderer();
    xSkeletonReset(renderer->xModel->spineP);
    renderer->CalculateSkeleton();

    m_CurrentDirectory = Filesystem::GetFullPath(Filesystem::GetParentDir(m_Model.GetName()));
}
bool SceneSkeleton::Initialize(int left, int top, unsigned int width, unsigned int height)
{
    Scene::Initialize(left, top, width, height);
    InitInputMgr();
    g_InputMgr.mouseWheel = 0;
    if (!modifyButton) {
        modifyButton = strdup(g_InputMgr.GetKeyName(g_InputMgr.GetKeyCode(IC_BE_Modifier)).data());
        acceptButton = strdup(g_InputMgr.GetKeyName(g_InputMgr.GetKeyCode(IC_Accept)).data());
    }
    if (!m_Cameras.Current)
    {
        m_Cameras.Front.SetCamera(0.0f, -5.0f, 1.7f, 0.0f, 0.0f, 1.7f, 0.0f, 0.0f, 1.0f);
        m_Cameras.Back.SetCamera(0.0f, +5.0f, 1.7f, 0.0f, 0.0f, 1.7f, 0.0f, 0.0f, 1.0f);
        m_Cameras.Right.SetCamera(-5.0f, 0.0f, 1.7f, 0.0f, 0.0f, 1.7f, 0.0f, 0.0f, 1.0f);
        m_Cameras.Left.SetCamera(+5.0f, 0.0f, 1.7f, 0.0f, 0.0f, 1.7f, 0.0f, 0.0f, 1.0f);
        m_Cameras.Top.SetCamera(0.0f, 0.0f,  5.0f, 0.0f, 0.0f, 1.7f, 0.0f, -1.0f, 0.0f);
        m_Cameras.Bottom.SetCamera(0.0f, 0.0f, -5.0f, 0.0f, 0.0f, 1.7f, 0.0f, -1.0f, 0.0f);
        m_Cameras.Perspective.SetCamera(-5.0f, -5.0f, 1.7f, 0.0f, 0.0f, 1.7f, 0.0f, 0.0f, 1.0f);
        m_Cameras.Current = &m_Cameras.Front;
    }
    return InitGL();
}

bool SceneSkeleton::InitGL()
{
    glClearColor( 0.5f, 0.5f, 0.5f, 0.f );  // Background color
    glClearDepth(100.0f);                   // Draw distance ???
    glDepthFunc(GL_LEQUAL);                 // Depth testing function

    glEnable(GL_CULL_FACE);                 // Do not draw hidden faces
    glCullFace (GL_BACK);                   // Hide back faces
    glFrontFace(GL_CCW);                    // Front faces are drawn in counter-clockwise direction

    glShadeModel(GL_SMOOTH);                // GL_SMOOTH - enable smooth shading, GL_FLAT - no gradient on faces
    glDisable (GL_POINT_SMOOTH);
    glEnable (GL_LINE_SMOOTH);
    glEnable (GL_POLYGON_SMOOTH);           // produces errors on many cards... use FSAA!

    glHint(GL_POINT_SMOOTH_HINT, GL_FASTEST /*GL_NICEST*/);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); // Nice perspective calculations

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
    im.SetInputCode(VK_BACK,   IC_Con_BackSpace);
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

    im.SetInputCode(VK_LBUTTON, IC_LClick);
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
    m_Cameras.Current = NULL;
    if (modifyButton) {
        delete[] modifyButton;  modifyButton = 0;
        delete[] acceptButton;  acceptButton = 0;
    }
    if (currentAnimation) {
        currentAnimation->Unload();
        delete currentAnimation;
        currentAnimation = NULL;
    }

    g_FontMgr.DeleteFont(m_Font);
    m_Font = HFont();

    if (m_PrevScene) {
        m_PrevScene->Terminate();
        delete m_PrevScene;
        m_PrevScene = NULL;
    }

    m_Directories.clear();
}

/************************** RENDER *************************************/
bool SceneSkeleton::Render()
{
    // Clear surface
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT/* | GL_STENCIL_BUFFER_BIT*/);

    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, g_PolygonMode);

    // Set projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    xglPerspective(45, AspectRatio, 0.1, 1000);
    glMatrixMode(GL_MODELVIEW);
    glViewport(Left, Top, Width, Height);

    setLights(); // lights at viewer position
    Camera_Aim_GL(*m_Cameras.Current);

    xRender *renderer = m_Model.GetRenderer();
    renderer->RenderModel();

    g_TextureMgr.DisableTextures();
    GLShader::EnableLighting(0);
    glDisable(GL_LIGHTING);

    if (m_EditMode == emSelectVertex)
        renderer->RenderVertices(xRender::smNone, selectedElemID, &selectedVert);
    if (m_EditMode == emSelectBone || m_EditMode == emCreateBone ||
        m_EditMode == emInputWght  || (m_EditMode == emAnimateBones && showBonesOnAnim))
        renderer->RenderSkeleton(false, selectedBone ? selectedBone->id : -1);

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

    if (mouseLIsDown && m_EditMode == emSelectVertex)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_CULL_FACE);
        glColor4f(0.3f, 0.3f, 0.9f, 0.3f);
        glBegin(GL_QUADS);
        {
            int x = g_InputMgr.mouseX, y = g_InputMgr.mouseY;
            glVertex3i(selStartX, Height - selStartY, 0);
            glVertex2i(selStartX, Height - y);
            glVertex2i(x, Height - y);
            glVertex2i(x, Height - selStartY);
        }
        glEnd();
        glEnable(GL_CULL_FACE);
        glDisable(GL_BLEND);
    }

    if (!g_FontMgr.IsHandleValid(m_Font))
        m_Font = g_FontMgr.GetFont("Courier New", 15);
    const GLFont* pFont = g_FontMgr.GetFont(m_Font);

    glColor4f( 1.f, 1.f, 1.f, 1.f );

    const char* sCamera = "NULL";
    if (m_Cameras.Current == &m_Cameras.Front)
        sCamera = "Front";
    if (m_Cameras.Current == &m_Cameras.Right)
        sCamera = "Right";
    if (m_Cameras.Current == &m_Cameras.Back)
        sCamera = "Back";
    if (m_Cameras.Current == &m_Cameras.Left)
        sCamera = "Left";
    if (m_Cameras.Current == &m_Cameras.Top)
        sCamera = "Top";
    if (m_Cameras.Current == &m_Cameras.Bottom)
        sCamera = "Bottom";
    if (m_Cameras.Current == &m_Cameras.Perspective)
        sCamera = "Perspective";
    pFont->PrintF(5.f, Height - 20.f, 0.f, sCamera);

    if (m_EditMode == emCreateBone)
        pFont->PrintF(5.f, 5.f, 0.f, "Skeleton |");
    else if (m_EditMode == emSelectElement)
        pFont->PrintF(5.f, 5.f, 0.f, "Skinning | Click: Select Element");
    else if (m_EditMode == emSelectVertex)
    {
        pFont->PrintF(5.f, 5.f, 0.f,
            "Skinning | Drag: Select Vertices | %s+Drag: Unselect Vertices | %s: Assign bones to selected Vertices",
            modifyButton, acceptButton);
        pFont->PrintF(5.f, Height-40.f, 0.f, "%d vertices selected", selectedVert.size());
    }
    else if (m_EditMode == emSelectBone)
        pFont->PrintF(5.f, 5.f, 0.f, "Skinning | Click: Select Bone");
    else if (m_EditMode == emInputWght)
        pFont->PrintF(5.f, 5.f, 0.f, "Skinning | Type in Bone weight | %s: Accept", acceptButton);
    else if (m_EditMode == emSelectAnimation || m_EditMode == emAnimateBones
        || m_EditMode == emLoadAnimation)
        pFont->PrintF(5.f, 5.f, 0.f, "Animation |");
    else if (m_EditMode == emEditAnimation)
    {
        pFont->PrintF(5.f, 5.f, 0.f, "Animation |");
        RenderProgressBar();
    }
    else if (m_EditMode == emFrameParams) {
        pFont->PrintF(5.f, 5.f, 0.f, "Animation |");
        if (param == 0)
            pFont->PrintF(5.f, Height - 40.f, 0.f, "Freeze: (%d) %s", frameParams[0], command.data());
        else
        {
            pFont->PrintF(5.f, Height - 40.f, 0.f, "Freeze: %d", frameParams[0]);
            pFont->PrintF(5.f, Height - 60.f, 0.f, "Duration: (%d) %s", frameParams[1], command.data());
        }
    }
    else if (m_EditMode == emSaveAnimation)
    {
        pFont->PrintF(5.f, 5.f, 0.f, "Animation |");
        if (m_AnimationName.size())
            pFont->PrintF(5.f, 25, 0.f, "Filename: (%s) %s", m_AnimationName.data(), command.data());
        else
            pFont->PrintF(5.f, 25, 0.f, "Filename: %s", command.data());
    }

    std::vector<GLButton>::iterator begin = m_Buttons[m_EditMode].begin();
    std::vector<GLButton>::iterator end   = m_Buttons[m_EditMode].end();
    for (; begin != end; ++begin)
        begin->Render(pFont);

    if (m_EditMode == emSaveAnimation || m_EditMode == emLoadAnimation)
    {
        begin = m_Directories.begin();
        end   = m_Directories.end();
        for (; begin != end; ++begin)
            begin->Render(pFont);
    }

    if (m_EditMode == emSelectBone || m_EditMode == emInputWght) {
        int i;
        int sum = 0;
        for (i=0; i < 4 && boneWghts[i] != 0; ++i) {
            sum += boneWghts[i];
            pFont->PrintF(5.f, Height - 20.f * (i+2), 0.f, "Bone id%d : %d", boneIds[i], boneWghts[i]);
        }
        if (m_EditMode == emInputWght)
            pFont->PrintF(5.f, Height - 20.f * (i+2), 0.f, "Bone id%d : (%d) %s", selectedBone->id, 100 - sum, command.data());
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

    xAnimationInfo info = currentAnimation->GetInfo();
    float scale = ((float)pWidth) / info.Duration;

    xKeyFrame *frame = currentAnimation->frameP;
    xWORD cnt = currentAnimation->frameC;
    for(; frame && cnt; frame = frame->next, --cnt)
    {
        x1 = x3;
        x2 = x1 + frame->freeze*scale;
        x3 = x2 + frame->duration*scale;

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glColor3f(0.4f, 0.4f, 1.f);
        GLUtils::Rectangle(x1, y2, x2, y1);

        glColor3f(0.f, 0.7f, 0.f);
        GLUtils::Rectangle(x2, y2, x3, y1);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glColor3f(0.f, 0.f, 0.f);
        GLUtils::Rectangle(x1, y2, x3, y1);

    }

    glColor3f(1.f, 1.f, 1.f);
    glBegin(GL_LINES),
        glVertex2f (x+info.Progress*scale, y1+2),
        glVertex2f (x+info.Progress*scale, y2-3);
    glEnd();

    const GLFont* pFont = g_FontMgr.GetFont(m_Font);
    pFont->PrintF(5.f, Height-40.f, 0.f, "Frame: %d/%d | Progress: %d/%d (%d/%d)",
        info.FrameNo, currentAnimation->frameC,
        info.Progress, info.Duration,
        currentAnimation->progress, currentAnimation->frameCurrent->freeze+currentAnimation->frameCurrent->duration);
}

/************************** SELECTIONS *************************************/
void SceneSkeleton::RenderSelect()
{
    if (m_EditMode == emCreateBone || m_EditMode == emSelectBone || m_EditMode == emAnimateBones)
        m_Model.GetRenderer()->RenderSkeleton(true);
    else
    if (m_EditMode == emSelectElement)
        m_Model.GetRenderer()->RenderVertices(xRender::smElement);
    else
    if (m_EditMode == emSelectVertex)
        m_Model.GetRenderer()->RenderVertices(xRender::smVertex, selectedElemID);
}
unsigned int SceneSkeleton::CountSelectable()
{
    if (m_EditMode == emCreateBone || m_EditMode == emSelectBone || m_EditMode == emAnimateBones)
        return xBoneChildCount(m_Model.GetRenderer()->xModel->spineP) + 1;
    else
    if (m_EditMode == emSelectElement)
        return xElementCount(m_Model.GetRenderer()->xModel);
    else
    if (m_EditMode == emSelectVertex)
        return selectedElement->verticesC;
    return 0;
}
std::vector<xDWORD> *SceneSkeleton::SelectCommon(int X, int Y, int W, int H)
{
    glViewport(Left, Top, Width, Height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    xglPerspective(45.f, AspectRatio, 0.1f, 1000.f);
    glMatrixMode(GL_MODELVIEW);
    Camera_Aim_GL(*m_Cameras.Current);

    return ISelectionProvider::Select(X, Y, W, H);
}

xBone *SceneSkeleton::SelectBone(int X, int Y)
{
    std::vector<xDWORD> *sel = SelectCommon(X, Y);

    if (sel && sel->size()) {
        GLuint id = sel->back();
        delete sel;
        return xBoneById(m_Model.GetRenderer()->xModel->spineP, id);
    }
    delete sel;
    return NULL;
}
xDWORD SceneSkeleton::SelectElement(int X, int Y)
{
    std::vector<xDWORD> *sel = SelectCommon(X, Y);

    if (sel && sel->size()) {
        GLuint id = sel->back();
        delete sel;
        return id;
    }
    delete sel;
    return -1;
}
