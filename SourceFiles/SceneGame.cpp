#include "../AppFramework/System.h"
#include <GL/gl.h>
#include "SceneGame.h"
#include "SceneConsole.h"
#include "SceneSkeleton.h"
#include "../AppFramework/Input/InputMgr.h"
#include "../AppFramework/Application.h"
#include "../Math/Cameras/CameraHuman.h"
#include "../OpenGL/GLAnimSkeletal.h"

#include "../GLExtensions/EXT_stencil_wrap.h"
#include "../GLExtensions/EXT_stencil_two_side.h"

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
        //GLShader::Terminate();
        //GLShader::Load();
        //GLShader::Initialize();
    }
    return true;
}

bool SceneGame::Invalidate()
{
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
            g_Application.SetCurrentScene(new SceneSkeleton(this,
                obj->GetModelGr()->fileName, obj->GetModelPh()->fileName), false);
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

void SceneGame::SetLight(xLight &light, bool t_Ambient, bool t_Diffuse, bool t_Specular)
{
    float light_off[4] = { 0.f, 0.f, 0.f, 0.f };
    xColor ambient; ambient = light.color * light.softness;
    xColor diffuse; diffuse = light.color - ambient;

    // turn off ambient lighting
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_off);

    xVector4 position; position.init(light.position, light.type == xLight_INFINITE ? 0.f : 1.f);
    glLightfv(GL_LIGHT0, GL_POSITION, position.xyzw);

    glLightfv(GL_LIGHT0, GL_AMBIENT, t_Ambient ? ambient.col : light_off);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  t_Diffuse ? diffuse.col : light_off); // direct light
    glLightfv(GL_LIGHT0, GL_SPECULAR, t_Specular ? diffuse.col : light_off); // light on mirrors/metal

    // rozpraszanie siê œwiat³a
    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION,  light.attenuationConst);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION,    light.attenuationLinear);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, light.attenuationSquare);

    if (light.type == xLight_SPOT)
    {
        glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, light.spotDirection.xyz);
        glLightf(GL_LIGHT0,  GL_SPOT_CUTOFF,    light.spotCutOff);
        glLightf(GL_LIGHT0,  GL_SPOT_EXPONENT,  light.spotAttenuation);
    }
    else
        glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 180.0f);

    GLShader::SetLightType(light.type, t_Ambient, t_Diffuse, t_Specular);
}

bool SceneGame::Render()
{
    if (!g_FontMgr.IsHandleValid(m_Font1))
        m_Font1 = g_FontMgr.GetFont("Courier New", 12);
    if (!g_FontMgr.IsHandleValid(m_Font2))
        m_Font2 = g_FontMgr.GetFont("Courier New", 15);

    if (Config::Initialize)
    {
        world.Finalize();
        world.Initialize();
        Config::Initialize = false;
    }

    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE); // GL_FALSE = infinite viewpoint, GL_TRUE = locale viewpoint
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE); // GL_TRUE=two, GL_FALSE=one
    glDisable (GL_LINE_SMOOTH);
    glDisable (GL_POLYGON_SMOOTH);                    // produces errors on many cards... use FSAA!
    glDisable(GL_LIGHT0); glDisable(GL_LIGHT1); glDisable(GL_LIGHT2); glDisable(GL_LIGHT3);
    glDisable(GL_LIGHT4); glDisable(GL_LIGHT5); glDisable(GL_LIGHT6); glDisable(GL_LIGHT7);

    // Render the contents of the world
    World::objectVec::iterator i,j,   begin = world.objects.begin(), end = world.objects.end();
    World::lightsVec::iterator light, beginL = world.lights.begin(), endL = world.lights.end();

    //////////////////// WORLD - BEGIN

    glViewport(Left, Top, Width, Height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    xglPerspective(FOV);
    glMatrixMode(GL_MODELVIEW);
    DefaultCamera->LookAtMatrix(FOV.ViewTransform);
    glLoadMatrixf(&FOV.ViewTransform.x0); //Camera_Aim_GL(*DefaultCamera);

    glShadeModel(GL_SMOOTH);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    GLShader::SetLightType(xLight_NONE);
    GLShader::EnableTexturing(xState_Enable);
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /////// Render the SKY
    (*begin)->Render(false, FOV);

    if (Config::EnableLighting)
    {
        ////// RENDER GLOBAL AMBIENT PASS

        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glPolygonMode(GL_FRONT_AND_BACK, Config::PolygonMode);
        glEnable   (GL_DEPTH_TEST);
        glDepthMask(1);
        glDepthFunc(GL_LESS);
        glEnable   (GL_CULL_FACE);
        glCullFace (GL_BACK);
        glColorMask(1, 1, 1, 1);
        GLShader::EnableTexturing(xState_Enable);
        GLShader::SetLightType(xLight_GLOBAL, true, false, false);
        glDisable(GL_LIGHT0);
        GLfloat light_global_amb_color[]  = { 0.1f, 0.1f, 0.1f, 1.0f };
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_global_amb_color);
        for ( i = begin+1 ; i != end ; ++i ) (*i)->Render(false, FOV);
        glPopAttrib();

        ////// RENDER SHADOWS AND LIGHTS

        FOV.updateCorners3D();
        for (light=beginL; light!=endL; ++light)
        {
            light->update();
            if (light->turned_on && light->isVisible(&FOV))
            {
                ////// AMBIENT PASS
                glPushAttrib(GL_ALL_ATTRIB_BITS);
                glPolygonMode(GL_FRONT_AND_BACK, Config::PolygonMode);
                glEnable   (GL_DEPTH_TEST);
                glDepthMask(0);                     // do not write to z-buffer
                glDepthFunc(GL_LEQUAL);
                glEnable(GL_CULL_FACE);             // enable face culling
                glCullFace(GL_BACK);
                glColorMask(1,1,1,1);
                glEnable(GL_BLEND);                 // add light contribution to frame buffer
                glBlendFunc(GL_ONE, GL_ONE);
                GLShader::EnableTexturing(xState_Enable);
                SetLight(*light, true, false, false);
                glEnable(GL_LIGHT0);                // light0 should be set to have the
                                                    // characteristics of the light
                                                    // we want to use for this pass
                for ( i = begin+1 ; i != end ; ++i ) (*i)->Render(false, FOV);
                glPopAttrib();

                if (light->type != xLight_INFINITE && light->radius > 0.f)
                {
                    //int x, y, width, height;
                    // set scissor region optimization
                    //getScreenBoundingRectangle(vector4to3(curLight.m_position),
                    //    curLight.m_radius, camera, view,
                    //    vars.m_winWidth, vars.m_winHeight,
                    //    x, y, width, height);

                    //glEnable(GL_SCISSOR_TEST);
                    //glScissor(x, y, width, height);
                }

                ////// SHADOW DETERMINATION PASS
                if (Config::EnableShadows)
                {
                    glPushAttrib(GL_ALL_ATTRIB_BITS);
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    glClear(GL_STENCIL_BUFFER_BIT);
                    glEnable(GL_STENCIL_TEST);          // write to stencil buffer

                    if (GLExtensions::Exists_EXT_StencilTwoSide)
                    {
                        glDisable(GL_CULL_FACE);
                        glEnable(GL_STENCIL_TEST_TWO_SIDE_EXT);

                        glActiveStencilFaceEXT(GL_BACK);
                        glStencilMask(0xff);
                        glStencilFunc(GL_ALWAYS, 0, 0xff);
                        glActiveStencilFaceEXT(GL_FRONT);
                    }
                    else
                        glEnable(GL_CULL_FACE);
                    glStencilMask(0xff);                // allow writing to the first byte of buffer
                    glStencilFunc(GL_ALWAYS, 0, 0xff);  // always pass stencil test
                    glEnable   (GL_DEPTH_TEST);
                    glDepthMask(0);                     // do not write to z-buffer
                    glDepthFunc(GL_LESS);
                    glColorMask(0, 0, 0, 0);            // do not write to frame buffer
                    GLShader::EnableTexturing(xState_Disable);
                    GLShader::SetLightType(xLight_NONE);
                    GLShader::EnableShaders(xState_Disable);

                    for ( i = begin+1 ; i != end ; ++i )
                        if ((*i)->castsShadows)
                            (*i)->RenderShadowVolume(*light, FOV);

                    GLShader::EnableShaders(xState_Enable);

                    glPopAttrib();
                }

                ////// ILLUMINATION PASS
                glPushAttrib(GL_ALL_ATTRIB_BITS);
                glPolygonMode(GL_FRONT_AND_BACK, Config::PolygonMode);
                if (Config::EnableShadows)
                {
                    glEnable(GL_STENCIL_TEST);          // read from stencil buffer
                    glStencilMask(0);                   // do not write to stencil buffer
                    glStencilFunc(GL_EQUAL, 0, 0xff);   // set stencil test function
                }
                glEnable   (GL_DEPTH_TEST);
                glDepthMask(0);                     // do not write to z-buffer
                glDepthFunc(GL_LEQUAL);
                glEnable(GL_CULL_FACE);             // enable face culling
                glCullFace(GL_BACK);
                glColorMask(1,1,1,1);
                glEnable(GL_BLEND);                 // add light contribution to frame buffer
                glBlendFunc(GL_ONE, GL_ONE);
                GLShader::EnableTexturing(xState_Enable);
                SetLight(*light, false, true, true);
                glEnable(GL_LIGHT0);                // light0 should be set to have the
                                                    // characteristics of the light
                                                    // we want to use for this pass
                for ( i = begin+1 ; i != end ; ++i ) (*i)->Render(false, FOV);
                glPopAttrib();

                ////// DISPLAY SHADOW VOLUMES PASS
                if (Config::DisplayShadowVolumes)
                {
                    glPushAttrib(GL_ALL_ATTRIB_BITS);
                    glEnable(GL_CULL_FACE);             // enable face culling
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                    glDisable(GL_STENCIL_TEST);
                    glEnable(GL_DEPTH_TEST);
                    glDepthMask(1);                     // do not write to z-buffer
                    glColorMask(1, 1, 1, 1);            // do not write to frame buffer
                    glEnable(GL_BLEND);
                    GLShader::EnableTexturing(xState_Disable);
                    GLShader::SetLightType(xLight_NONE);
                    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
                    //glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_COLOR);

                    for ( i = begin+1 ; i != end ; ++i )
                        if ((*i)->castsShadows)
                            (*i)->RenderShadowVolume(*light, FOV);
                    glPopAttrib();
                }
            }
            light->modified = false;
        }

        ////// RENDER TRANSPARENT PASS

        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glPolygonMode(GL_FRONT_AND_BACK, Config::PolygonMode);
        glEnable   (GL_DEPTH_TEST);
        glDepthMask(1);
        glDepthFunc(GL_LESS);
        glEnable   (GL_CULL_FACE);
        glCullFace (GL_BACK);
        glColorMask(1, 1, 1, 1);
        glEnable(GL_BLEND);
        //glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_COLOR);
        GLShader::EnableTexturing(xState_Enable);
        GLShader::SetLightType(xLight_GLOBAL, false, true, true); // 3 * true
        glDisable(GL_LIGHT0);
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_global_amb_color);
        for ( i = begin+1 ; i != end ; ++i ) (*i)->Render(true, FOV);
        glPopAttrib();
    }
    else
    {
        ////// RENDER OPAQUE PASS

        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glEnable   (GL_DEPTH_TEST);
        glDepthMask(1);
        glDepthFunc(GL_LESS);
        glEnable   (GL_CULL_FACE);
        glCullFace (GL_BACK);
        glColorMask(1, 1, 1, 1);
        GLShader::EnableTexturing(xState_Enable);
        GLShader::SetLightType(xLight_NONE);

        for ( i = begin+1 ; i != end ; ++i ) (*i)->Render(false, FOV);

        ////// RENDER TRANSPARENT PASS

        glEnable(GL_BLEND);
        //glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_COLOR);

        for ( i = begin+1 ; i != end ; ++i ) (*i)->Render(true, FOV);

        glPopAttrib();
    }

    //////////////////// WORLD - END

    //glFlush();
    glFinish();
    return true;
}
