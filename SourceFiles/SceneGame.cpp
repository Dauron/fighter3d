#include "../AppFramework/System.h"
#include <GL/gl.h>
#include "SceneGame.h"
#include "SceneConsole.h"
#include "SceneSkeleton.h"
#include "../AppFramework/Input/InputMgr.h"
#include "../AppFramework/Application.h"
#include "../Math/Cameras/CameraHuman.h"
#include "../OpenGL/GLAnimSkeletal.h"
#include "LightsAndMaterials.h"

#define MULT_MOVE   5.0f
#define MULT_RUN    2.0f
#define MULT_ROT    80.0f
#define MULT_STEP   60.0f

bool SceneGame::Initialize(int left, int top, unsigned int width, unsigned int height)
{
    Scene::Initialize(left, top, width, height);

    if (DefaultCamera != &hCamera)
    {
        hCamera.SetCamera(0.0f, 5.0f, 2.2f, 0.0f, 0.0f, 2.2f, 0.0f, 0.0f, 1.0f);
        DefaultCamera = &hCamera;
    }
    stepAccum = 0.0f;
    InitInputMgr();

    if (!world.IsValid())
        world.Initialize();
    return InitGL();
}

bool SceneGame::InitGL()
{
    glClearColor( 0.1f, 0.1f, 0.3f, 0.0f ); // Background color

    glClearDepth(100.0f);                   // Draw distance ???
    glDepthFunc(GL_LEQUAL);                 // Depth testing function

    glEnable(GL_CULL_FACE);                 // Do not draw hidden faces
    glCullFace (GL_BACK);                   // Hide back faces
    glFrontFace(GL_CCW);                    // Front faces are drawn in counter-clockwise direction

    glShadeModel(GL_SMOOTH);                // GL_SMOOTH - enable smooth shading, GL_FLAT - no gradient on faces
    glEnable (GL_POINT_SMOOTH);
    glEnable (GL_LINE_SMOOTH);
//    glEnable (GL_POLYGON_SMOOTH);         // produces errors on many cards... use FSAA!

    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); // Nice perspective calculations

    //setFog();

    //if (!shader.IsInitialized())
    {
        shader.Terminate();
        shader.Load("Data/program.vert", "Data/program.frag");
        shader.Initialize();
    }
    return true;
}

void SceneGame::InitInputMgr()
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

    im.SetInputCode(VK_F11, IC_FullScreen);
}

void SceneGame::Terminate()
{
    DefaultCamera = NULL;
    world.Finalize();
    g_FontMgr.DeleteFont(m_Font1);
    m_Font1 = HFont();
    g_FontMgr.DeleteFont(m_Font2);
    m_Font2 = HFont();
}

bool SceneGame::Update(float deltaTime)
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

    bool moving = false;
    if (im.GetInputState(IC_MoveForward))
    {
        DefaultCamera->Move (deltaTmp, 0.0f, 0.0f);
        moving = true;
    }
    if (im.GetInputState(IC_MoveBack))
    {
        DefaultCamera->Move (-deltaTmp, 0.0f, 0.0f);
        moving = true;
    }
    if (im.GetInputState(IC_MoveLeft))
    {
        DefaultCamera->Move (0.0f, -deltaTmp, 0.0f);
        moving = true;
    }
    if (im.GetInputState(IC_MoveRight))
    {
        DefaultCamera->Move (0.0f, deltaTmp, 0.0f);
        moving = true;
    }
    if (im.GetInputState(IC_MoveUp))
        DefaultCamera->Move (0.0f, 0.0f, deltaTmp);
    if (im.GetInputState(IC_MoveDown))
        DefaultCamera->Move (0.0f, 0.0f, -deltaTmp);

    if (moving && &hCamera == DefaultCamera)
    {
        stepAccum += deltaTime*MULT_STEP*run;
        if (stepAccum > 1)
        {
            hCamera.MakeStep(floorf(stepAccum));
            stepAccum = fmodf(stepAccum, 1.0f);
        }
    }

    if (im.GetInputStateAndClear(IC_LClick))
    {
        glViewport(Left, Top, Width, Height);
        // Set projection
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        xglPerspective(45.0f, AspectRatio, 0.1f, 1000.0f);
        glMatrixMode(GL_MODELVIEW);
        Camera_Aim_GL(*DefaultCamera);
        ModelObj *obj = world.Select(g_InputMgr.mouseX, g_InputMgr.mouseY);
        if (obj)
            g_Application.SetCurrentScene(new SceneSkeleton(this, obj->GetName()), false);
/*
        float near_height = 0.1f * tan(45.0f * PI / 360.0f);
        float norm_x = 1.0f - (float)g_InputMgr.mouseX/(Width/2.0f);
        float norm_y = (float)g_InputMgr.mouseY/(Height/2.0f) - 1.0f;

        // get model view matrix
        glMatrixMode(GL_MODELVIEW);
        DefaultCamera->Aim();
        xMatrix modelView;
        glGetFloatv(GL_MODELVIEW_MATRIX, &(modelView.x0));
        modelView.invert();

        // get ray of the mouse
        xVector3 rayDir(near_height * AspectRatio * norm_x, near_height * norm_y, 0.1f);
        rayDir = rayDir * modelView;
        xVector3 rayPos = (xVector4(0.0f,0.0f,0.0f,1.0f)*modelView).vector3;

        ModelObj *obj = world.CollideWithRay(rayPos, -rayDir);
        if (obj)
            g_Application.SetCurrentScene(new SceneSkeleton(this, obj->GetName()), false);
*/
        return true;
    }

    world.Update(deltaTime);
    return true;
}

bool SceneGame::Render()
{
    if (!g_FontMgr.IsHandleValid(m_Font1))
        m_Font1 = g_FontMgr.GetFont("Courier New", 12);
    if (!g_FontMgr.IsHandleValid(m_Font2))
        m_Font2 = g_FontMgr.GetFont("Courier New", 15);

    if (g_UseCustomShader && shader.IsInitialized())
        shader.Start();

    glViewport(Left, Top, Width, Height);
    // Clear surface
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT/* | GL_STENCIL_BUFFER_BIT*/);

    // Set projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    xglPerspective(45.0f, AspectRatio, 0.1f, 1000.0f);
    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_DEPTH_TEST);                // Enable depth testing
    glPolygonMode(GL_FRONT_AND_BACK, g_PolygonMode);

    Camera_Aim_GL(*DefaultCamera);
    setLights();

    if (g_Init)
    {
        world.Finalize();
        world.Initialize();
        g_Init = false;
    }
    world.Render();

    // Flush the buffer to force drawing of all objects thus far
    glFlush();

    if (g_UseCustomShader && shader.IsInitialized())
        shader.Suspend();

    return true;
}
