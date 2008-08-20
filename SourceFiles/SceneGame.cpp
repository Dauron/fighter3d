#include "SceneGame.h"
#include "SceneConsole.h"
#include "SceneSkeleton.h"

#include "../AppFramework/Application.h"
#include "../AppFramework/Input/InputMgr.h"
#include "../Math/Cameras/CameraHuman.h"

#include "../Graphics/OGL/GLShader.h"
#include "../Graphics/OGL/Extensions/EXT_stencil_wrap.h"
#include "../Graphics/OGL/Extensions/EXT_stencil_two_side.h"
#include "../Graphics/OGL/Extensions/ARB_multisample.h"

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

    if (!world.objects.size())
    {
        world.Initialize();
        world.InitialUpdate();
    }
    return InitGL();
}

bool SceneGame :: InitGL()
{
    glClearDepth(1.f);                      // Mapped draw distance ([0-1])
    glDepthFunc(GL_LEQUAL);                 // Depth testing function

    glEnable(GL_CULL_FACE);                 // Do not draw hidden faces
    glCullFace (GL_BACK);                   // Hide back faces
    glFrontFace(GL_CCW);                    // Front faces are drawn in counter-clockwise direction

    glShadeModel(GL_SMOOTH);                // GL_SMOOTH - enable smooth shading, GL_FLAT - no gradient on faces
    glEnable (GL_POINT_SMOOTH);

    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); // Nice perspective calculations

    GLExtensions::SetVSync(false);

    //if (!shader.IsInitialized())
    {
        //GLShader::Terminate();
        //GLShader::Load();
        //GLShader::Initialize();
    }
    return true;
}

bool SceneGame :: Invalidate()
{
    world.Invalidate();
    return true;
}

void SceneGame :: InitInputMgr()
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

    im.SetInputCode(VK_HOME,   IC_TurnUp);
    im.SetInputCode(VK_END,    IC_TurnDown);
    im.SetInputCode(VK_DELETE, IC_TurnLeft);
    im.SetInputCode(VK_NEXT,   IC_TurnRight);
    im.SetInputCode('Y', IC_RollLeft);
    im.SetInputCode('I', IC_RollRight);

    //im.SetInputCode('U', IC_OrbitUp);
    //im.SetInputCode('J', IC_OrbitDown);
    //im.SetInputCode('H', IC_OrbitLeft);
    //im.SetInputCode('K', IC_OrbitRight);

    im.SetInputCode('U', IC_MoveForward);
    im.SetInputCode('J', IC_MoveBack);
    im.SetInputCode('H', IC_MoveLeft);
    im.SetInputCode('K', IC_MoveRight);
    im.SetInputCode('O', IC_MoveUp);
    im.SetInputCode('L', IC_MoveDown);
    im.SetInputCode(VK_LSHIFT, IC_RunModifier);

    im.SetInputCode('S', IC_CB_LeftPunch);
    im.SetInputCode('A', IC_CB_LeftHandGuard);
    im.SetInputCode('X', IC_CB_LeftKick);
    im.SetInputCode('Z', IC_CB_LeftLegGuard);
    im.SetInputCode('D', IC_CB_RightPunch);
    im.SetInputCode('F', IC_CB_RightHandGuard);
    im.SetInputCode('C', IC_CB_RightKick);
    im.SetInputCode('V', IC_CB_RightLegGuard);
    im.SetInputCode(VK_UP,    IC_CB_Forward);
    im.SetInputCode(VK_DOWN,  IC_CB_Backward);
    im.SetInputCode(VK_LEFT,  IC_CB_Left);
    im.SetInputCode(VK_RIGHT, IC_CB_Right);

    im.SetInputCode(VK_F11, IC_FullScreen);
}

void SceneGame :: Terminate()
{
    DefaultCamera = NULL;
    world.Finalize();
    Scene::Terminate();
}


bool SceneGame :: ShellCommand (std::string &cmd, std::string &output)
{
    if (cmd == "?" || cmd == "help")
    {
        output.append("\n\
  Available shell comands for [game]:\n\
    Full command        | Short command | Description\n\
    ------------------------------------------------------------------------\n\
    help                | ?             | print this help screen\n\
    ------------------------------------------------------------------------\n\
    toggle_lights       | tls           | turns the lighting on and off\n\
    toggle_shadows      | tshadow       | toggles shadow rendering\n\
    toggle_shadow_vol   | tshadowv      | toggles shadow volume rendering\n\
    reinitialize        | init          | reinitialize objects, etc.\n\
    toggle_shader       | tshd          | toggles custom shader\n\
    toggle_polygon_mode | tpm           | toggle polygon mode\n\
    ------------------------------------------------------------------------\n\
    level {int}         | level {int}   | load 'level_{int}.map' scene\n\
    speed {float}       | speed {float} | enter clock speed multiplier\n");
        return true;
    }
    if (cmd.substr(0, 6) == "level ")
    {
        Config::TestCase = atoi(cmd.substr(6).c_str());
        world.Finalize();
        world.Initialize();
        world.Interact(0.f, world.objects);
        return true;
    }
    if (cmd == "init" || cmd == "reinitialize")
    {
        world.Finalize();
        world.Initialize();
        world.InitialUpdate();
        return true;
    }
    if (cmd.substr(0, 6) == "speed ")
    {
        Config::Speed = atof(cmd.substr(6).c_str());
        return true;
    }
    if (cmd == "tls" || cmd == "toggle_lights")
    {
        if (Config::EnableLighting = !Config::EnableLighting)
            output.append("\nThe lights are ON.\n");
        else
            output.append("\nThe lights are OFF.\n");
        return true;
    }
    if (cmd.substr(0, 4) == "tls ")
    {
        unsigned int id = (unsigned int)atoi(cmd.substr(4).c_str());
        if (id >= 0 && id < world.lights.size())
            world.lights[id].turned_on = !world.lights[id].turned_on;
        return true;
    }
    if (cmd == "tshadow" || cmd == "toggle_shadows")
    {
        if (Config::EnableShadows = !Config::EnableShadows)
            output.append("\nThe shadows are ON.\n");
        else
            output.append("\nThe shadows are OFF.\n");
        return true;
    }
    if (cmd == "tshadowv" || cmd == "toggle_shadow_vol")
    {
        if (Config::DisplayShadowVolumes = !Config::DisplayShadowVolumes)
            output.append("\nThe shadow volumes are ON.\n");
        else
            output.append("\nThe shadow volumes are OFF.\n");
        return true;
    }
    if (cmd == "tshd" || cmd == "toggle_shader")
    {
        if (Config::EnableShaders = !Config::EnableShaders)
            output.append("\nThe shaders are ON.\n");
        else
            output.append("\nThe shaders are OFF.\n");
        return true;
    }
    if (cmd == "tpm" || cmd == "toggle_polygon_mode")
    {
        switch (Config::PolygonMode)
        {
            case GL_FILL:
                Config::PolygonMode = GL_LINE;
                output.append("\nPolygon mode: GL_LINE.\n");
                break;
            case GL_LINE:
                Config::PolygonMode = GL_FILL;
                output.append("\nPolygon mode: GL_FILL.\n");
                break;
        }
        return true;
    }
    return false;
}

bool SceneGame::FrameUpdate(float deltaTime)
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
        RigidObj *obj = Select(g_InputMgr.mouseX, g_InputMgr.mouseY);
        if (obj)
            g_Application.SetCurrentScene(new SceneSkeleton(this,
                obj->GetModelGr()->FileName, obj->GetModelPh()->FileName), false);
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

        RigidObj *obj = world.CollideWithRay(rayPos, -rayDir);
        if (obj)
            g_Application.SetCurrentScene(new SceneSkeleton(this, obj->GetName()), false);
*/
        return true;
    }

    world.FrameUpdate(deltaTime);
    return true;
}

void SceneGame::SetLight(xLight &light, bool t_Ambient, bool t_Diffuse, bool t_Specular)
{
    float light_off[4] = { 0.f, 0.f, 0.f, 0.f };

    // turn off ambient lighting
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_off);

    xVector4 position; position.init(light.position, light.type == xLight_INFINITE ? 0.f : 1.f);
    glLightfv(GL_LIGHT0, GL_POSITION, position.xyzw);

    glLightfv(GL_LIGHT0, GL_AMBIENT, t_Ambient ? light.ambient.col : light_off);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  t_Diffuse ? light.diffuse.col : light_off); // direct light
    glLightfv(GL_LIGHT0, GL_SPECULAR, t_Specular ? light.diffuse.col : light_off); // light on mirrors/metal

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

bool SceneGame::FrameRender()
{
    world.FrameRender();

    static xLight dayLight;
    static xLightVector dayLightVec;

    if (dayLight.id == 0)
    {
        dayLight.create();
        dayLight.turned_on = true;
        dayLight.color.init(0.9f, 0.9f, 1.f, 1.f);
        dayLight.softness = 0.4f;
        //dayLight.position.init(-20.f, 20.f, 100.f);
        dayLight.position.init(0.f, 0.f, 10.f);
        dayLight.type = xLight_POINT;
        dayLight.attenuationConst  = 0.9f;
        dayLight.attenuationLinear = 0.005f;
        dayLight.attenuationSquare = 0.0005f;
        dayLight.update();
        dayLightVec.push_back(dayLight);
    }

    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE); // GL_FALSE = infinite viewpoint, GL_TRUE = locale viewpoint
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE); // GL_TRUE=two, GL_FALSE=one
    glDisable (GL_LINE_SMOOTH);
    glDisable (GL_POLYGON_SMOOTH);                    // produces errors on many cards... use FSAA!
    glDisable(GL_LIGHT0); glDisable(GL_LIGHT1); glDisable(GL_LIGHT2); glDisable(GL_LIGHT3);
    glDisable(GL_LIGHT4); glDisable(GL_LIGHT5); glDisable(GL_LIGHT6); glDisable(GL_LIGHT7);

    // Render the contents of the world
    World::ObjectVector::iterator i,j, begin = world.objects.begin(), end = world.objects.end();
    xLightVector::iterator light, beginL = world.lights.begin(), endL = world.lights.end();

    //////////////////// WORLD - BEGIN

    glViewport(Left, Top, Width, Height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    xglPerspectiveInf(FOV);
    glMatrixMode(GL_MODELVIEW);
    DefaultCamera->LookAtMatrix(FOV.ViewTransform);
    glLoadMatrixf(&FOV.ViewTransform.x0); //Camera_Aim_GL(*DefaultCamera);
    FOV.update();

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    if (GLExtensions::Exists_ARB_Multisample)
        glDisable(GL_MULTISAMPLE_ARB);

    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    GLShader::Suspend();
    GLShader::SetLightType(xLight_NONE);
    GLShader::EnableTexturing(xState_Enable);
    if (Config::EnableFullLighting)
        glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
    else
        glClearColor( 0.8f, 0.8f, 1.0f, 0.0f );
    glDepthMask(1);
    glColorMask(1,1,1,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (Config::EnableLighting)
        if (Config::EnableFullLighting)
        {
            /////// Render the SKY
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glDisable(GL_STENCIL_TEST);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);
            if (world.skyBox)
                world.skyBox->Render(false, FOV);

            ////// RENDER Z-ONLY PASS
            glPolygonMode(GL_FRONT_AND_BACK, Config::PolygonMode);
            glEnable   (GL_DEPTH_TEST);
            glDepthMask(1);
            glDepthFunc(GL_LESS);
            glEnable   (GL_CULL_FACE);
            glCullFace (GL_BACK);
            glColorMask(0,0,0,0);
            GLShader::SetLightType(xLight_NONE);
            GLShader::EnableTexturing(xState_Off);
            for ( i = begin ; i != end ; ++i )
                ((RigidObj*)*i)->RenderDepth(FOV, false);

            ////// RENDER GLOBAL AMBIENT PASS
            for (light=beginL; light!=endL; ++light)
                light->update();

            //if (GLExtensions::Exists_ARB_Multisample)
            //    glEnable(GL_MULTISAMPLE_ARB);
            glDepthMask(0);
            glDepthFunc(GL_LEQUAL);
            glColorMask(1,1,1,1);
            GLShader::SetLightType(xLight_GLOBAL, true, false, false);
            glDisable(GL_LIGHT0);
            for ( i = begin ; i != end ; ++i ) ((RigidObj*)*i)->RenderAmbient(world.lights, FOV, false);

            ////// RENDER SHADOWS AND LIGHTS
            for (light=beginL; light!=endL; ++light)
            {
                if (light->turned_on && light->isVisible(&FOV))
                {
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
                        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                        glStencilMask(0xff);
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
                        glDepthFunc(GL_LESS);
                        glColorMask(0, 0, 0, 0);            // do not write to frame buffer
                        GLShader::EnableTexturing(xState_Disable);
                        GLShader::SetLightType(xLight_NONE);
                        GLShader::Suspend();
                        for ( i = begin ; i != end ; ++i )
                            if (((RigidObj*)*i)->FL_shadowcaster)
                                ((RigidObj*)*i)->RenderShadowVolume(*light, FOV);
                    }

                    ////// ILLUMINATION PASS
                    glPolygonMode(GL_FRONT_AND_BACK, Config::PolygonMode);
                    if (Config::EnableShadows)
                    {
                        glStencilMask(0);                 // do not write to stencil buffer
                        glStencilFunc(GL_EQUAL, 0, 0xff); // set stencil test function
                    }
                    glDepthFunc(GL_LEQUAL);
                    glEnable(GL_CULL_FACE);     // enable face culling
                    glCullFace(GL_BACK);
                    glColorMask(1,1,1,1);
                    glEnable(GL_BLEND);                 // add light contribution to frame buffer
                    glBlendFunc(GL_ONE, GL_ONE);
                    GLShader::EnableTexturing(xState_Enable);
                    SetLight(*light, false, true, true);
                    glEnable(GL_LIGHT0);
                    for ( i = begin ; i != end ; ++i ) ((RigidObj*)*i)->RenderDiffuse(*light, FOV, false);

                    ////// DISPLAY SHADOW VOLUMES PASS
                    if (Config::DisplayShadowVolumes)
                    {
                        glPushAttrib(GL_ALL_ATTRIB_BITS);
                        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                        glDisable(GL_STENCIL_TEST);
                        glDepthMask(1);
                        glColorMask(1, 1, 1, 1);
                        glEnable(GL_BLEND);
                        GLShader::EnableTexturing(xState_Disable);
                        GLShader::SetLightType(xLight_NONE);
                        GLShader::Suspend();
                        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
                        for ( i = begin ; i != end ; ++i )
                            if (((RigidObj*)*i)->FL_shadowcaster)
                                ((RigidObj*)*i)->RenderShadowVolume(*light, FOV);
                        glPopAttrib();
                    }
                }
                light->modified = false;
            }

            ////// RENDER TRANSPARENT PASS
            glPolygonMode(GL_FRONT_AND_BACK, Config::PolygonMode);
            glDepthMask(0);
            glDepthFunc(GL_LESS);
            glDisable(GL_STENCIL_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
            GLShader::EnableTexturing(xState_Enable);
            GLShader::SetLightType(xLight_GLOBAL, true, false, false); // 3 * true
            glDisable(GL_LIGHT0);
            for ( i = begin ; i != end ; ++i ) ((RigidObj*)*i)->RenderAmbient(world.lights, FOV, true);
            GLShader::Suspend();
/*
            GLShader::EnableTexturing(xState_Disable);
            GLShader::SetLightType(xLight_NONE);
            GLShader::Start();
            for ( i = begin ; i != end ; ++i )
            {
                RigidObj &model = **i;
                model.renderer.RenderSkeleton(*model.GetModelGr(), model.modelInstanceGr, xWORD_MAX);
            }
            GLShader::Suspend();
*/
            /*
            GLShader::EnableTexturing(xState_Disable);
            GLShader::SetLightType(xLight_NONE);
            GLShader::Start();
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            glColor3f(1.f, 1.f, 1.f);
            for ( i = begin ; i != end ; ++i )
                ((RigidObj*)*i)->renderer.RenderBVH((**i).BVHierarchy, (**i).MX_LocalToWorld_Get());
            GLShader::Suspend();
            */
        }
        else
        {
            ////// RENDER GLOBAL AMBIENT PASS

            //if (GLExtensions::Exists_ARB_Multisample)
            //    glEnable(GL_MULTISAMPLE_ARB);
            glEnable   (GL_DEPTH_TEST);
            glDepthMask(1);
            glDepthFunc(GL_LESS);
            glEnable   (GL_CULL_FACE);
            glCullFace (GL_BACK);
            glColorMask(1,1,1,1);
            GLShader::SetLightType(xLight_GLOBAL, true, false, false);
            glDisable(GL_BLEND);
            glDisable(GL_LIGHT0);
            for ( i = begin ; i != end ; ++i ) ((RigidObj*)*i)->RenderAmbient(dayLightVec, FOV, false);

            ////// RENDER SHADOWS AND LIGHTS
            glDepthMask(0);

            ////// SHADOW DETERMINATION PASS
            if (Config::EnableShadows)
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                glStencilMask(0xff);
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
                glDepthFunc(GL_LESS);
                glColorMask(0, 0, 0, 0);            // do not write to frame buffer
                GLShader::EnableTexturing(xState_Disable);
                GLShader::SetLightType(xLight_NONE);
                GLShader::Suspend();
                for ( i = begin ; i != end ; ++i )
                    if (((RigidObj*)*i)->FL_shadowcaster)
                        ((RigidObj*)*i)->RenderShadowVolume(*dayLightVec.begin(), FOV);
            }

            ////// ILLUMINATION PASS
            glPolygonMode(GL_FRONT_AND_BACK, Config::PolygonMode);
            if (Config::EnableShadows)
            {
                glStencilMask(0);                 // do not write to stencil buffer
                glStencilFunc(GL_EQUAL, 0, 0xff); // set stencil test function
            }
            glDepthFunc(GL_LEQUAL);
            glEnable(GL_CULL_FACE);     // enable face culling
            glCullFace(GL_BACK);
            glColorMask(1,1,1,1);
            glEnable(GL_BLEND);                 // add light contribution to frame buffer
            glBlendFunc(GL_ONE, GL_ONE);
            GLShader::EnableTexturing(xState_Enable);
            SetLight(dayLight, false, true, true);
            glEnable(GL_LIGHT0);
            for ( i = begin ; i != end ; ++i ) ((RigidObj*)*i)->RenderDiffuse(*dayLightVec.begin(), FOV, false);

            dayLight.modified = false;

            ////// RENDER TRANSPARENT PASS
            glPolygonMode(GL_FRONT_AND_BACK, Config::PolygonMode);
            glDepthMask(0);
            glDepthFunc(GL_LESS);
            glDisable(GL_STENCIL_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
            GLShader::EnableTexturing(xState_Enable);
            GLShader::SetLightType(xLight_GLOBAL, true, false, false); // 3 * true
            glDisable(GL_LIGHT0);
            for ( i = begin ; i != end ; ++i ) ((RigidObj*)*i)->RenderAmbient(dayLightVec, FOV, true);
            GLShader::Suspend();
        }
    else
    {
        ////// RENDER OPAQUE PASS
        glEnable   (GL_DEPTH_TEST);
        glDepthMask(1);
        glDepthFunc(GL_LESS);
        glEnable   (GL_CULL_FACE);
        glCullFace (GL_BACK);
        glColorMask(1, 1, 1, 1);
        GLShader::EnableTexturing(xState_Enable);
        GLShader::SetLightType(xLight_NONE);
        for ( i = begin ; i != end ; ++i ) ((RigidObj*)*i)->RenderAmbient(world.lights, FOV, false);

        ////// RENDER TRANSPARENT PASS
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        for ( i = begin ; i != end ; ++i ) ((RigidObj*)*i)->RenderAmbient(world.lights, FOV, true);
    }

    glPopAttrib();
    if (GLExtensions::Exists_ARB_Multisample)
        glDisable(GL_MULTISAMPLE_ARB);

    //////////////////// WORLD - END
    glFlush();
    //glFinish();
    return true;
}

////// ISelectionProvider

void SceneGame :: RenderSelect(const xFieldOfView *FOV)
{
    int objectID = -1;
    World::ObjectVector::iterator iter = world.objects.begin(), end = world.objects.end();
    for ( ; iter != end ; ++iter ) {
        glLoadName(++objectID);
        RigidObj &mdl = *(RigidObj*)*iter;
        mdl.renderer.RenderVertices(*mdl.GetModelGr(), mdl.modelInstanceGr, Renderer::smModel);
    }
}

unsigned int SceneGame::CountSelectable()
{
    return world.objects.size();
}