#include "SceneWorld.h"

#include "../AppFramework/Application.h"
#include "../AppFramework/Input/InputMgr.h"
#include "InputCodesWorld.h"
#include "../Graphics/OGL/Shader.h"
#include "../Graphics/OGL/Utils.h"
#include "../Graphics/Textures/TextureMgr.h"
#include "../AppFramework/OGL/GLWindow.h"

#include "../Utils/Debug.h"

namespace Scenes {

// Hawk following script
void Script_Eye_Follow_Hawk(Math::Cameras::ObjectTracker &tracker, xBYTE *CameraDataPtr)
{
    if (tracker.Targets->L_objects.size() == 0) return;
    Math::Cameras::CameraTrackingData &ctd = *(Math::Cameras::CameraTrackingData*) CameraDataPtr;

    World::Hawk *bird = (World::Hawk*)tracker.Targets->L_objects[tracker.ID_object];
    xVector3 NW_dir = bird->V_velocity+bird->V_initial_speed;
    NW_dir.z += bird->Gravity;
    if (NW_dir.lengthSqr() > 1.f)
        tracker.P_destination = bird->P_center_Trfm - 20*NW_dir.normalize();
    else
    {
        if (ctd.camera->P_eye.z - ctd.camera->P_center.z < 100.f)
            tracker.P_destination.z += 10.f;
    }
}
// Bird following script
void Script_Eye_Follow_Bird(Math::Cameras::ObjectTracker &tracker, xBYTE *CameraDataPtr)
{
    if (tracker.Targets->L_objects.size() == 0) return;
    Math::Cameras::CameraTrackingData &ctd = *(Math::Cameras::CameraTrackingData*) CameraDataPtr;

    World::Bird *bird = (World::Bird*)tracker.Targets->L_objects[tracker.ID_object];
    if (bird->V_velocity.lengthSqr() > 1.f)
        tracker.P_destination = bird->P_center_Trfm - 20*xVector3::Normalize(bird->V_velocity);
    else
    {
        if (ctd.camera->P_eye.z - ctd.camera->P_center.z < 100.f)
            tracker.P_destination.z += 10.f;
    }
}
// Follow the bird that was previously a followed hawk
void Map_OnHawkReplace(World::Map &map, void* receiver, World::Hawk *&hawk, World::Bird *&bird)
{ ((SceneWorld*)receiver)->HawkReplace(hawk, bird); }

void SceneWorld :: HawkReplace(World::Hawk *hawk, World::Bird *bird)
{
    if (CamFollow.EyeTracker.Targets->L_objects.size() > 0 && CamFollow.EyeTracker.Targets->L_objects[0] == hawk)
    {
        CamFollow.EyeTracker.Script = Script_Eye_Follow_Bird;
        CamFollow.EyeTracker.Targets->L_objects[0]    = bird;
        CamFollow.CenterTracker.Targets->L_objects[0] = bird;
    }
}
// Follow the hawk that ate previously followed bird
void Map_OnBirdRemove(World::Map &map, void* receiver, World::Hawk *&hawk, World::Bird *&bird)
{ ((SceneWorld*)receiver)->BirdRemove(hawk, bird); }

void SceneWorld :: BirdRemove(World::Hawk *hawk, World::Bird *bird)
{
    if (CamFollow.EyeTracker.Targets->L_objects.size() > 0 && CamFollow.EyeTracker.Targets->L_objects[0] == bird)
    {
        CamFollow.EyeTracker.Script = Script_Eye_Follow_Hawk;
        CamFollow.EyeTracker.Targets->L_objects[0]    = hawk;
        CamFollow.CenterTracker.Targets->L_objects[0] = hawk;
    }
}

/////////////////////////////////////

bool SceneWorld :: Create(int left, int top, unsigned int width, unsigned int height, IScene *prevScene)
{
    Clear();

    IScene::Create(left, top, width, height, prevScene);
    InitInputMgr();

    deferred.Create(width, height);

    // Create map
    map.Create();
    map.OnHawkReplace.Set(this, Map_OnHawkReplace);
    map.OnBirdRemove.Set(this, Map_OnBirdRemove);
    mapRender.Create(map);

    Camera.Init(0.f, 0.f, 1.5f, 1.f, 1.f, 1.5f, 0.f, 0.f, 1.f);
    // Prepare tracking camera
    CamFollow.Init(0.f, 0.f, 1.5f, 1.f, 1.f, 1.5f, 0.f, 0.f, 1.f);
    CamFollow.EyeTracker.Targets = new Math::Tracking::TrackingSet();
    CamFollow.EyeTracker.Mode   = Math::Tracking::ObjectTracker::TRACK_CUSTOM_SCRIPT;
    CamFollow.W_TrackingSpeedEye = 5.f;
    CamFollow.W_TrackingSpeedCtr = 100.f;
    CamFollow.CenterTracker.Targets = new Math::Tracking::TrackingSet();
    CamFollow.CenterTracker.Mode = Math::Tracking::ObjectTracker::TRACK_OBJECT;

    // Create lights
    sun.id        = 1;
    sun.turned_on = true;
    sun.type      = xLight_INFINITE;
    sun.position.init(-10,-10,10);
    sun.color.init(1.f,1.f,0.9f,1.f);
    if (World::Map::Night)
        sun.color.vector3 *= 0.5f;
    sun.softness  = 0.6f;
    sun.modified  = true;
    sun.update();
    torch.id        = 2;
    torch.turned_on = true;
    torch.type      = xLight_POINT;
    torch.position.init(0,0,1.5);
    torch.color.init(1.f,1.f,0.f,1.f);
    torch.softness  = 0.2f;
    torch.attenuationConst  = 1.f;
    torch.attenuationLinear = 0.5f;
    torch.attenuationSquare = 0.f;//0001f;
    torch.modified  = true;
    torch.update();

    return true;
}

void SceneWorld :: Destroy()
{
    deferred.Destroy();
    mapRender.Destroy();
    map.Destroy();
    g_FontMgr.Release(font02);
    g_FontMgr.Release(font04);
    font02 = font04 = HFont();
    IScene::Destroy();
}

/////////////////////////////////////

bool SceneWorld :: Invalidate()
{
    deferred.Clear();
    mapRender.Invalidate();
    return true;
}

void SceneWorld :: Resize(int left, int top, unsigned int width, unsigned int height)
{
    IScene::Resize(left, top, width, height);

    if (!deferred.IsValid())
        deferred.Create(Width, Height);
    deferred.Resize(width, height);

    Camera.FOV.InitPerspective(45.f, 0.1f, 10000.f);
    Camera.FOV.InitViewport(left,top,width,height);
    CamFollow.FOV.InitPerspective(45.f, 0.1f, 10000.f);
    CamFollow.FOV.InitViewport(left,top,width,height);
#ifdef WIN32
    ShowCursor(false);
#endif
    SetVSync_GL(Config::VSync);

    g_FontMgr.Release(font04);
    font04 = g_FontMgr.GetFont("Arial", (int)(Height * 0.035f));
    g_FontMgr.Release(font02);
    font02 = g_FontMgr.GetFont("Arial", (int)(Height * 0.025f));
}

/////////////////////////////////////

void SceneWorld :: InitInputMgr()
{
    InputMgr &im = g_InputMgr;
    im.SetScene(Name);

    im.Key2InputCode_SetIfKeyFree(VK_RETURN,  IC_ReturnToPlayer/*IC_Accept*/);
    im.Key2InputCode_SetIfKeyFree(VK_ESCAPE,  IC_Reject);
    im.Key2InputCode_SetIfKeyFree(VK_SHIFT,   IC_RunModifier);
    im.Key2InputCode_SetIfKeyFree(VK_LBUTTON, IC_LClick);
    im.Key2InputCode_SetIfKeyFree(VK_RBUTTON, IC_ReturnToPlayer/*IC_RClick*/);
    im.Key2InputCode_SetIfKeyFree(VK_F11,     IC_FullScreen);

    // Cameras
    im.Key2InputCode_SetIfKeyFree('W', IC_MoveForward);
    im.Key2InputCode_SetIfKeyFree('S', IC_MoveBack);
    im.Key2InputCode_SetIfKeyFree('A', IC_MoveLeft);
    im.Key2InputCode_SetIfKeyFree('D', IC_MoveRight);
    im.Key2InputCode_SetIfKeyFree('E', IC_MoveUp);
    im.Key2InputCode_SetIfKeyFree('Q', IC_MoveDown);

    im.Key2InputCode_SetIfKeyFree(VK_UP,    IC_TurnUp);
    im.Key2InputCode_SetIfKeyFree(VK_DOWN,  IC_TurnDown);
    im.Key2InputCode_SetIfKeyFree(VK_LEFT,  IC_TurnLeft);
    im.Key2InputCode_SetIfKeyFree(VK_RIGHT, IC_TurnRight);

    im.Key2InputCode_SetIfKeyFree(VK_PRIOR,  IC_ZoomIn);
    im.Key2InputCode_SetIfKeyFree(VK_NEXT,   IC_ZoomOut);
}


bool SceneWorld :: Update(float T_delta)
{
    InputMgr &im = g_InputMgr;

    // Return to previous scene or close the app
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
    // Enter Full Screen mode
    if (im.InputDown_GetAndRaise(IC_FullScreen))
    {
        if (g_Application.MainWindow_Get().IsFullScreen())
            g_Application.MainWindow_Get().FullScreen_Set(Config::WindowX, Config::WindowY, false);
        else
            g_Application.MainWindow_Get().FullScreen_Set(Config::FullScreenX, Config::FullScreenY, true);
        return true;
    }

    if (!FL_followHawk)
    {
        // Rotate camera with keyboard
        float T_scaled = T_delta*80.0f;
        if (im.InputDown_Get(IC_TurnLeft))
            Camera.Rotate (T_scaled*2, 0.0f, 0.0f);
        if (im.InputDown_Get(IC_TurnRight))
            Camera.Rotate (-T_scaled*2, 0.0f, 0.0f);
        if (im.InputDown_Get(IC_TurnUp))
            Camera.Rotate (0.0f, T_scaled, 0.0f);
        if (im.InputDown_Get(IC_TurnDown))
            Camera.Rotate (0.0f, -T_scaled, 0.0f);
        // Rotate camera with mouse
        if (mouseX > 0 || mouseY > 0)
        {
            int deltaX = mouseX - im.mouseX;
            int deltaY = mouseY - im.mouseY;

            if (deltaX != 0 || deltaY != 0) {
                Camera.Rotate ((float)deltaX, (float)deltaY, 0);
            }
        }
#ifdef WIN32
        if (mouseX < 50 || mouseX > Width - 50 ||
            mouseY < 50 || mouseY > Height - 50)
        {
            POINT point;
            point.x = im.mouseX = Width >> 1;
            point.y = im.mouseY = Height >> 1;
            if (ClientToScreen(((GLWindow*)&g_Application.MainWindow_Get())->HWND(), &point))
                SetCursorPos(point.x, point.y);
        }
#endif
        mouseX = im.mouseX;
        mouseY = im.mouseY;

        xPoint3 P_eye_old = Camera.P_eye;

        float run = im.InputDown_Get(IC_RunModifier) ? 10.f : 1.f;
        bool  moving = false;
        // Move camera with keyboard
        T_scaled = T_delta*run*25.f;
        if (im.InputDown_Get(IC_MoveLeft))
        { moving = true; Camera.Move (0.0f, -T_scaled, 0.0f); }
        if (im.InputDown_Get(IC_MoveRight))
        { moving = true; Camera.Move (0.0f, T_scaled, 0.0f); }
        if (im.InputDown_Get(IC_MoveForward))
        { moving = true; Camera.Move (T_scaled, 0.0f, 0.0f); }
        if (im.InputDown_Get(IC_MoveBack))
        { moving = true; Camera.Move (-T_scaled, 0.0f, 0.0f); }
        if (im.InputDown_Get(IC_MoveUp))
            Camera.Move (0.0f, 0.0f, T_scaled);
        if (im.InputDown_Get(IC_MoveDown))
            Camera.Move (0.0f, 0.0f, -T_scaled);
        // Move camera with wheel
        if (im.InputDown_Get(IC_ZoomIn) || im.mouseWheel > 0)
        {
            moving = true;
            Camera.Move (4.f * T_scaled, 0.0f, 0.0f);
            im.mouseWheel -= 10;
            if (im.mouseWheel < 0) im.mouseWheel = 0;
        }
        if (im.InputDown_Get(IC_ZoomOut) || im.mouseWheel < 0)
        {
            moving = true;
            Camera.Move (-4.f * T_scaled, 0.0f, 0.0f);
            im.mouseWheel += 10;
            if (im.mouseWheel > 0) im.mouseWheel = 0;
        }
        // Simulate human step (up and down wave)
        if (moving)
            Camera.MakeStep(T_delta*run*100.f);

        // Get position over the city
        float x = Camera.P_eye.x - map.MinX;
        float y = Camera.P_eye.y - map.MinY;
        // Get current Quarter's col and row
        int Col = (int)(x * World::Quarter::SquareWidth_Inv);
        int Row = (int)(y * World::Quarter::SquareWidth_Inv);
        // Clip position if we are over the map
        if (Col >= 0 && Col < map.Cols && Row >= 0 && Row < map.Rows)
        {
            // Get position over the current Quarter
            xVector3 P_quarter_pos, P_quarter_pos_Old;
            P_quarter_pos.x = x - Col * World::Quarter::SquareWidth;
            P_quarter_pos.y = y - Row * World::Quarter::SquareWidth;
            P_quarter_pos.z = Camera.P_eye.z;
            P_quarter_pos_Old.x = P_eye_old.x - map.MinX -  Col * World::Quarter::SquareWidth;
            P_quarter_pos_Old.y = P_eye_old.y - map.MinY -  Row * World::Quarter::SquareWidth;
            P_quarter_pos_Old.z = P_eye_old.z;

            World::Quarter *quarter = map.Grid + (map.Rows - Row - 1) * map.Cols + Col;
            xPlane PN_collision;

            while ( map.GetNearestCollision(PN_collision, 1.f, *quarter,
                                            P_quarter_pos, P_quarter_pos_Old) )
            {
                if (PN_collision.z != 0.f)  // ground/roof bounce
                    P_quarter_pos.z   = PN_collision.w + PN_collision.z * 0.5f;
                else
                if (PN_collision.x != 0.f) // west/east wall bounce
                    P_quarter_pos.x   = PN_collision.w + PN_collision.x * 0.5f;
                else
                if (PN_collision.y != 0.f) // north/south wall bounce
                    P_quarter_pos.y   = PN_collision.w + PN_collision.y * 0.5f;
            }

            xVector3 NW_dir = Camera.P_center - Camera.P_eye;
            Camera.P_eye.init(
                P_quarter_pos.x + map.MinX + Col*World::Quarter::SquareWidth,
                P_quarter_pos.y + map.MinY + Row*World::Quarter::SquareWidth,
                P_quarter_pos.z);
            Camera.P_center = Camera.P_eye + NW_dir;
        }
    }
    else
    {
        if (im.InputDown_GetAndRaise(IC_ReturnToPlayer))
        {
            FL_followHawk = false;
#ifdef WIN32
            POINT point;
            point.x = im.mouseX = mouseX = Width >> 1;
            point.y = im.mouseY = mouseY = Height >> 1;
            if (ClientToScreen(((GLWindow*)&g_Application.MainWindow_Get())->HWND(), &point))
                SetCursorPos(point.x, point.y);
#endif
        }
    }

    // Send a Hawk
    if (im.InputDown_GetAndRaise(IC_LClick))
    {
        World::Hawk *hawk = new World::Hawk();
        hawk->Create(map, Camera.P_eye, Camera.P_center - Camera.P_eye);

        CamFollow.P_eye    = Camera.P_eye;
        CamFollow.P_center = Camera.P_center;
        CamFollow.EyeTracker.Script = Script_Eye_Follow_Hawk;
        CamFollow.EyeTracker.Targets->L_objects.clear();
        CamFollow.EyeTracker.Targets->L_objects.push_back(hawk);
        CamFollow.CenterTracker.Targets->L_objects.clear();
        CamFollow.CenterTracker.Targets->L_objects.push_back(hawk);
        FL_followHawk = true;
    }

    // Update world
    map.Update(T_delta);
    // Update cameras
    if (FL_followHawk)
    {
        CamFollow.Update(T_delta);
        torch.position = CamFollow.EyeTracker.Targets->L_objects[0]->P_center_Trfm;// CamFollow.P_eye;
    }
    else
    {
        Camera.Update(T_delta);
        torch.position = Camera.P_eye;
    }

    return false;
}

/////////////////////////////////////

bool SceneWorld :: Render()
{
    /////////////////////// Render world
    if (!deferred.IsValid())
        deferred.Create(Width, Height);

    if (World::Map::Night)
        deferred.Stage1_MRT(xColor::Create(0.2f, 0.2f, 0.5f, 0.f));
    else
        deferred.Stage1_MRT(xColor::Create(0.8f, 0.8f, 1.f, 0.f));

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_POLYGON_SMOOTH);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    Math::Cameras::Camera *cam = FL_followHawk ? &CamFollow : &Camera;
    ViewportSet_GL(*cam);

    mapRender.RenderCity(cam->FOV);
    mapRender.RenderBirds(cam->FOV);

    deferred.Stage2_PreLights();
    deferred.Stage2_InfiniteLight(sun);

    torch.color.init(2.0f,2.0f,0.f,1.f);
    torch.ambient.init(torch.color.vector3 * torch.softness, 1.f);
    torch.diffuse.init(torch.color.vector3 - torch.ambient.vector3, 1.f);
    World::LstBirdP::const_iterator BRD_curr = map.Birds.begin(),
                                    BRD_last = map.Birds.end();
    for(; BRD_curr != BRD_last; ++BRD_curr)
    {
        World::Bird &bird = **BRD_curr;
        torch.position = bird.P_center_Trfm;
        deferred.Stage2_PointLight(torch, *cam);
    }
    torch.color.init(2.f,0.f,0.f,1.f);
    torch.ambient.init(torch.color.vector3 * torch.softness, 1.f);
    torch.diffuse.init(torch.color.vector3 - torch.ambient.vector3, 1.f);
    World::LstHawkP::const_iterator HWK_curr = map.Hawks.begin(),
                                    HWK_last = map.Hawks.end();
    for(; HWK_curr != HWK_last; ++HWK_curr)
    {
        World::Hawk &bird = **HWK_curr;
        torch.position = bird.P_center_Trfm;
        deferred.Stage2_PointLight(torch, *cam);
    }

    deferred.Stage2_PostLights();
    deferred.Stage3_Join();

    //deferred.Stage4_PreFilters();

    //deferred.Stage4_Resize(0.5f);

    //xFLOAT lapEdge3[] = {  1, -2,  1,
    //                      -2,  5, -2,
    //                       1, -2,  1 };
    //deferred.Stage4_ConvolutionFilter(lapEdge3);

    //deferred.Stage4_Tint(xColor::Create(3.f,3.f,3.f,1.f), 0.5f);

    //xFLOAT gauss3[] = { 1 / 16.f, 2 / 16.f, 1 / 16.f,
    //                    2 / 16.f, 4 / 16.f, 2 / 16.f,
    //                    1 / 16.f, 2 / 16.f, 1 / 16.f };
    //deferred.Stage4_ConvolutionFilter(gauss3);

    //deferred.Stage4_Resize(2.f);

    deferred.Stage8_Fin();
    deferred.Stage9_Test();

    /////////////////////// Render overlay
    {
        Profile("Overlay");

        // Set projection
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, Width, 0, Height, 0, 100);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);

        const Graphics::OGL::Font* pFont02 = g_FontMgr.GetFont(font02);
        const Graphics::OGL::Font* pFont04 = g_FontMgr.GetFont(font04);
        float lineH02 = pFont02->LineH();
        float y = Height - lineH02;

        // Print some stats:
        glColor3ub(255,255,255);
        pFont02->PrintF(10, y, 0, "FPS: %d", (int)Performance.FPSsnap);
        y -= lineH02;
        pFont02->PrintF(10, y, 0, "Birds: %d", map.Birds.size());
        y -= lineH02;
        pFont02->PrintF(10, y, 0, "Hawks: %d", map.Hawks.size());

        // Display hawks' energy status
        if (map.Hawks.size() > 0)
        {
            const char *text = "Most recent hawks:";
            y -= lineH02;
            pFont02->Print(10, y, 0.f, text);

            int maxWidth = Width >> 2;
            unsigned int displayCnt = (unsigned int)((y - pFont04->LineH() - 20) / lineH02);

            World::LstHawkP::iterator HWK_curr = map.Hawks.begin(),
                                      HWK_last = map.Hawks.end();
            // Display most recent hawks
            if (map.Hawks.size() > displayCnt)
                for(int first = map.Hawks.size() - displayCnt; first; --first)
                    ++HWK_curr;

            for (; HWK_curr != HWK_last; ++HWK_curr)
            {
                float width;
                if ((*HWK_curr)->Energy >= World::Hawk::InitialEnergy)
                    width = (float)maxWidth;
                else
                    width = maxWidth * (*HWK_curr)->Energy / World::Hawk::InitialEnergy;

                y -= lineH02;

                glColor3ub( 255, 0, 0 );
                glBegin(GL_QUADS);
                {
                    glVertex2f(10, y);
                    glVertex2f(10 + width, y);
                    glVertex2f(10 + width, y + pFont02->Size * 0.5f);
                    glVertex2f(10, y + pFont02->Size * 0.5f);
                }
                glEnd();

                glColor3ub( 255, 255, 255 );
                pFont02->PrintF(20 + width, y, 0.f, "ate %d birds", (*HWK_curr)->BirdsEaten);
            }
        }

        if (!FL_followHawk)
        {
            // Render Crosshair
            float dist = (Camera.FOV.Corners3D[0] - Camera.FOV.Corners3D[1]).length();
            float scale = Width / dist;
            float crossWidth = scale * 0.01f;     // 1cm
            float crossSpan  = crossWidth * 0.05f; // 0,5mm

            glColor4ub(255,255,255, 128);
            glEnable (GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBegin(GL_LINES);
            {
                glVertex2f((Width-crossWidth) * 0.5f, Height * 0.5f - crossSpan);
                glVertex2f((Width+crossWidth) * 0.5f, Height * 0.5f - crossSpan);
                glVertex2f((Width-crossWidth) * 0.5f, Height * 0.5f + crossSpan);
                glVertex2f((Width+crossWidth) * 0.5f, Height * 0.5f + crossSpan);

                glVertex2f(Width * 0.5f - crossSpan, (Height-crossWidth) * 0.5f);
                glVertex2f(Width * 0.5f - crossSpan, (Height+crossWidth) * 0.5f);
                glVertex2f(Width * 0.5f + crossSpan, (Height-crossWidth) * 0.5f);
                glVertex2f(Width * 0.5f + crossSpan, (Height+crossWidth) * 0.5f);
            }
            glEnd();
            glDisable (GL_BLEND);
        }
        else
        {
            World::Bird *bird = (World::Bird*) CamFollow.EyeTracker.Targets->L_objects[0];
            if (bird->Type == OBJECT_HAWK)
            {
                World::Hawk *hawk = (World::Hawk*)bird;

                // Draw energy bar
                int   center   = Width >> 1;
                int   maxWidth = Width >> 2;
                float width;
                if (hawk->Energy >= World::Hawk::InitialEnergy)
                    width = (float) maxWidth;
                else
                    width = maxWidth * hawk->Energy / World::Hawk::InitialEnergy;
                float y = 10.f + pFont04->LineH();

                glColor3ub( 255, 0, 0 );
                glBegin(GL_QUADS);
                    glVertex2f(center - width, y);
                    glVertex2f(center + width, y);
                    glVertex2f(center + width, y + 10.f);
                    glVertex2f(center - width, y + 10.f);
                glEnd();
            }

            glColor3ub(255,255,255);
            const char *text = "Follow mode, press [Enter] or [Right Click] to return to player";
            float width = pFont04->Length(text);
            pFont04->Print((Width - width)*0.5f, 10.f, 0.f, text);
        }
    }

    glFlush(); //glFinish();
    return true;
}

} // namespace Scenes
