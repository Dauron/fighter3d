#include "../AppFramework/System.h"
#include <GL/gl.h>
#include "SceneGame.h"
#include "SceneConsole.h"
#include "SceneSkeleton.h"
#include "../AppFramework/Input/InputMgr.h"
#include "../AppFramework/Application.h"
#include "../Math/Cameras/CameraHuman.h"
#include "../OpenGL/GLAnimSkeletal.h"

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

    FOV.init(45.0f, AspectRatio, 0.1f, 1000.0f);

    if (!world.IsValid())
        world.Initialize();
    return InitGL();
}

bool SceneGame::InitGL()
{
    glClearDepth(1.f);                      // Mapped draw distance ([0-1])
    glDepthFunc(GL_LEQUAL);                 // Depth testing function

    glEnable(GL_CULL_FACE);                 // Do not draw hidden faces
    glCullFace (GL_BACK);                   // Hide back faces
    glFrontFace(GL_CCW);                    // Front faces are drawn in counter-clockwise direction

    glShadeModel(GL_SMOOTH);                // GL_SMOOTH - enable smooth shading, GL_FLAT - no gradient on faces
    glEnable (GL_POINT_SMOOTH);
    //glEnable (GL_LINE_SMOOTH);
    //glEnable (GL_POLYGON_SMOOTH);         // produces errors on many cards... use FSAA!

    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); // Nice perspective calculations

    //if (!shader.IsInitialized())
    {
        shader.Terminate();
        shader.Load("Data/program.vert", "Data/program.frag");
        shader.Initialize();
    }
    return true;
}

bool SceneGame::Invalidate()
{
    shader.Invalidate();
    World::lightsVec::iterator light, begin = world.lights.begin(), end = world.lights.end();
    for (light=begin; light!=end; ++light)
        light->modified = true;

    World::objectVec::iterator model, beginM = world.objects.begin(), endM = world.objects.end();
    for (model=beginM; model!=endM; ++model)
        (*model)->Invalidate();

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
        xglPerspective(FOV);
        glMatrixMode(GL_MODELVIEW);
        DefaultCamera->LookAtMatrix(FOV.ViewTransform);
        glLoadMatrixf(&FOV.ViewTransform.x0);
        ModelObj *obj = world.Select(&FOV, g_InputMgr.mouseX, g_InputMgr.mouseY);
        if (obj)
        {
            xRender *rend = obj->GetRenderer();
            g_Application.SetCurrentScene(new SceneSkeleton(this,
                rend->xModelGraphics->fileName, rend->xModelPhysical->fileName), false);
        }
/*
        float near_height = 0.1f * tan(45.0f * PI / 360.0f);
        float norm_x = 1.0f - (float)g_InputMgr.mouseX/(Width/2.0f);
        float norm_y = (float)g_InputMgr.mouseY/(Height/2.0f) - 1.0f;

        // get model view matrix
        xMatrix modelView;
        DefaultCamera->LookAtMatrix(modelView);
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

void SceneGame::SetLights()
{
    World::lightsVec::iterator light, begin = world.lights.begin(), end = world.lights.end();
    GLuint lightNo = GL_LIGHT0;

    for (light=begin; light!=end; ++light, ++lightNo)
    {
        xVector4 position; position.init(light->position, light->type == xLight_INFINITE ? 0.f : 1.f);
        glLightfv(lightNo, GL_POSITION, position.xyzw);

        xVector4 ambient; ambient.init(light->color.col); ambient *= 0.1f;
        glLightfv(lightNo, GL_AMBIENT,  ambient.xyzw);     // environment
        glLightfv(lightNo, GL_DIFFUSE,  light->color.col); // direct light
        glLightfv(lightNo, GL_SPECULAR, light->color.col); // light on mirrors/metal

        // rozpraszanie siê œwiat³a
        glLightf(lightNo, GL_CONSTANT_ATTENUATION,  light->attenuationConst);
        glLightf(lightNo, GL_LINEAR_ATTENUATION,    light->attenuationLinear);
        glLightf(lightNo, GL_QUADRATIC_ATTENUATION, light->attenuationSquare);

        if (light->type == xLight_SPOT)
        {
            glLightfv(lightNo, GL_SPOT_DIRECTION, light->spotDirection.xyz);
            glLightf(lightNo,  GL_SPOT_CUTOFF,    light->spotCutOff);
            glLightf(lightNo,  GL_SPOT_EXPONENT,  light->spotAttenuation);
        }
        else
            glLightf(lightNo, GL_SPOT_CUTOFF, 180.0f);

        glEnable(lightNo);
    }
    for (; lightNo < GL_LIGHT0+GL_MAX_LIGHTS; ++lightNo)
        glDisable(lightNo);
}

bool SceneGame::Render()
{
    if (!g_FontMgr.IsHandleValid(m_Font1))
        m_Font1 = g_FontMgr.GetFont("Courier New", 12);
    if (!g_FontMgr.IsHandleValid(m_Font2))
        m_Font2 = g_FontMgr.GetFont("Courier New", 15);

    if (g_Init)
    {
        world.Finalize();
        world.Initialize();
        g_Init = false;
    }

    glPolygonMode(GL_FRONT_AND_BACK, g_PolygonMode);

    GLfloat light_global_amb_color[]  = { 0.2f, 0.2f, 0.2f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_global_amb_color);
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE); // GL_FALSE = infinite viewpoint, GL_TRUE = locale viewpoint
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE); // GL_TRUE=two, GL_FALSE=one

    // Render the contents of the world
    World::objectVec::iterator i,j, begin = world.objects.begin(), end = world.objects.end();

    //////////////////// SHADOW MAPS - BEGIN

    glViewport(0, 0, g_ShadowMapSize, g_ShadowMapSize);
    //glEnable(GL_SCISSOR_TEST);
    //glScissor(0, 0, g_ShadowMapSize, g_ShadowMapSize);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // We will make a dark grey on white shadow-map, so clear the buffer with white
    glClearColor(1.0, 1.0, 1.0, 0.0);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    GLShader::EnableLighting(0);
    GLShader::EnableTexturing(0);
    glShadeModel(GL_FLAT);
    glDisable (GL_POINT_SMOOTH);
    glDisable (GL_LINE_SMOOTH);
    glDisable (GL_POLYGON_SMOOTH);
    for ( i = begin + 35 ; i != end ; ++i )
        if (world.lights[0].modified || !(*i)->GetShadowMap().texId)
            (*i)->CreateShadowMap(&world.lights[0]);

    //////////////////// SHADOW MAPS - END

    world.lights[0].modified = false;

    //////////////////// WORLD - BEGIN

    glViewport(Left, Top, Width, Height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    xglPerspective(FOV);
    glMatrixMode(GL_MODELVIEW);
    DefaultCamera->LookAtMatrix(FOV.ViewTransform);
    glLoadMatrixf(&FOV.ViewTransform.x0); //Camera_Aim_GL(*DefaultCamera);

    glShadeModel(GL_SMOOTH);
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /////// Render the SKY
    (*begin)->Render(false, NULL);

    SetLights();
    glEnable(GL_DEPTH_TEST);

    /////// Render the opaque World
    if (g_UseCustomShader && shader.IsInitialized()) shader.Start();
    if (g_EnableLighting)                            GLShader::EnableLighting(world.lights.size());
    for ( i = begin+1 ; i != end ; ++i )             (*i)->Render(false, &FOV);
    if (g_UseCustomShader && shader.IsInitialized()) shader.Suspend();
    /////// Render shadows of the World
    glClearStencil(0);
    //glEnable( GL_STENCIL_TEST );
    glStencilFunc( GL_NOTEQUAL, 1, 1 );
    glStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );
    for ( j = begin+1 ; j != begin+35 ; ++j )
    {
        glClear(GL_STENCIL_BUFFER_BIT);
        for ( i = begin+35 ; i != end ; ++i )
            if (*i != *j)
            {
                xShadowMap &smap = (*i)->GetShadowMap();
                if (smap.texId)
                {
                    xVector3 vecI = (xVector4::Create((*i)->centerOfTheMass, 1.f) * (*i)->mLocationMatrix).vector3;
                    xVector3 vecJ = (xVector4::Create((*j)->centerOfTheMass, 1.f) * (*j)->mLocationMatrix).vector3;
                    vecI -= world.lights[0].position;
                    vecJ -= world.lights[0].position;
                    if (xVector3::DotProduct(vecI, vecJ) > 0.f)
                        (*j)->RenderShadow(smap, &FOV);
                }
            }
    }
    glDisable( GL_STENCIL_TEST );
    /////// Render the transparent World
    //if (g_UseCustomShader && shader.IsInitialized()) shader.Start();
    if (g_EnableLighting)                            GLShader::EnableLighting(world.lights.size());
    for ( i = begin+1 ; i != end ; ++i )             (*i)->Render(true, &FOV);
    //if (g_UseCustomShader && shader.IsInitialized()) shader.Suspend();

    //////////////////// WORLD - END

    // Flush the buffer to force drawing of all objects thus far
    glFlush();
    return true;
}
