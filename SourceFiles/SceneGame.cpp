#include "SceneGame.h"
#include "SceneMenu.h"
#include "SceneSkeleton.h"

#include "../AppFramework/Application.h"
#include "../AppFramework/Input/InputMgr.h"
#include "../Math/Cameras/CameraHuman.h"
#include "../Math/Cameras/CameraFree.h"

#include "../Graphics/OGL/WorldRenderGL.h"

#include "../World/ObjectTypes.h"

using namespace Scenes;

#define MULT_MOVE   5.0f
#define MULT_RUN    2.0f
#define MULT_ROT    80.0f
#define MULT_STEP   60.0f
/*
bool SceneGame :: Initialize(int left, int top, unsigned int width, unsigned int height,
                             SkeletizedObj *player1, SkeletizedObj *player2)
{
    this->player1 = player1;
    this->player2 = player2;
    return Initialize(left, top, width, height);
}
*/
bool SceneGame :: Initialize(int left, int top, unsigned int width, unsigned int height)
{
    Scene::Initialize(left, top, width, height);

    InitInputMgr();

    if (!world.objects.size())
        InitWorld();
    else
        InitCameras();

    GLExtensions::SetVSync(Config::VSync);

    return true;
}

bool SceneGame :: InitWorld()
{
    FreeWorld();
    world.Initialize(MapFileName);

    if (player1) { player1->MX_LocalToWorld_Set() = world.MX_spawn1; world.objects.push_back(player1); player1 = NULL; }
    if (player2) { player2->MX_LocalToWorld_Set() = world.MX_spawn2; world.objects.push_back(player2); player2 = NULL; }

    Targets.L_objects.clear();
    Physics::PhysicalWorld::Vec_Object::iterator
        OB_curr = world.objects.begin(),
        OB_last = world.objects.end();
    for (; OB_curr != OB_last; ++OB_curr)
        if ( (**OB_curr).Type == AI::ObjectType::Human )
        {
            Targets.L_objects.push_back(*OB_curr);
            ((SkeletizedObj*)(*OB_curr))->Tracker.Targets = &Targets;
        }

    Cameras.Free();
    InitCameras();

    world.InitialUpdate();

    return true;
}

void SceneGame :: FreeWorld()
{
    WorldRenderGL renderer;
    renderer.Free(world);
    world.Finalize();
}

void SceneGame :: InitCameras()
{
    if (Cameras.L_cameras.size() == 0)
    {
        Cameras.Load("Data/cameras.txt");

        if (Cameras.L_cameras.size() == 0)
        {
            Cameras.L_cameras.push_back( new Math::Cameras::CameraFree() );
            Cameras.L_cameras[0]->Init(0.0f, 5.0f, 2.2f, 0.0f, 0.0f, 2.2f, 0.0f, 0.0f, 1.0f);
            Cameras.L_cameras[0]->FOV.InitPerspective();
            Cameras.L_cameras[0]->FOV.InitViewportPercent(0.f,0.f,1.f,1.f, Width, Height);

            Cameras.L_cameras[0]->CenterTracker.Targets = &Targets;
            Cameras.L_cameras[0]->CenterTracker.Mode    = Math::Tracking::ObjectTracker::TRACK_ALL_CENTER;
            Cameras.L_cameras[0]->EyeTracker.Targets    = &Targets;
            Cameras.L_cameras[0]->EyeTracker.Mode       = Math::Tracking::ObjectTracker::TRACK_CUSTOM_SCRIPT;
            Cameras.L_cameras[0]->EyeTracker.ScriptName = "EyeSeeAll_Center";
            Cameras.L_cameras[0]->EyeTracker.Script     = Math::Cameras::Camera::SCRIPT_EyeSeeAll_Center;
            Cameras.L_cameras[0]->EyeTracker.NW_destination_shift.init(0.f,0.f,1.5f);
        }

        DefaultCamera = Cameras.L_cameras[0];
    }

    Math::Cameras::CameraSet::Vec_Camera::iterator
        CAM_curr = Cameras.L_cameras.begin(),
        CAM_last = Cameras.L_cameras.end();
    for (; CAM_curr != CAM_last; ++CAM_curr)
    {
        (**CAM_curr).FOV.ResizeViewport(Width, Height);
        (**CAM_curr).CenterTracker.Targets = &Targets;
        (**CAM_curr).EyeTracker.Targets    = &Targets;
    }

    Cameras.Update(0.f);
}

void SceneGame :: InitInputMgr()
{
    InputMgr &im = g_InputMgr;
    im.SetScene(sceneName);

    im.SetInputCodeIfKeyIsFree(VK_F11,     IC_FullScreen);
    im.SetInputCodeIfKeyIsFree(VK_RETURN,  IC_Accept);
    im.SetInputCodeIfKeyIsFree(VK_ESCAPE,  IC_Reject);
    im.SetInputCodeIfKeyIsFree(VK_LBUTTON, IC_LClick);

    // Cameras
    im.SetInputCodeIfKeyIsFree(VK_NUMPAD8, IC_TurnUp);
    im.SetInputCodeIfKeyIsFree(VK_NUMPAD5, IC_TurnDown);
    im.SetInputCodeIfKeyIsFree(VK_NUMPAD4, IC_TurnLeft);
    im.SetInputCodeIfKeyIsFree(VK_NUMPAD6, IC_TurnRight);
    im.SetInputCodeIfKeyIsFree('Y', IC_RollLeft);
    im.SetInputCodeIfKeyIsFree('I', IC_RollRight);

    im.SetInputCodeIfKeyIsFree('U', IC_OrbitUp);
    im.SetInputCodeIfKeyIsFree('J', IC_OrbitDown);
    im.SetInputCodeIfKeyIsFree('H', IC_OrbitLeft);
    im.SetInputCodeIfKeyIsFree('K', IC_OrbitRight);

    im.SetInputCodeIfKeyIsFree(VK_HOME,   IC_MoveForward);
    im.SetInputCodeIfKeyIsFree(VK_END,    IC_MoveBack);
    im.SetInputCodeIfKeyIsFree(VK_DELETE, IC_MoveLeft);
    im.SetInputCodeIfKeyIsFree(VK_NEXT,   IC_MoveRight);
    im.SetInputCodeIfKeyIsFree(VK_PRIOR,  IC_MoveUp);
    im.SetInputCodeIfKeyIsFree(VK_INSERT, IC_MoveDown);
    im.SetInputCodeIfKeyIsFree(VK_LSHIFT, IC_RunModifier);

    // ComBoard
    im.SetInputCodeIfKeyIsFree('S', IC_CB_LeftPunch);
    im.SetInputCodeIfKeyIsFree('A', IC_CB_LeftHandGuard);
    im.SetInputCodeIfKeyIsFree('X', IC_CB_LeftKick);
    im.SetInputCodeIfKeyIsFree('Z', IC_CB_LeftLegGuard);
    im.SetInputCodeIfKeyIsFree('D', IC_CB_RightPunch);
    im.SetInputCodeIfKeyIsFree('F', IC_CB_RightHandGuard);
    im.SetInputCodeIfKeyIsFree('C', IC_CB_RightKick);
    im.SetInputCodeIfKeyIsFree('V', IC_CB_RightLegGuard);
    im.SetInputCodeIfKeyIsFree(VK_UP,    IC_CB_Forward);
    im.SetInputCodeIfKeyIsFree(VK_DOWN,  IC_CB_Backward);
    im.SetInputCodeIfKeyIsFree(VK_LEFT,  IC_CB_Left);
    im.SetInputCodeIfKeyIsFree(VK_RIGHT, IC_CB_Right);
}

bool SceneGame :: Invalidate()
{
    Vec_xLight::iterator LT_curr = world.lights.begin(),
                         LT_last = world.lights.end();
    for (; LT_curr != LT_last ; ++LT_curr)
        LT_curr->modified = true;

    WorldRenderGL renderer;
    renderer.Invalidate(world);
    return true;
}

void SceneGame :: Terminate()
{
    DefaultCamera = NULL;
    FreeWorld();
    Targets.L_objects.clear();
    Scene::Terminate();

    Cameras.Free();
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
        MapFileName.clear();
        InitWorld();
        return true;
    }
    if (cmd == "init" || cmd == "reinitialize")
    {
        InitWorld();
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

bool SceneGame :: FrameUpdate(float deltaTime)
{
    InputMgr &im = g_InputMgr;

    if (im.GetInputStateAndClear(IC_Reject))
    {
        if (PrevScene)
        {
            Scene *tmp = PrevScene;
            PrevScene = NULL;
            g_Application.SetCurrentScene(tmp/*new SceneMenu()*/);
        }
        else
            g_Application.MainWindow().Terminate();
        return true;
    }

    if (im.GetInputStateAndClear(IC_FullScreen))
        g_Application.MainWindow().SetFullScreen(!g_Application.MainWindow().FullScreen());

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

    if (im.GetInputStateAndClear(IC_LClick))
    {
        RigidObj *obj = Select(g_InputMgr.mouseX, g_InputMgr.mouseY);
        if (obj)
            if (obj->ModelPh)
                g_Application.SetCurrentScene(new SceneSkeleton(
                    obj->ModelGr->xModelP->FileName, obj->ModelPh->xModelP->FileName), false);
            else
                g_Application.SetCurrentScene(new SceneSkeleton(
                    obj->ModelGr->xModelP->FileName, NULL), false);
        return true;
    }

    world.FrameUpdate(deltaTime);
    Cameras.Update(deltaTime);

    return true;
}

bool SceneGame :: FrameRender()
{
    world.FrameRender();

    static xLight dayLight;
    static xColor skyColor;
    if (dayLight.id == 0)
    {
        dayLight.create();
        dayLight.turned_on = true;
        dayLight.color.init(0.9f, 0.9f, 1.f, 1.f);
        dayLight.softness = 0.4f;
        //dayLight.position.init(-20.f, 20.f, 100.f);
        dayLight.position.init(10.f, 10.f, 10.f);
        dayLight.type = xLight_INFINITE;
        dayLight.attenuationConst  = 0.9f;
        dayLight.attenuationLinear = 0.005f;
        dayLight.attenuationSquare = 0.0005f;
        dayLight.update();

        skyColor.init(0.8f, 0.8f, 1.f, 0.f);
    }

    WorldRenderGL renderer;
    renderer.FreeIfNeeded(world);

    if (Config::EnableLighting)
        if (Config::EnableFullLighting)
            renderer.RenderWorld(world, Cameras);
        else
            renderer.RenderWorld(world, dayLight, skyColor, Cameras);
    else
        renderer.RenderWorldNoLights(world, world.skyColor, Cameras);

    if (Config::DisplayCameras)
    {
        glDisable(GL_DEPTH_TEST);
        glColor3f(1.f,1.f,1.f);
        Math::Cameras::CameraSet::Vec_Camera::iterator
            CAM_curr = Cameras.L_cameras.begin(),
            CAM_curr2 = Cameras.L_cameras.begin(),
            CAM_last = Cameras.L_cameras.end();
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDisable(GL_CULL_FACE);

        for (; CAM_curr2 != CAM_last; ++CAM_curr2)
        {
            ViewportSet_GL(**CAM_curr2);

            CAM_curr = Cameras.L_cameras.begin();
            for (; CAM_curr != CAM_last; ++CAM_curr)
            {
                glBegin(GL_QUADS);
                {
                    glVertex3fv((*CAM_curr)->FOV.Corners3D[0].xyz);
                    glVertex3fv((*CAM_curr)->FOV.Corners3D[1].xyz);
                    glVertex3fv((*CAM_curr)->FOV.Corners3D[2].xyz);
                    glVertex3fv((*CAM_curr)->FOV.Corners3D[3].xyz);
                }
                glEnd();

                glBegin(GL_LINES);
                {
                    glVertex3fv((*CAM_curr)->P_eye.xyz);
                    glVertex3fv((*CAM_curr)->FOV.Corners3D[0].xyz);
                    glVertex3fv((*CAM_curr)->P_eye.xyz);
                    glVertex3fv((*CAM_curr)->FOV.Corners3D[1].xyz);
                    glVertex3fv((*CAM_curr)->P_eye.xyz);
                    glVertex3fv((*CAM_curr)->FOV.Corners3D[2].xyz);
                    glVertex3fv((*CAM_curr)->P_eye.xyz);
                    glVertex3fv((*CAM_curr)->FOV.Corners3D[3].xyz);
                }
                glEnd();
            }
        }
    }

    //////////////////// WORLD - END
    glFlush(); //glFinish();
    return true;
}

////// ISelectionProvider

void SceneGame :: RenderSelect(const Math::Cameras::FieldOfView &FOV)
{
    RendererGL renderer;
    int objectID = -1;
    World::Vec_Object::iterator iter = world.objects.begin(), end = world.objects.end();
    for ( ; iter != end ; ++iter ) {
        glLoadName(++objectID);
        RigidObj &mdl = *(RigidObj*)*iter;
        renderer.RenderVertices(*mdl.ModelGr->xModelP, mdl.ModelGr->instance, Renderer::smModel);
    }
}
RigidObj *SceneGame :: Select(int X, int Y)
{
    if (!DefaultCamera->FOV.ViewportContains(X,Height-Y)) return NULL;
    std::vector<xDWORD> *objectIDs = ISelectionProvider::Select(*DefaultCamera, X, Height-Y);
    return objectIDs == NULL ? NULL : (RigidObj*) world.objects[objectIDs->back()];
}
