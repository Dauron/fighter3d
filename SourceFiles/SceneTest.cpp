#include "SceneTest.h"

#include "../AppFramework/Application.h"
#include "../AppFramework/Input/InputMgr.h"
#include "../Graphics/OGL/Utils.h"
#include "SceneConsole.h"
#include "../Graphics/OGL/Extensions/GLExtensions.h"
#include "../Physics/Colliders/FigureCollider.h"

#define MULT_MOVE   5.0f
#define MULT_RUN    2.0f
#define MULT_ROT    80.0f
#define MULT_STEP   60.0f

using namespace Math::Figures;
using namespace Physics::Colliders;

bool SceneTest::Initialize(int left, int top, unsigned int width, unsigned int height)
{
    Scene::Initialize(left, top, width, height);

    if (DefaultCamera != &fCamera)
    {
        figures[0].clear();
        figures[0].push_back(&pf_sphere1);
        figures[0].push_back(&pf_sphere2);

        figures[1].clear();
        figures[1].push_back(&pf_sphere1);
        figures[1].push_back(&pf_capsule1);

        figures[2].clear();
        figures[2].push_back(&pf_capsule1);
        figures[2].push_back(&pf_capsule2);

        figures[3].clear();
        figures[3].push_back(&pf_cube1);
        figures[3].push_back(&pf_cube2);

        figures[4].clear();
        figures[4].push_back(&pf_cube1);
        figures[4].push_back(&pf_sphere2);

        figures[5].clear();
        figures[5].push_back(&pf_cube1);
        figures[5].push_back(&pf_capsule1);

        figures[6].clear();
        figures[6].push_back(&pf_mesh1);
        figures[6].push_back(&pf_sphere2);

        figures[7].clear();
        figures[7].push_back(&pf_mesh1);
        figures[7].push_back(&pf_cube2);

        figures[8].clear();
        figures[8].push_back(&pf_mesh1);
        figures[8].push_back(&pf_capsule1);

        figures[9].clear();
        figures[9].push_back(&pf_mesh1);
        figures[9].push_back(&pf_mesh2);

        fCamera.SetCamera(0.0f, 5.0f, 2.2f, 0.0f, 0.0f, 2.2f, 0.0f, 0.0f, 1.0f);
        DefaultCamera = &fCamera;
        InitObjects();
        Config::TestCase = 0;
        selected         = -1;
        FL_pause         = false;
    }
    InitInputMgr();
    FOV.init(45.0f, AspectRatio, 0.1f, 1000.0f);

    FL_mouse_down = false;

    return InitGL();
}

void SceneTest::InitObjects()
{
    xSphere &sphere1 = *(xSphere*) pf_sphere1.BVHierarchy.Figure;
    pf_sphere1.ApplyDefaults();
    sphere1.P_center.init(-3,-5,2);
    sphere1.S_radius = 1.f;
    pf_sphere1.Initialize();
    xSphere &sphere2 = *(xSphere*) pf_sphere2.BVHierarchy.Figure;
    pf_sphere2.ApplyDefaults();
    sphere2.P_center.init(3,-5,0);
    sphere2.S_radius = 2.f;
    pf_sphere2.Initialize();

    xCapsule &capsule1 = *(xCapsule*) pf_capsule1.BVHierarchy.Figure;
    pf_capsule1.ApplyDefaults();
    capsule1.P_center.init(3, -5, 0);
    capsule1.N_top.init(1,0,0);
    capsule1.S_radius = 0.5f;
    capsule1.S_top    = 2.f;
    pf_capsule1.Initialize();
    xCapsule &capsule2 = *(xCapsule*) pf_capsule2.BVHierarchy.Figure;
    pf_capsule2.ApplyDefaults();
    capsule2.P_center.init(-3, -5, 0);
    capsule2.N_top.init(0,0,1);
    capsule2.S_radius = 1.5f;
    capsule2.S_top    = 1.f;
    pf_capsule2.Initialize();

    xBoxO &cube1 = *(xBoxO*) pf_cube1.BVHierarchy.Figure;
    pf_cube1.ApplyDefaults();
    cube1.P_center.init(-3,-5,-2);
    cube1.S_top   = 1.f;
    cube1.S_front = 2.f;
    cube1.S_side  = 0.5f;
    cube1.N_top.init(0,0,1);
    cube1.N_side.init(1,0,0);
    cube1.N_front = xVector3::CrossProduct(cube1.N_top, cube1.N_side);
    pf_cube1.Initialize();
    xBoxO &cube2 = *(xBoxO*) pf_cube2.BVHierarchy.Figure;
    pf_cube2.ApplyDefaults();
    cube2.P_center.init(3,-5,0);
    cube2.S_top   = 1.2f;
    cube2.S_front = 1.7f;
    cube2.S_side  = 1.2f;
    cube2.N_top.init(0,0,1);
    cube2.N_side.init(1,0,0);
    cube2.N_front = xVector3::CrossProduct(cube2.N_top, cube2.N_side);
    pf_cube2.Initialize();

    xMesh &mesh1 = *(xMesh*) pf_mesh1.BVHierarchy.Figure;
    if (!mesh1.MeshData)
    {
        mesh1.I_FaceIndices = 3;
        mesh1.L_FaceIndices = new xDWORD[3];
        mesh1.L_FaceIndices[0] = 0;
        mesh1.L_FaceIndices[1] = 1;
        mesh1.L_FaceIndices[2] = 2;

        mesh1.I_VertexIndices = 4;
        mesh1.L_VertexIndices = new xDWORD[4];
        mesh1.L_VertexIndices[0] = 0;
        mesh1.L_VertexIndices[1] = 1;
        mesh1.L_VertexIndices[2] = 2;
        mesh1.L_VertexIndices[3] = 3;

        mesh1.MeshData = new xMeshData();
        mesh1.MeshData->MX_MeshToLocal.identity();

        xPoint3 *P_vertices = new xPoint3[4];
        P_vertices[0].init(-3,-5,2);
        P_vertices[1].init(-3,-2,1);
        P_vertices[2].init(-4,-2,1.5f);
        P_vertices[3].init(-4,-3,1.5f);
        mesh1.MeshData->L_VertexData   = (xBYTE*) P_vertices;
        mesh1.MeshData->I_VertexCount  = 4;
        mesh1.MeshData->I_VertexStride = sizeof(xPoint3);

        xWORD3 *ID_face = new xWORD3[3];
        ID_face[0][0] = 0;
        ID_face[0][1] = 1;
        ID_face[0][2] = 2;
        ID_face[1][0] = 0;
        ID_face[1][1] = 1;
        ID_face[1][2] = 3;
        ID_face[2][0] = 1;
        ID_face[2][1] = 2;
        ID_face[2][2] = 3;
        mesh1.MeshData->L_FaceData   = (xBYTE*) ID_face;
        mesh1.MeshData->I_FaceCount  = 3;
        mesh1.MeshData->I_FaceStride = sizeof(xWORD3);
    }
    mesh1.Transform(xMatrix::Identity());
    mesh1.MeshData->CalculateProperties();
    pf_mesh1.ApplyDefaults();
    mesh1.P_center = mesh1.MeshData->P_center;
    pf_mesh1.Initialize();

    xMesh &mesh2 = *(xMesh*) pf_mesh2.BVHierarchy.Figure;
    if (!mesh2.MeshData)
    {
        mesh2.I_FaceIndices = 3;
        mesh2.L_FaceIndices = new xDWORD[3];
        mesh2.L_FaceIndices[0] = 0;
        mesh2.L_FaceIndices[1] = 1;
        mesh2.L_FaceIndices[2] = 2;

        mesh2.I_VertexIndices = 4;
        mesh2.L_VertexIndices = new xDWORD[4];
        mesh2.L_VertexIndices[0] = 0;
        mesh2.L_VertexIndices[1] = 1;
        mesh2.L_VertexIndices[2] = 2;
        mesh2.L_VertexIndices[3] = 3;

        mesh2.MeshData = new xMeshData();
        mesh2.MeshData->MX_MeshToLocal.identity();

        xPoint3 *P_vertices = new xPoint3[4];
        P_vertices[0].init(3,-3,2);
        P_vertices[1].init(5,-2,1);
        P_vertices[2].init(4,-2,1.5f);
        P_vertices[3].init(4,-3,1.5f);
        mesh2.MeshData->L_VertexData   = (xBYTE*) P_vertices;
        mesh2.MeshData->I_VertexCount  = 4;
        mesh2.MeshData->I_VertexStride = sizeof(xPoint3);

        xWORD3 *ID_face = new xWORD3[3];
        ID_face[0][0] = 0;
        ID_face[0][1] = 1;
        ID_face[0][2] = 2;
        ID_face[1][0] = 0;
        ID_face[1][1] = 1;
        ID_face[1][2] = 3;
        ID_face[2][0] = 1;
        ID_face[2][1] = 2;
        ID_face[2][2] = 3;
        mesh2.MeshData->L_FaceData   = (xBYTE*) ID_face;
        mesh2.MeshData->I_FaceCount  = 3;
        mesh2.MeshData->I_FaceStride = sizeof(xWORD3);
    }
    mesh2.Transform(xMatrix::Identity());
    mesh2.MeshData->CalculateProperties();
    pf_mesh2.ApplyDefaults();
    mesh2.P_center = mesh2.MeshData->P_center;
    pf_mesh2.Initialize();

    if (Config::TestCase < 0) Config::TestCase = 0;
    if (Config::TestCase > 9) Config::TestCase = 9;

    figures[Config::TestCase][0]->FrameUpdate(0);
    figures[Config::TestCase][1]->FrameUpdate(0);
}

bool SceneTest::InitGL()
{
    glClearDepth(1.f);                      // Mapped draw distance ([0-1])
    glDepthFunc(GL_LEQUAL);                 // Depth testing function

    glDisable(GL_CULL_FACE);                // Do not draw hidden faces
    glCullFace (GL_BACK);                   // Hide back faces
    glFrontFace(GL_CCW);                    // Front faces are drawn in counter-clockwise direction

    glShadeModel(GL_SMOOTH);                // GL_SMOOTH - enable smooth shading, GL_FLAT - no gradient on faces
    glEnable (GL_POINT_SMOOTH);

    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); // Nice perspective calculations

    GLExtensions::SetVSync(false);

    return true;
}

bool SceneTest::Invalidate()
{
    return true;
}

void SceneTest::InitInputMgr()
{
    InputMgr &im = g_InputMgr;
    im.SetScene(sceneName);

    im.SetInputCode(VK_RETURN, IC_Accept);
    im.SetInputCode(VK_ESCAPE, IC_Reject);
#ifdef WIN32
    im.SetInputCode(VK_OEM_3,  IC_Console);
#else
    im.SetInputCode('`',       IC_Console);
#endif
    im.SetInputCode(VK_LBUTTON, IC_LClick);

    im.SetInputCode(VK_UP, IC_TurnUp);
    im.SetInputCode(VK_DOWN, IC_TurnDown);
    im.SetInputCode(VK_LEFT, IC_TurnLeft);
    im.SetInputCode(VK_RIGHT, IC_TurnRight);
    im.SetInputCode('Q', IC_RollLeft);
    im.SetInputCode('E', IC_RollRight);

    im.SetInputCode('U', IC_OrbitUp);
    im.SetInputCode('J', IC_OrbitDown);
    im.SetInputCode('H', IC_OrbitLeft);
    im.SetInputCode('K', IC_OrbitRight);

    im.SetInputCode('W', IC_MoveForward);
    im.SetInputCode('S', IC_MoveBack);
    im.SetInputCode('A', IC_MoveLeft);
    im.SetInputCode('D', IC_MoveRight);
    im.SetInputCode('R', IC_MoveUp);
    im.SetInputCode('F', IC_MoveDown);
    im.SetInputCode(VK_LSHIFT, IC_RunModifier);

    im.SetInputCode(VK_SPACE,  IC_TS_Pause);
    im.SetInputCode(VK_TAB,    IC_CameraReset);
    im.SetInputCode(VK_RETURN, IC_TS_Stop);
    im.SetInputCode('0', IC_TS_Test0);
    im.SetInputCode('1', IC_TS_Test1);
    im.SetInputCode('2', IC_TS_Test2);
    im.SetInputCode('3', IC_TS_Test3);
    im.SetInputCode('4', IC_TS_Test4);
    im.SetInputCode('5', IC_TS_Test5);
    im.SetInputCode('6', IC_TS_Test6);
    im.SetInputCode('7', IC_TS_Test7);
    im.SetInputCode('8', IC_TS_Test8);
    im.SetInputCode('9', IC_TS_Test9);

    im.SetInputCode(VK_F11, IC_FullScreen);
}

void SceneTest::Terminate()
{
    DefaultCamera = NULL;
    Scene::Terminate();
}


xVector3 SceneTest::Get3dPos(int X, int Y, xVector3 P_onPlane)
{
    float norm_x = 1.0f - (float)X/(Width/2.0f);
    float norm_y = (float)Y/(Height/2.0f) - 1.0f;

    // get model view matrix
    xMatrix MX_ViewToModel;
    DefaultCamera->LookAtMatrix(MX_ViewToModel);
    MX_ViewToModel.invert();

    // get ray of the mouse
    xVector3 N_ray;
    xVector3 P_ray;

    float near_height = FOV.FrontClip * tan(DegToRad(FOV.Angle)*0.5f);
    P_ray = MX_ViewToModel.preTransformP(xVector3::Create(0.0f,0.0f,0.0f));
    N_ray.init(near_height * AspectRatio * norm_x, near_height * norm_y, FOV.FrontClip);
    N_ray = MX_ViewToModel.preTransformV(N_ray);

    // get plane of ray intersection
    xPlane PN_plane; PN_plane.init(
        (DefaultCamera->eye-DefaultCamera->center).normalize(), P_onPlane);
    return PN_plane.intersectRay(P_ray, N_ray);
}

bool SceneTest::Update(float deltaTime)
{
    InputMgr &im = g_InputMgr;

    if (im.GetInputStateAndClear(IC_Reject))
    {
        g_Application.MainWindow().Terminate();
        return true;
    }

    if (im.GetInputStateAndClear(IC_FullScreen))
        g_Application.MainWindow().SetFullScreen(!g_Application.MainWindow().FullScreen());

    if (im.GetInputStateAndClear(IC_Console))
        g_Application.SetCurrentScene(new SceneConsole(this), false);

    float run = (im.GetInputState(IC_RunModifier)) ? MULT_RUN : 1.0f;
    float deltaTmp = deltaTime*MULT_ROT*run;

    if (im.GetInputState(IC_TurnLeft))
        DefaultCamera->Rotate (deltaTmp, 0.0f, 0.0f);
    if (im.GetInputState(IC_TurnRight))
        DefaultCamera->Rotate (-deltaTmp, 0.0f, 0.0f);
    if (im.GetInputState(IC_TurnUp))
        DefaultCamera->Rotate (0.0f, deltaTmp, 0.0f);
    if (im.GetInputState(IC_TurnDown))
        DefaultCamera->Rotate (0.0f, -deltaTmp, 0.0f);
    if (im.GetInputState(IC_RollLeft))
        DefaultCamera->Rotate (0.0f, 0.0f, -deltaTmp);
    if (im.GetInputState(IC_RollRight))
        DefaultCamera->Rotate (0.0f, 0.0f, deltaTmp);

    if (im.GetInputState(IC_OrbitLeft))
        DefaultCamera->Orbit (deltaTmp, 0.0f);
    if (im.GetInputState(IC_OrbitRight))
        DefaultCamera->Orbit (-deltaTmp, 0.0f);
    if (im.GetInputState(IC_OrbitUp))
        DefaultCamera->Orbit (0.0f, deltaTmp);
    if (im.GetInputState(IC_OrbitDown))
        DefaultCamera->Orbit (0.0f, -deltaTmp);

    deltaTmp = deltaTime*MULT_MOVE*run;

    if (im.GetInputState(IC_MoveForward))
        DefaultCamera->Move (deltaTmp, 0.0f, 0.0f);
    if (im.GetInputState(IC_MoveBack))
        DefaultCamera->Move (-deltaTmp, 0.0f, 0.0f);
    if (im.GetInputState(IC_MoveLeft))
        DefaultCamera->Move (0.0f, -deltaTmp, 0.0f);
    if (im.GetInputState(IC_MoveRight))
        DefaultCamera->Move (0.0f, deltaTmp, 0.0f);
    if (im.GetInputState(IC_MoveUp))
        DefaultCamera->Move (0.0f, 0.0f, deltaTmp);
    if (im.GetInputState(IC_MoveDown))
        DefaultCamera->Move (0.0f, 0.0f, -deltaTmp);

    if (im.GetInputStateAndClear(IC_CameraReset))
    {
        fCamera.SetCamera(0.0f, 5.0f, 2.2f, 0.0f, 0.0f, 2.2f, 0.0f, 0.0f, 1.0f);
        DefaultCamera = &fCamera;
    }
    if (im.GetInputStateAndClear(IC_TS_Stop))
    {
        figures[Config::TestCase][0]->Stop();
        figures[Config::TestCase][1]->Stop();
    }
    if (im.GetInputStateAndClear(IC_TS_Pause))
        FL_pause = !FL_pause;
    for (int i = 0; i < 10; ++i)
        if (im.GetInputStateAndClear(IC_TS_Test0+i))
        { Config::TestCase = i; InitObjects(); }

    if (im.GetInputState(IC_LClick))
    {
        if (FL_mouse_down)
        {
            Physics::PhysicalFigure &p_figure = *(Physics::PhysicalFigure*)figures[Config::TestCase][selected];
            if (selectedSub == 0)
            {
                xVector3 P_currMouse = Get3dPos(g_InputMgr.mouseX, g_InputMgr.mouseY, p_figure.P_center_Trfm);
                xVector3 NW_shift = P_currMouse - P_prevMouse;
                p_figure.MX_LocalToWorld_Set().postTranslateT(NW_shift);
                P_prevMouse = P_currMouse;
            }
            else
            if (p_figure.BVHierarchy.Figure->Type == xIFigure3d::Capsule)
            {
                xCapsule &object   = *(xCapsule*) p_figure.BVHierarchy.Figure;
                xCapsule &object_T = *(xCapsule*) p_figure.BVHierarchy.GetTransformed();

                if (selectedSub == 1)
                {
                    xVector3 P_topCap = object_T.P_center + object_T.S_top * object_T.N_top;
                    xVector3 P_mouse2 = Get3dPos(g_InputMgr.mouseX, g_InputMgr.mouseY, P_topCap);
                    object.N_top = P_mouse2 - object_T.P_center;
                }
                else
                {
                    xVector3 P_topCap = object.P_center - object.S_top * object.N_top;
                    xVector3 P_mouse2 = Get3dPos(g_InputMgr.mouseX, g_InputMgr.mouseY, P_topCap);
                    object.N_top = object_T.P_center - P_mouse2;
                }
                object.S_top = object.N_top.length();
                object.N_top /= object.S_top;
                object.N_top = xMatrix::Invert(p_figure.MX_LocalToWorld_Get()).preTransformV(object.N_top);
            }
            else
            if (p_figure.BVHierarchy.Figure->Type == xIFigure3d::BoxOriented)
            {
                xBoxO &object   = *(xBoxO*) p_figure.BVHierarchy.Figure;
                xBoxO &object_T = *(xBoxO*) p_figure.BVHierarchy.GetTransformed();

                xQuaternion QT_rotation;

                if (selectedSub == 1)
                {
                    xPoint3 P_topCap = object_T.P_center + object_T.S_front * object_T.N_front;
                    xPoint3 P_mouse2 = Get3dPos(g_InputMgr.mouseX, g_InputMgr.mouseY, P_topCap);
                    QT_rotation = xQuaternion::getRotation(object_T.N_front, P_mouse2-object_T.P_center);
                    object.S_front = (P_mouse2 - object_T.P_center).length();
                }
                else
                if (selectedSub == 2)
                {
                    xPoint3 P_topCap = object_T.P_center + object_T.S_side * object_T.N_side;
                    xPoint3 P_mouse2 = Get3dPos(g_InputMgr.mouseX, g_InputMgr.mouseY, P_topCap);
                    QT_rotation = xQuaternion::getRotation(object_T.N_side, P_mouse2-object_T.P_center);
                    object.S_side = (P_mouse2 - object_T.P_center).length();
                }
                else
                {
                    xPoint3 P_topCap = object_T.P_center + object_T.S_top * object_T.N_top;
                    xPoint3 P_mouse2 = Get3dPos(g_InputMgr.mouseX, g_InputMgr.mouseY, P_topCap);
                    QT_rotation = xQuaternion::getRotation(object_T.N_top, P_mouse2-object_T.P_center);
                    object.S_top = (P_mouse2 - object_T.P_center).length();
                }
                xMatrix MX_WorldToLocal = xMatrix::Invert(p_figure.MX_LocalToWorld_Get());
                object.N_top   = xQuaternion::rotate(QT_rotation, object.N_top).normalize();
                object.N_side  = xQuaternion::rotate(QT_rotation, object.N_side).normalize();
                object.N_front = xQuaternion::rotate(QT_rotation, object.N_front ).normalize();
                object.N_top = MX_WorldToLocal.preTransformV(object.N_top);
                object.N_side = MX_WorldToLocal.preTransformV(object.N_side);
                object.N_front = MX_WorldToLocal.preTransformV(object.N_front);
            }
            p_figure.Modify();
        }
        else
        {
            glViewport(Left, Top, Width, Height);
            // Set projection
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            xglPerspective(FOV);
            glMatrixMode(GL_MODELVIEW);
            DefaultCamera->LookAtMatrix(FOV.ViewTransform);
            glLoadMatrixf(&FOV.ViewTransform.x0);
            selected = Select(g_InputMgr.mouseX, g_InputMgr.mouseY);

            if (selected != -1)
            {
                FL_mouse_down = true;
                Physics::PhysicalFigure &p_figure = *(Physics::PhysicalFigure*)figures[Config::TestCase][selected];
                p_figure.Stop();

                selectedSub = 0;
                T_total = 0.f;
                P_firstMouse = P_prevMouse = Get3dPos(g_InputMgr.mouseX, g_InputMgr.mouseY, p_figure.P_center_Trfm);
                xFLOAT S_dist = (P_prevMouse - p_figure.P_center_Trfm).lengthSqr();

                if (p_figure.BVHierarchy.Figure->Type == xIFigure3d::Capsule)
                {
                    const xCapsule &object_T = *(xCapsule*) p_figure.BVHierarchy.GetTransformed();

                    xVector3 P_topCap = object_T.P_center + object_T.S_top * object_T.N_top;
                    xVector3 P_mouse2 = Get3dPos(g_InputMgr.mouseX, g_InputMgr.mouseY, P_topCap);
                    xFLOAT   S_dist2  = (P_mouse2 - P_topCap).lengthSqr();

                    if (S_dist2 < S_dist)
                    {
                        selectedSub = 1;
                        P_prevMouse = P_mouse2;
                    }
                    else
                    {
                        P_topCap = object_T.P_center - object_T.S_top * object_T.N_top;
                        P_mouse2 = Get3dPos(g_InputMgr.mouseX, g_InputMgr.mouseY, P_topCap);
                        S_dist2  = (P_mouse2 - P_topCap).lengthSqr();
                        if (S_dist2 < S_dist)
                        {
                            selectedSub = 2;
                            P_prevMouse = P_mouse2;
                        }
                    }
                }

                if (p_figure.BVHierarchy.Figure->Type == xIFigure3d::BoxOriented)
                {
                    const xBoxO &object_T = *(xBoxO*) p_figure.BVHierarchy.GetTransformed();

                    xVector3 P_topCap = object_T.P_center + object_T.S_top * object_T.N_top;
                    xVector3 P_mouse2 = Get3dPos(g_InputMgr.mouseX, g_InputMgr.mouseY, P_topCap);
                    xFLOAT   S_dist2  = (P_mouse2 - P_topCap).lengthSqr();

                    if (S_dist2 < S_dist)
                    {
                        S_dist = S_dist2;
                        selectedSub = 3;
                        P_prevMouse = P_mouse2;
                    }

                    P_topCap = object_T.P_center + object_T.S_side * object_T.N_side;
                    P_mouse2 = Get3dPos(g_InputMgr.mouseX, g_InputMgr.mouseY, P_topCap);
                    S_dist2  = (P_mouse2 - P_topCap).lengthSqr();

                    if (S_dist2 < S_dist)
                    {
                        S_dist = S_dist2;
                        selectedSub = 2;
                        P_prevMouse = P_mouse2;
                    }

                    P_topCap = object_T.P_center + object_T.S_front * object_T.N_front;
                    P_mouse2 = Get3dPos(g_InputMgr.mouseX, g_InputMgr.mouseY, P_topCap);
                    S_dist2  = (P_mouse2 - P_topCap).lengthSqr();

                    if (S_dist2 < S_dist)
                    {
                        S_dist = S_dist2;
                        selectedSub = 1;
                        P_prevMouse = P_mouse2;
                    }
                }
            }
        }
        figures[Config::TestCase][0]->FrameUpdate(0);
        figures[Config::TestCase][1]->FrameUpdate(0);
    }

    T_total += deltaTime;
    if (!FL_mouse_down && !FL_pause)
        Physics::PhysicalWorld().Interact(deltaTime, figures[Config::TestCase]);

    if (!im.GetInputState(IC_LClick) && FL_mouse_down)
    {
        FL_mouse_down = false;

        if ((selected == 0 || selected == 1) && selectedSub == 0)
        {
            xVector3 P_currMouse = Get3dPos(g_InputMgr.mouseX, g_InputMgr.mouseY, P_firstMouse);
            xVector3 NW_shift_total = P_currMouse - P_firstMouse;
            figures[Config::TestCase][selected]->ApplyAcceleration(NW_shift_total*10.f, 1.f);
        }

        CollisionSet cs, cs2;
        xVector3 NW_fix_1;
        if (false && FigureCollider().Collide(figures[Config::TestCase][0]->BVHierarchy.GetTransformed(), figures[Config::TestCase][1]->BVHierarchy.GetTransformed(), cs))
        {
            xPoint3  P1, P2;
            cs.MergeCollisions(P1, P2, NW_fix_1);
            if (selected == 0)
                figures[Config::TestCase][1]->MX_LocalToWorld_Set().postTranslateT(-NW_fix_1);
            else
                figures[Config::TestCase][0]->MX_LocalToWorld_Set().postTranslateT(NW_fix_1);
            figures[Config::TestCase][0]->FrameUpdate(0);
            figures[Config::TestCase][1]->FrameUpdate(0);
        }
        if (false && FigureCollider().Collide(figures[Config::TestCase][0]->BVHierarchy.GetTransformed(), figures[Config::TestCase][1]->BVHierarchy.GetTransformed(), cs2))
        {
            //xPoint3  P1, P2;
            //xVector3 NW_fix_1;
            //cs.MergeCollisions(P1, P2, NW_fix_1);
            //if (selected == 0)
            //    figures[Config::TestCase][1]->MX_LocalToWorld.postTranslateT(-NW_fix_1);
            //else
            //    figures[Config::TestCase][0]->MX_LocalToWorld.postTranslateT(+NW_fix_1);
            figures[Config::TestCase][0]->FrameUpdate(0);
            figures[Config::TestCase][1]->FrameUpdate(0);
        }
    }

    return true;
}


void RenderFigure(const xIFigure3d *figure)
{
    glPushMatrix();
    glTranslatef(figure->P_center.x,figure->P_center.y,figure->P_center.z);

    ////////////////////////////// Sphere

    if (figure->Type == xIFigure3d::Sphere)
    {
        const xSphere &object = *((xSphere*) figure);

        const xBYTE I_steps = 20;
        xQuaternion QT_step; QT_step.init(sin(PI / I_steps), 0,0, cos(PI / I_steps));
        xQuaternion QT_bigs; QT_bigs.init(0, 0, sin(PI / (2*I_steps)), cos(PI / (2*I_steps)));
        xMatrix  MX_bigs = xMatrixFromQuaternion(QT_bigs);

        xPoint3 P_points[I_steps];
        P_points[0] = xVector3::Create(0,1,0) * object.S_radius;
        for (int i = 1; i < I_steps; ++i)
            P_points[i] = xQuaternion::rotate(QT_step, P_points[i-1]);

        for (int i = 0; i < I_steps; ++i)
        {
            glBegin(GL_POLYGON);
            for (int j = 0; j < I_steps; ++j)
                glVertex3fv(P_points[j].xyz);
            glEnd();

            glMultMatrixf(&MX_bigs.x0);
        }
    }

    ////////////////////////////// Capsule

    if (figure->Type == xIFigure3d::Capsule)
    {
        const xCapsule &object = *((xCapsule*) figure);

        const xBYTE I_steps = 10;
        xVector3 N_side = xVector3::CrossProduct(object.N_top, xVector3::Create(0,1,0));
        if (N_side.lengthSqr() < EPSILON2)
            N_side = xVector3::CrossProduct(object.N_top, xVector3::Create(1,0,0));
        N_side.normalize();

        xQuaternion QT_step; QT_step.init(xVector3::CrossProduct(object.N_top, N_side) * -sin(PI / (2*I_steps)),
                                       cos(PI / (2*I_steps)));
        xQuaternion QT_bigs = QT_bigs.init(object.N_top * sin(PI / (2*I_steps)), cos(PI / (2*I_steps)));
        xMatrix     MX_bigs = xMatrixFromQuaternion(QT_bigs);

        xVector3 P_points[I_steps * 2];
        P_points[0] = N_side * object.S_radius;
        for (int i = 1; i < I_steps; ++i)
            P_points[i] = xQuaternion::rotate(QT_step, P_points[i-1]);

        P_points[I_steps] = -N_side * object.S_radius;
        for (int i = I_steps+1; i < 2*I_steps; ++i)
            P_points[i] = xQuaternion::rotate(QT_step, P_points[i-1]);

        xVector3 NW_top = object.N_top * object.S_top;

        for (int i = 0; i < I_steps; ++i)
        {
            glBegin(GL_POLYGON);
            for (int j = 0; j < I_steps; ++j)
                glVertex3fv((P_points[j] + NW_top).xyz);
            for (int j = I_steps; j < 2*I_steps; ++j)
                glVertex3fv((P_points[j] - NW_top).xyz);
            glEnd();

            glMultMatrixf(&MX_bigs.x0);
        }
    }

    ////////////////////////////// Cube

    if (figure->Type == xIFigure3d::BoxOriented)
    {
        const xBoxO &object = *((xBoxO*) figure);

        glBegin(GL_LINES);
            glVertex3f(0,0,0);
            glVertex3fv((object.N_top*object.S_top).xyz);
            glVertex3f(0,0,0);
            glVertex3fv((object.N_side*object.S_side).xyz);
            glVertex3f(0,0,0);
            glVertex3fv((object.N_front*object.S_front).xyz);
        glEnd();

        const xBYTE I_steps = 10;
        xVector3 P_points[4];
        P_points[0] = object.N_top*object.S_top + object.N_side*object.S_side;
        P_points[1] = object.N_top*object.S_top - object.N_side*object.S_side;
        P_points[2] = -object.N_top*object.S_top - object.N_side*object.S_side;
        P_points[3] = -object.N_top*object.S_top + object.N_side*object.S_side;

        xVector3 NW_shift = object.N_front*object.S_front;
        glPushMatrix();
        glTranslatef(NW_shift.x,NW_shift.y,NW_shift.z);
        NW_shift /= -I_steps*0.5f;
        for (int i = 0; i < I_steps+1; ++i)
        {
            glBegin(GL_QUADS);
            for (int j = 0; j < 4; ++j)
                glVertex3fv(P_points[j].xyz);
            glEnd();

            glTranslatef(NW_shift.x,NW_shift.y,NW_shift.z);
        }
        glPopMatrix();

        P_points[0] = object.N_top*object.S_top + object.N_front*object.S_front;
        P_points[1] = object.N_top*object.S_top - object.N_front*object.S_front;
        P_points[2] = -object.N_top*object.S_top - object.N_front*object.S_front;
        P_points[3] = -object.N_top*object.S_top + object.N_front*object.S_front;

        NW_shift = object.N_side*object.S_side;
        glTranslatef(NW_shift.x,NW_shift.y,NW_shift.z);
        NW_shift /= -I_steps*0.5f;
        for (int i = 0; i < I_steps+1; ++i)
        {
            glBegin(GL_QUADS);
            for (int j = 0; j < 4; ++j)
                glVertex3fv(P_points[j].xyz);
            glEnd();

            glTranslatef(NW_shift.x,NW_shift.y,NW_shift.z);
        }
    }

    ////////////////////////////// Cylinder

    if (figure->Type == xIFigure3d::Cylinder)
    {
        const xCylinder &object = *((xCylinder*) figure);
    }

    ////////////////////////////// Mesh

    if (figure->Type == xIFigure3d::Mesh)
    {
        const xMesh &object = *((xMesh*) figure);

        glPopMatrix();

        glBegin(GL_TRIANGLES);

        xDWORD  *L_FaceIndex_Itr     = object.L_FaceIndices;
        for (int i = object.I_FaceIndices; i; --i, ++L_FaceIndex_Itr)
        {
            xWORD3 &Face = object.MeshData->GetFace(*L_FaceIndex_Itr);
            for (int j=0; j<3; ++j)
            {
                if (j == 1) glColor3f(0.f,0.f,1.f);
                else
                if (j == 2) glColor3f(0.f,1.f,1.f);
                glVertex3fv(object.MeshData->L_VertexData_Transf[Face[j]].xyz);
            }
        }

        glPushMatrix();


        glBegin(GL_LINES);

        L_FaceIndex_Itr     = object.L_FaceIndices;
        for (int i = object.I_FaceIndices; i; --i, ++L_FaceIndex_Itr)
        {
            xWORD3 &Face = object.MeshData->GetFace(*L_FaceIndex_Itr);
            glColor3f(1.f,0.f,1.f);

            const xPoint3 &P_A = object.MeshData->GetVertexTransf(Face[0]);
            const xPoint3 &P_B = object.MeshData->GetVertexTransf(Face[1]);
            const xPoint3 &P_C = object.MeshData->GetVertexTransf(Face[2]);
            xVector3 N_tri = xVector3::CrossProduct(P_B-P_A, P_C-P_A).normalize() * 0.1f;
            glVertex3fv(P_A.xyz);
            glVertex3fv(P_A.xyz);
            glVertex3fv((P_A + N_tri).xyz);
        }

        glPushMatrix();

        glEnd();
    }

    glPopMatrix();
}

bool SceneTest::Render()
{
    if (Config::Initialize)
    {
        Config::Initialize = false;
        InitObjects();
    }

    glDisable (GL_LINE_SMOOTH);
    glDisable (GL_POLYGON_SMOOTH);                    // produces errors on many cards... use FSAA!
    glDisable(GL_LIGHT0); glDisable(GL_LIGHT1); glDisable(GL_LIGHT2); glDisable(GL_LIGHT3);
    glDisable(GL_LIGHT4); glDisable(GL_LIGHT5); glDisable(GL_LIGHT6); glDisable(GL_LIGHT7);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glViewport(Left, Top, Width, Height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    xglPerspectiveInf(FOV);
    glMatrixMode(GL_MODELVIEW);
    DefaultCamera->LookAtMatrix(FOV.ViewTransform);
    glLoadMatrixf(&FOV.ViewTransform.x0);
    FOV.update();

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glShadeModel(GL_SMOOTH);
    glDisable(GL_DEPTH_TEST);

    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
    glDepthMask(1);
    glColorMask(1,1,1,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ::Physics::Colliders::CollisionSet cset;

    for (int i = 0; i < 2; ++i)
    {
        if (i == selected)
            glColor3f(1.f, 1.f, 0.f);
        else
        if (selected != -1 && FigureCollider().Collide( figures[Config::TestCase][0]->BVHierarchy.GetTransformed(),
                                                        figures[Config::TestCase][1]->BVHierarchy.GetTransformed(), cset) )
            glColor3f(1.f, 0.f, 0.f);
        else
            glColor3f(1.f, 1.f, 1.f);
        RenderFigure(figures[Config::TestCase][i]->BVHierarchy.GetTransformed());
    }

    if (cset.collisions.size())
    {
        glPointSize(5.f);

        ::Physics::Colliders::CollisionSet::CollisionVec::iterator iter, end = cset.collisions.end();

        for (iter = cset.collisions.begin(); iter != end; ++iter)
        {
            glBegin(GL_POINTS);
            {
                glColor3f(1.f,0.f,1.f);
                glVertex3fv(iter->P_collision_1.xyz);
                glColor3f(0.5f,0.5f,1.f);
                glVertex3fv(iter->P_collision_2.xyz);
            }
            glEnd();

            glColor3f(0.5f,0.5f,0.5f);
            glBegin(GL_LINES);
            {
                glVertex3fv(iter->P_collision_1.xyz);
                glVertex3fv((iter->P_collision_1 + iter->NW_fix_1).xyz);
            }
            glEnd();
        }

        glPointSize(5.f);
    }

    if (FL_mouse_down)
    {
        glColor3f(0.5f,1.f,0.5f);
        glBegin(GL_LINES);
        {
            glVertex3fv(P_firstMouse.xyz);
            glVertex3fv(Get3dPos(g_InputMgr.mouseX, g_InputMgr.mouseY, P_firstMouse).xyz);
        }
        glEnd();
    }

    glPopAttrib();

    glFlush();
    //glFinish();
    return true;
}

////// ISelectionProvider

void SceneTest :: RenderSelect(const xFieldOfView *FOV)
{
    for (int i = 0; i < 2; ++i)
    {
        glLoadName(i);
        RenderFigure(figures[Config::TestCase][i]->BVHierarchy.GetTransformed());
    }
}

unsigned int SceneTest::CountSelectable()
{
    return 2;
}