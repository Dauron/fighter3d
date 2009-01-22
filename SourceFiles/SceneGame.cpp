#include "SceneGame.h"
#include "SceneMenu.h"
#include "SceneSkeleton.h"

#include "../AppFramework/Application.h"
#include "../AppFramework/Input/InputMgr.h"
#include "InputCodes.h"
#include "../Math/Cameras/CameraHuman.h"
#include "../Math/Cameras/CameraFree.h"

#include "../Graphics/OGL/WorldRenderGL.h"

#include "../World/ObjectTypes.h"

using namespace Scenes;

#define MULT_MOVE   5.0f
#define MULT_RUN    2.0f
#define MULT_ROT    80.0f
#define MULT_STEP   60.0f

bool SceneGame :: Create(int left, int top, unsigned int width, unsigned int height, IScene *prevScene)
{
    IScene::Create(left, top, width, height, prevScene);

    InitInputMgr(); // inside -> InitCameras();
    InitMap();

    wglSwapIntervalEXT(Config::VSync ? 1 : 0);

    font = g_FontMgr.GetFont("Courier New", 12);

    return true;
}

bool SceneGame :: InitMap()
{
    FreeMap();
    Map.Create(MapFileName);

    if (Player1) { Player1->MX_LocalToWorld_Set() = Map.MX_spawn1; Map.objects.push_back(Player1); Player1 = NULL; }
    if (Player2) { Player2->MX_LocalToWorld_Set() = Map.MX_spawn2; Map.objects.push_back(Player2); Player2 = NULL; }

    Targets.L_objects.clear();
    Physics::PhysicalWorld::Vec_Object::iterator
        OB_curr = Map.objects.begin(),
        OB_last = Map.objects.end();

    size_t i = 0;

    for (; OB_curr != OB_last; ++OB_curr)
        if ( (**OB_curr).Type == AI::ObjectType::Human )
        {
            Targets.L_objects.push_back(*OB_curr);
            SkeletizedObj &obj = *(SkeletizedObj*)(*OB_curr);
            obj.Tracker.Targets = &Targets;

            if (pages.size() < i+1)
            {
                pages.push_back(new StatPage());
                g_StatMgr.Add(*pages[i]);
            }
            pages[i]->Name = obj.Name;
            obj.RegisterStats(*pages[i]);

            ++i;
        }

    if (Targets.L_objects.size() >= 2)
    {
        int p1 = Targets.L_objects.size()-2;
        SkeletizedObj &obj1 = *(SkeletizedObj*)Targets.L_objects[p1];
        obj1.Tracker.ID_object = p1+1;
        SkeletizedObj &obj2 = *(SkeletizedObj*)Targets.L_objects.back();
        obj2.Tracker.ID_object = p1;
    }

    //Cameras.Free();
    InitCameras();

    Map.InitialUpdate();

    return true;
}

void SceneGame :: FreeMap()
{
    for (int i = pages.size(); i > 0; --i)
        pages[i-1]->Destroy();

    WorldRenderGL().Free(Map);
    Map.Destroy();
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

            Cameras.L_cameras[0]->CenterTracker.Mode    = Math::Tracking::ObjectTracker::TRACK_ALL_CENTER;
            Cameras.L_cameras[0]->EyeTracker.Mode       = Math::Tracking::ObjectTracker::TRACK_CUSTOM_SCRIPT;
            Cameras.L_cameras[0]->EyeTracker.ScriptName = "EyeSeeAll_Center";
            Cameras.L_cameras[0]->EyeTracker.Script     = Math::Cameras::Camera::SCRIPT_EyeSeeAll_Center;
            Cameras.L_cameras[0]->EyeTracker.NW_destination_shift.init(0.f,0.f,1.5f);
        }

        MainCamera = Cameras.L_cameras[0];
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
    im.SetScene(Name);

    im.Key2InputCode_SetIfKeyFree(VK_F11,     IC_FullScreen);
    im.Key2InputCode_SetIfKeyFree(VK_RETURN,  IC_Accept);
    im.Key2InputCode_SetIfKeyFree(VK_ESCAPE,  IC_Reject);
    im.Key2InputCode_SetIfKeyFree(VK_LBUTTON, IC_LClick);

    if (strcmp(Name, "[Game]") == 0)
    {
        // Cameras
        im.Key2InputCode_SetIfKeyFree(VK_NUMPAD8, IC_TurnUp);
        im.Key2InputCode_SetIfKeyFree(VK_NUMPAD5, IC_TurnDown);
        im.Key2InputCode_SetIfKeyFree(VK_NUMPAD4, IC_TurnLeft);
        im.Key2InputCode_SetIfKeyFree(VK_NUMPAD6, IC_TurnRight);
        im.Key2InputCode_SetIfKeyFree('Y', IC_RollLeft);
        im.Key2InputCode_SetIfKeyFree('I', IC_RollRight);

        im.Key2InputCode_SetIfKeyFree('U', IC_OrbitUp);
        im.Key2InputCode_SetIfKeyFree('J', IC_OrbitDown);
        im.Key2InputCode_SetIfKeyFree('H', IC_OrbitLeft);
        im.Key2InputCode_SetIfKeyFree('K', IC_OrbitRight);

        im.Key2InputCode_SetIfKeyFree(VK_HOME,   IC_MoveForward);
        im.Key2InputCode_SetIfKeyFree(VK_END,    IC_MoveBack);
        im.Key2InputCode_SetIfKeyFree(VK_DELETE, IC_MoveLeft);
        im.Key2InputCode_SetIfKeyFree(VK_NEXT,   IC_MoveRight);
        im.Key2InputCode_SetIfKeyFree(VK_PRIOR,  IC_MoveUp);
        im.Key2InputCode_SetIfKeyFree(VK_INSERT, IC_MoveDown);
        im.Key2InputCode_SetIfKeyFree(VK_LSHIFT, IC_RunModifier);

        // ComBoard set 0 (single player plays on comboard)
        im.Key2InputCode_SetIfKeyFree('S', IC_CB_LeftPunch);
        im.Key2InputCode_SetIfKeyFree('A', IC_CB_LeftHandGuard);
        im.Key2InputCode_SetIfKeyFree('X', IC_CB_LeftKick);
        im.Key2InputCode_SetIfKeyFree('Z', IC_CB_LeftLegGuard);
        im.Key2InputCode_SetIfKeyFree('D', IC_CB_RightPunch);
        im.Key2InputCode_SetIfKeyFree('F', IC_CB_RightHandGuard);
        im.Key2InputCode_SetIfKeyFree('C', IC_CB_RightKick);
        im.Key2InputCode_SetIfKeyFree('V', IC_CB_RightLegGuard);
        im.Key2InputCode_SetIfKeyFree(VK_UP,    IC_CB_Forward);
        im.Key2InputCode_SetIfKeyFree(VK_DOWN,  IC_CB_Backward);
        im.Key2InputCode_SetIfKeyFree(VK_LEFT,  IC_CB_Left);
        im.Key2InputCode_SetIfKeyFree(VK_RIGHT, IC_CB_Right);
    }
    else
    {
        // ComBoard set 1 (first player plays on comboard, second too)
        im.Key2InputCode_SetIfKeyFree('A', IC_CB_LeftPunch1);
        im.Key2InputCode_SetIfKeyFree(VK_CAPITAL, IC_CB_LeftHandGuard1);
        im.Key2InputCode_SetIfKeyFree('Z', IC_CB_LeftKick1);
        im.Key2InputCode_SetIfKeyFree(VK_LSHIFT, IC_CB_LeftLegGuard1);
        im.Key2InputCode_SetIfKeyFree('S', IC_CB_RightPunch1);
        im.Key2InputCode_SetIfKeyFree('D', IC_CB_RightHandGuard1);
        im.Key2InputCode_SetIfKeyFree('X', IC_CB_RightKick1);
        im.Key2InputCode_SetIfKeyFree('C', IC_CB_RightLegGuard1);
        im.Key2InputCode_SetIfKeyFree('T', IC_CB_Forward1);
        im.Key2InputCode_SetIfKeyFree('G', IC_CB_Backward1);
        im.Key2InputCode_SetIfKeyFree('F', IC_CB_Left1);
        im.Key2InputCode_SetIfKeyFree('H', IC_CB_Right1);

        // ComBoard set 2 (second player plays on comboard, first too)
        im.Key2InputCode_SetIfKeyFree('O', IC_CB_LeftPunch2);
        im.Key2InputCode_SetIfKeyFree('I', IC_CB_LeftHandGuard2);
        im.Key2InputCode_SetIfKeyFree('L', IC_CB_LeftKick2);
        im.Key2InputCode_SetIfKeyFree('K', IC_CB_LeftLegGuard2);
        im.Key2InputCode_SetIfKeyFree('P', IC_CB_RightPunch2);
#ifdef WIN32
        im.Key2InputCode_SetIfKeyFree(VK_OEM_4, IC_CB_RightHandGuard2);
        im.Key2InputCode_SetIfKeyFree(VK_OEM_1, IC_CB_RightKick2);
        im.Key2InputCode_SetIfKeyFree(VK_OEM_7, IC_CB_RightLegGuard2);
#else
        im.Key2InputCode_SetIfKeyFree('[', IC_CB_RightHandGuard2);
        im.Key2InputCode_SetIfKeyFree(':', IC_CB_RightKick2);
        im.Key2InputCode_SetIfKeyFree('\'', IC_CB_RightLegGuard2);
#endif
        im.Key2InputCode_SetIfKeyFree(VK_UP,   IC_CB_Forward2);
        im.Key2InputCode_SetIfKeyFree(VK_DOWN,    IC_CB_Backward2);
        im.Key2InputCode_SetIfKeyFree(VK_LEFT, IC_CB_Left2);
        im.Key2InputCode_SetIfKeyFree(VK_RIGHT,   IC_CB_Right2);
    }
}

bool SceneGame :: Invalidate()
{
    Vec_xLight::iterator LT_curr = Map.lights.begin(),
                         LT_last = Map.lights.end();
    for (; LT_curr != LT_last ; ++LT_curr)
        LT_curr->modified = true;
    WorldRenderGL().Invalidate(Map);
    return true;
}

void SceneGame :: Destroy()
{
    IScene::Destroy();

    g_FontMgr.Release(font);
    font = HFont();

    Cameras.Free();
    FreeMap();
    for (int i = pages.size(); i > 0; --i)
        g_StatMgr.Remove(*pages[i-1]);
    Clear();
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
    init map            | initm         | reinitialize map (no players)\n\
    init cam            | initc         | reinitialize cameras\n\
    level {int}         | level {int}   | load 'level_{int}.map' scene\n\
    speed {float}       | speed {float} | enter clock speed multiplier\n\
    ------------------------------------------------------------------------\n\
    toggle_lights       | tls           | turns the lighting on and off\n\
    toggle_shaders      | tshdr         | toggles GPU shaders\n\
    toggle_shadows      | tshdw         | toggles shadow rendering\n\
    toggle_shadow_vol   | tshdv         | toggles shadow volume rendering\n\
    toggle_skeleton     | tskel         | toggles skeleton rendering\n\
    toggle_bvh          | tbvh          | toggles BVH rendering\n\
    toggle_cameras      | tcam          | toggles cameras rendering\n\
    toggle_polygon_mode | tpm           | toggles polygon mode\n\
    ------------------------------------------------------------------------\n\
    ");
        return true;
    }
    if (cmd.substr(0, 6) == "level ")
    {
        Config::TestCase = atoi(cmd.substr(6).c_str());
        MapFileName.clear();
        Cameras.Free();
        InitMap();
        return true;
    }
    if (cmd == "initm" || cmd == "init map")
    {
        InitMap();
        return true;
    }
    if (cmd == "initc" || cmd == "init cam")
    {
        Cameras.Free();
        InitCameras();
        return true;
    }
    if (cmd.substr(0, 6) == "speed ")
    {
        Config::Speed = atof(cmd.substr(6).c_str());
        return true;
    }
    if (cmd == "tls" || cmd == "toggle_lights")
    {
        if ( (Config::EnableLighting = !Config::EnableLighting) )
            output.append("\nThe lights are ON.\n");
        else
            output.append("\nThe lights are OFF.\n");
        return true;
    }
    if (cmd.substr(0, 4) == "tls ")
    {
        unsigned int id = (unsigned int)atoi(cmd.substr(4).c_str());
        if (id >= 0 && id < Map.lights.size())
            Map.lights[id].turned_on = !Map.lights[id].turned_on;
        return true;
    }
    if (cmd == "tshdw" || cmd == "toggle_shadows")
    {
        if ( (Config::EnableShadows = !Config::EnableShadows) )
            output.append("\nThe shadows are ON.\n");
        else
            output.append("\nThe shadows are OFF.\n");
        return true;
    }
    if (cmd == "tshdv" || cmd == "toggle_shadow_vol")
    {
        if ( (Config::DisplayShadowVolumes = !Config::DisplayShadowVolumes) )
            output.append("\nShadow volumes drawing is ON.\n");
        else
            output.append("\nShadow volumes drawing is OFF.\n");
        return true;
    }
    if (cmd == "tskel" || cmd == "toggle_skeleton")
    {
        if ( (Config::DisplaySkeleton = !Config::DisplaySkeleton) )
            output.append("\nSkeleton drawing is ON.\n");
        else
            output.append("\nSkeleton drawing is OFF.\n");
        return true;
    }
    if (cmd == "tbvh" || cmd == "toggle_bvh")
    {
        if ( (Config::DisplayBVH = !Config::DisplayBVH) )
            output.append("\nBVH drawing is ON.\n");
        else
            output.append("\nBVH drawing is OFF.\n");
        return true;
    }
    if (cmd == "tcam" || cmd == "toggle_cameras")
    {
        if ( (Config::DisplayCameras = !Config::DisplayCameras) )
            output.append("\nCameras drawing is ON.\n");
        else
            output.append("\nCameras drawing is OFF.\n");
        return true;
    }
    if (cmd == "tshdr" || cmd == "toggle_shaders")
    {
        if ( (Config::EnableShaders = !Config::EnableShaders) )
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

bool SceneGame :: Update(float T_delta)
{
    InputMgr &im = g_InputMgr;

    if (im.InputDown_GetAndRaise(IC_Reject))
    {
        if (PrevScene)
        {
            IScene &tmp = *PrevScene;
            PrevScene = NULL;
            g_Application.Scene_Set(tmp);
        }
        else
            g_Application.Destroy();
        return true;
    }

    if (im.InputDown_GetAndRaise(IC_FullScreen))
    {
        if (g_Application.MainWindow_Get().IsFullScreen())
            g_Application.MainWindow_Get().FullScreen_Set(Config::WindowX, Config::WindowY, false);
        else
            g_Application.MainWindow_Get().FullScreen_Set(Config::FullScreenX, Config::FullScreenY, true);
        return true;
    }

    float run = (im.InputDown_Get(IC_RunModifier)) ? MULT_RUN : 1.0f;
    float T_scaled = T_delta*MULT_ROT*run;

    if (im.InputDown_Get(IC_TurnLeft))
        MainCamera->Rotate (T_scaled, 0.0f, 0.0f);
    if (im.InputDown_Get(IC_TurnRight))
        MainCamera->Rotate (-T_scaled, 0.0f, 0.0f);
    if (im.InputDown_Get(IC_TurnUp))
        MainCamera->Rotate (0.0f, T_scaled, 0.0f);
    if (im.InputDown_Get(IC_TurnDown))
        MainCamera->Rotate (0.0f, -T_scaled, 0.0f);
    if (im.InputDown_Get(IC_RollLeft))
        MainCamera->Rotate (0.0f, 0.0f, -T_scaled);
    if (im.InputDown_Get(IC_RollRight))
        MainCamera->Rotate (0.0f, 0.0f, T_scaled);

    if (im.InputDown_Get(IC_OrbitLeft))
        MainCamera->Orbit (-T_scaled, 0.0f);
    if (im.InputDown_Get(IC_OrbitRight))
        MainCamera->Orbit (T_scaled, 0.0f);
    if (im.InputDown_Get(IC_OrbitUp))
        MainCamera->Orbit (0.0f, T_scaled);
    if (im.InputDown_Get(IC_OrbitDown))
        MainCamera->Orbit (0.0f, -T_scaled);

    T_scaled = T_delta*MULT_MOVE*run;

    bool moving = false;
    if (im.InputDown_Get(IC_MoveForward))
    {
        MainCamera->Move (T_scaled, 0.0f, 0.0f);
        moving = true;
    }
    if (im.InputDown_Get(IC_MoveBack))
    {
        MainCamera->Move (-T_scaled, 0.0f, 0.0f);
        moving = true;
    }
    if (im.InputDown_Get(IC_MoveLeft))
    {
        MainCamera->Move (0.0f, -T_scaled, 0.0f);
        moving = true;
    }
    if (im.InputDown_Get(IC_MoveRight))
    {
        MainCamera->Move (0.0f, T_scaled, 0.0f);
        moving = true;
    }
    if (im.InputDown_Get(IC_MoveUp))
        MainCamera->Move (0.0f, 0.0f, T_scaled);
    if (im.InputDown_Get(IC_MoveDown))
        MainCamera->Move (0.0f, 0.0f, -T_scaled);

    if (im.InputDown_GetAndRaise(IC_LClick))
    {
        RigidObj *obj = Select(g_InputMgr.mouseX, Height-g_InputMgr.mouseY);
        if (obj)
        {
            if (obj->ModelPh)
                g_Application.Scene_Set(* new SceneSkeleton(
                    obj->ModelGr->xModelP->FileName, obj->ModelPh->xModelP->FileName), false);
            else
                g_Application.Scene_Set(* new SceneSkeleton(
                    obj->ModelGr->xModelP->FileName, NULL), false);
        }
        return true;
    }

    Map.Update(T_delta);
    Cameras.Update(T_delta);

    return true;
}

#include "../Graphics/OGL/Shader.h"

bool SceneGame :: Render()
{
    Profile("Render game");
    Map.Render();

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
    renderer.FreeIfNeeded(Map);

    if (Config::EnableLighting)
        if (Config::EnableFullLighting && Map.lights.size())
            renderer.RenderWorld(Map, Cameras);
        else
            renderer.RenderWorld(Map, dayLight, skyColor, Cameras);
    else
        renderer.RenderWorldNoLights(Map, Map.skyColor, Cameras);

    if (Config::DisplayCameras)
    {
        Profile("Display camera's avatars");

        glDisable(GL_DEPTH_TEST);
        glColor3f(1.f,1.f,1.f);
        Math::Cameras::CameraSet::Vec_Camera::iterator
            CAM_curr  = Cameras.L_cameras.begin(),
            CAM_curr2 = Cameras.L_cameras.begin(),
            CAM_last  = Cameras.L_cameras.end();
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

    // render UI
    if (Targets.L_objects.size() >= 2)
    {
        glDisable(GL_DEPTH_TEST);  // Disable depth testing
        Graphics::OGL::Shader::SetLightType(xLight_NONE);
        Graphics::OGL::Shader::EnableTexturing(Graphics::OGL::xState_Off);
        glDisable (GL_POLYGON_SMOOTH);

        // Set projection
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, Width, Height, 0, 0, 100);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        //const GLFont* pFont = g_FontMgr.GetFont(font);
        //float lineHeight = pFont->LineH();

        glViewport(Left, Top, Width, Height); // Set viewport

        SkeletizedObj &player1 = *(SkeletizedObj*) *(Targets.L_objects.rbegin()+1);
        SkeletizedObj &player2 = *(SkeletizedObj*) Targets.L_objects.back();

        const Graphics::OGL::Font* pFont = g_FontMgr.GetFont(font);
        float lineHeight = pFont->LineH();

        pFont->Print(10.f, lineHeight, 0, ("Player 1 : " + player1.Name).c_str());
        std::string name_p2 = player2.Name + " : Player 2";
        pFont->Print(Width - 10.f - pFont->Length(name_p2.c_str()),
            lineHeight, 0, name_p2.c_str());

        xFLOAT BarWidth = Width * 0.5f - 20.f;
        xFLOAT LifeWidthP1 = player1.LifeEnergy == 0.f ? 0.f
            : max(1.f, BarWidth * player1.LifeEnergy / player1.LifeEnergyMax);
        xFLOAT LifeWidthP2 = player2.LifeEnergy == 0.f ? 0.f
            : max(1.f, BarWidth * player2.LifeEnergy / player2.LifeEnergyMax);

        // Draw life-bars
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glColor3ub( 255, 0, 0 );
        glBegin(GL_QUADS);
        {
            if (player1.FL_recovering)
                glColor3ub( 255, 0, 0 );
            else
                glColor3ub( 0, 255, 0 );
            glVertex2f(10.f, lineHeight+10.f);
            glVertex2f(10.f + LifeWidthP1, lineHeight+10.f);
            glVertex2f(10.f + LifeWidthP1, lineHeight+20.f);
            glVertex2f(10.f, lineHeight+20.f);

            if (player2.FL_recovering)
                glColor3ub( 255, 0, 0 );
            else
                glColor3ub( 0, 255, 0 );glVertex2f(Width - 10.f, lineHeight+10.f);
            glVertex2f(Width - 10.f - LifeWidthP2, lineHeight+10.f);
            glVertex2f(Width - 10.f - LifeWidthP2, lineHeight+20.f);
            glVertex2f(Width - 10.f, lineHeight+20.f);
        }
        glEnd();
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glColor3ub( 255, 255, 255 );
        glBegin(GL_QUADS);
        {
            glVertex2f(9.f, lineHeight+10.f);
            glVertex2f(11.f + BarWidth, lineHeight+10.f);
            glVertex2f(11.f + BarWidth, lineHeight+20.f);
            glVertex2f(9.f, lineHeight+20.f);

            glVertex2f(Width - 9.f, lineHeight+10.f);
            glVertex2f(Width - 11.f - BarWidth, lineHeight+10.f);
            glVertex2f(Width - 11.f - BarWidth, lineHeight+20.f);
            glVertex2f(Width - 9.f, lineHeight+20.f);
        }
        glEnd();
    }

    glFlush(); //glFinish();
    return true;
}

////// ISelectionProvider

void SceneGame :: RenderSelect(const Math::Cameras::FieldOfView &FOV)
{
    RendererGL renderer;
    int ID_object = 0;
    World::Vec_Object::iterator OB_curr = Map.objects.begin(),
                                OB_last = Map.objects.end();
    for ( ; OB_curr != OB_last ; ++OB_curr, ++ID_object ) {
        glLoadName(ID_object);
        RigidObj &mdl = *(RigidObj*)*OB_curr;
        renderer.RenderVertices(*mdl.ModelGr->xModelP, mdl.ModelGr->instance, Renderer::smModel);
    }
}
RigidObj *SceneGame :: Select(int X, int Y)
{
    if (!MainCamera->FOV.ViewportContains(X,Y)) return NULL;
    std::vector<xDWORD> *objectIDs = ISelectionProvider::Select(*MainCamera, X, Y);
    return objectIDs == NULL ? NULL : (RigidObj*) Map.objects[objectIDs->back()];
}
