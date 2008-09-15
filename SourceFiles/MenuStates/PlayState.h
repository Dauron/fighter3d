// included in MenuStates.h

#include <fstream>

#include "../../Utils/Filesystem.h"
#include "../../World/SkeletizedObj.h"
#include "../../Graphics/OGL/WorldRenderGL.h"
#include "../../Math/Cameras/CameraHuman.h"
#include "../../Graphics/OGL/Utils.h"

#include "../SceneGame.h"
#include "../../MotionCapture/CaptureInput.h"
#include "../../Multiplayer/NetworkInput.h"

namespace Scenes { namespace Menu {
    using namespace Graphics::OGL;

    struct PlayState : public BaseState {

        std::string                MapFile;

        Math::Cameras::CameraHuman Front;

        SkeletizedObj             *choosen[2];
        std::vector<SkeletizedObj> players[2];
        int                        control[2];

        xRectangle MenuButton;
        xRectangle PlayButton;

        xDWORD widthHalf;
        xDWORD controlsY;
        xDWORD controlsH;
        xDWORD fightersY;
        xDWORD fightersH;
        xDWORD stylesY;
        xDWORD stylesH;

        HTexture backGround;
        HTexture nextBtn;
        HTexture backBtn;

        virtual void Init(BaseState *parent)
        {
            BaseState::Init(parent);
            Name = "Play";
            LoadPlayers();
            FL_enabled = players[0].size();
            MapFile.clear();
            backGround = g_TextureMgr.GetTexture("Data/textures/menu/fighters.tga");
            nextBtn = g_TextureMgr.GetTexture("Data/textures/menu/next.tga");
            backBtn = g_TextureMgr.GetTexture("Data/textures/menu/back.tga");
        }
        virtual void Invalidate()
        {
            BaseState::Invalidate();

            WorldRenderGL renderer;
            for (int i = 0; i < 2; ++i)
                for (size_t j = 0; j < players[i].size(); ++j)
                    if (players[i][j].IsCreated())
                        renderer.Invalidate(players[i][j]);
        }
        virtual void Clear()
        {
            BaseState::Clear();
            WorldRenderGL renderer;
            for (int i = 0; i < 2; ++i)
            {
                for (size_t j = 0; j < players[i].size(); ++j)
                    if (players[i][j].IsCreated())
                    {
                        renderer.Free(players[i][j]);
                        players[i][j].Destroy();
                    }
                players[i].clear();
            }
            g_TextureMgr.Release(backGround);
            g_TextureMgr.Release(nextBtn);
            g_TextureMgr.Release(backBtn);
            nextBtn = backBtn = backGround = HTexture();
        }

        void ChoosePlayer(xBYTE player, SkeletizedObj &model)
        {
            choosen[player] = &model;
            if (!model.IsCreated())
            {
                if (model.modelFile.size() && model.fastModelFile.size())
                    model.Create(model.modelFile.c_str(), model.fastModelFile.c_str());
                else
                if (model.modelFile.size())
                    model.Create(model.modelFile.c_str());
                model.ControlType = SkeletizedObj::Control_ComBoardInput;
                model.Update(0.1f);
            }
        }

        virtual void Enter()
        {
            ChoosePlayer(0, players[0].front());
            ChoosePlayer(1, players[1].back());
            control[0] = 0;
            control[1] = 1;
        }

        virtual void Exit()
        {
        }

        virtual bool Update(xFLOAT T_time)
        {
            if (!BaseState::Update(T_time))
            {
                if (g_InputMgr.InputDown_GetAndRaise(IC_LClick))
                {
                    xDWORD x = (xDWORD)g_InputMgr.mouseX, y = (xDWORD)g_InputMgr.mouseY;
                    xDWORD i = (x < widthHalf) ? 0 : 1;

                    if (controlsY < y && y < controlsY + controlsH * 3)
                    {
                        xDWORD j = (y - controlsY) / controlsH;
                        control[i] = j;
                        if (j == 2 && control[(i+1)%2] == 2)
                            control[(i+1)%2] = 1;
                    }
                    else
                    if (fightersY < y && y < fightersY + fightersH * players[0].size())
                    {
                        xDWORD j = (y - fightersY) / fightersH;
                        ChoosePlayer(i, players[i][j]);
                    }
                    else
                    if (stylesY < y && y < stylesY + stylesH * choosen[0]->styles.size())
                    {
                        xDWORD j = (y - stylesY) / stylesH;
                        choosen[i]->comBoard.Load(choosen[i]->styles[j].FileName.c_str(), choosen[i]->I_bones);
                    }
                    else
                    if (MenuButton.Contains(x,y))
                    {
                        g_InputMgr.InputDown_Set(IC_Reject, true);
                        BaseState::Update(0.f);
                        return true;
                    }
                    else
                    if (PlayButton.Contains(x,y))
                        g_InputMgr.InputDown_Set(IC_Accept, true);
                }
                if (g_InputMgr.InputDown_GetAndRaise(IC_Accept))
                {
                    SkeletizedObj *player1 = new SkeletizedObj(),
                                  *player2 = new SkeletizedObj();
                    *player1 = *choosen[0];
                    *player2 = *choosen[1];

                    player1->FL_shadowcaster = Config::EnableShadowsForPlayers;
                    player1->comBoard.ID_action_cur = player1->comBoard.StopAction.ID_action;
                    player1->Tracker.Mode      = Math::Tracking::ObjectTracker::TRACK_OBJECT;
                    //player1->Tracker.ID_object = 1;

                    player2->FL_shadowcaster = Config::EnableShadowsForPlayers;
                    player2->comBoard.ID_action_cur = player2->comBoard.StopAction.ID_action;
                    player2->Tracker.Mode      = Math::Tracking::ObjectTracker::TRACK_OBJECT;
                    //player2->Tracker.ID_object = 0;

                    SkeletizedObj::camera_controled = NULL;

                    player1->FL_auto_movement = control[0] > 0;
                    player2->FL_auto_movement = control[1] > 0;
                    if (control[0] < 2 && control[1] < 2)
                    {
                        player1->comBoard.PlayerSet = IC_CB_ComboSet1;
                        player1->ControlType = SkeletizedObj::Control_ComBoardInput;
                        player2->comBoard.PlayerSet = IC_CB_ComboSet2;
                        player2->ControlType = SkeletizedObj::Control_ComBoardInput;
                    }
                    else
                    if (control[0] == 2)
                    {
                        SkeletizedObj::camera_controled = player1;
                        player2->comBoard.PlayerSet = IC_CB_ComboSet0;
                        player2->ControlType = SkeletizedObj::Control_ComBoardInput;
                    }
                    else
                    if (control[1] == 2)
                    {
                        SkeletizedObj::camera_controled = player2;
                        player1->comBoard.PlayerSet = IC_CB_ComboSet0;
                        player1->ControlType = SkeletizedObj::Control_ComBoardInput;
                    }

                    g_CaptureInput.Finalize();
                    if (SkeletizedObj::camera_controled)
                    {
                        bool captureOK = g_CaptureInput.Initialize(SkeletizedObj::camera_controled->ModelGr_Get().xModelP->Spine);
                        SkeletizedObj::camera_controled->ControlType = (captureOK)
                            ? SkeletizedObj::Control_CaptureInput
                            : SkeletizedObj::Control_ComBoardInput;
                    }

                    *choosen[0] = SkeletizedObj();
                    *choosen[1] = SkeletizedObj();

                    player1->comBoard.FL_mirror = false;
                    player2->comBoard.FL_mirror = false;

                    SceneGame *scene = new SceneGame(SkeletizedObj::camera_controled ? "[Game]" : "[Game_Set2]");
                    scene->Player1 = player1;
                    scene->Player2 = player2;
                    scene->MapFileName = MapFile;

                    // reload players
                    this->Clear();
                    SwitchState(*Parent);
                    this->Init(Parent);

                    g_Application.Scene_Set(*scene, false);

                    return true;
                }

                for(int i = 0; i < 2; ++i)
                {
                    SkeletizedObj &player = *choosen[i];
                    if (player.comBoard.ID_action_cur == player.comBoard.StopAction.ID_action)
                    {
                        player.comBoard.ID_action_cur = (xBYTE) (rand() % player.comBoard.L_actions.size());
                        player.comBoard.T_progress = 0.f;
                    }
                    player.Update(T_time*0.5f);
                    player.MX_LocalToWorld_Set().row3.init(0,0,0,1);
                }
            }
            return true;
        }

        virtual void Render(const Graphics::OGL::Font* pFont03, const Graphics::OGL::Font* pFont04,
                            xDWORD Width, xDWORD Height)
        {
            glColor4f( 1.0f, 1.0f, 1.0f, 1.f );
            glEnable (GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_TEXTURE_2D);

            xFLOAT scale   = Height * 0.7f / 1024.f;

            if (!backGround.IsNull())
            {
                xFLOAT iWidth  = scale * g_TextureMgr.GetWidth(backGround);
                xFLOAT iHeight = scale * g_TextureMgr.GetHeight(backGround);
                xFLOAT left = Width  - iWidth  - 450.f*scale;
                xFLOAT top  = Height - iHeight - 480.f*scale;

                g_TextureMgr.BindTexture(backGround);
                glBegin(GL_QUADS);
                {
                    glTexCoord2f(0.01f,0.99f);
                    glVertex2f(left, top);
                    glTexCoord2f(0.99f,0.99f);
                    glVertex2f(left+iWidth, top);
                    glTexCoord2f(0.99f,0.01f);
                    glVertex2f(left+iWidth, top+iHeight);
                    glTexCoord2f(0.01f,0.01f);
                    glVertex2f(left, top+iHeight);
                }
                glEnd();
            }

            if (!nextBtn.IsNull())
            {
                PlayButton.W = 2.f * scale * g_TextureMgr.GetWidth(nextBtn);
                PlayButton.X = Width - PlayButton.W - 100.f * scale;
                PlayButton.H = 2.f * scale * g_TextureMgr.GetHeight(nextBtn);
                PlayButton.Y = Height - PlayButton.H - 50.f * scale;
                g_TextureMgr.BindTexture(nextBtn);
                glBegin(GL_QUADS);
                {
                    glTexCoord2f(0.01f,0.99f);
                    glVertex2f(PlayButton.X, PlayButton.Y);
                    glTexCoord2f(0.99f,0.99f);
                    glVertex2f(PlayButton.X+PlayButton.W, PlayButton.Y);
                    glTexCoord2f(0.99f,0.01f);
                    glVertex2f(PlayButton.X+PlayButton.W, PlayButton.Y+PlayButton.H);
                    glTexCoord2f(0.01f,0.01f);
                    glVertex2f(PlayButton.X, PlayButton.Y+PlayButton.H);
                }
                glEnd();
            }

            if (!backBtn.IsNull())
            {
                MenuButton.W = 2.f * scale * g_TextureMgr.GetWidth(backBtn);
                MenuButton.X = 100.f * scale;
                MenuButton.H = 2.f * scale * g_TextureMgr.GetHeight(backBtn);
                MenuButton.Y = Height - MenuButton.H - 50.f * scale;
                g_TextureMgr.BindTexture(backBtn);
                glBegin(GL_QUADS);
                {
                    glTexCoord2f(0.01f,0.99f);
                    glVertex2f(MenuButton.X, MenuButton.Y);
                    glTexCoord2f(0.99f,0.99f);
                    glVertex2f(MenuButton.X+MenuButton.W, MenuButton.Y);
                    glTexCoord2f(0.99f,0.01f);
                    glVertex2f(MenuButton.X+MenuButton.W, MenuButton.Y+MenuButton.H);
                    glTexCoord2f(0.01f,0.01f);
                    glVertex2f(MenuButton.X, MenuButton.Y+MenuButton.H);
                }
                glEnd();
            }

            glDisable(GL_TEXTURE_2D);
            glDisable(GL_BLEND);

            xFLOAT HeightHalf  = Height * 0.5f;
            xFLOAT WidthFourth = Width * 0.25f;
            widthHalf = (xDWORD)(Width * 0.5f);

            xFLOAT lineHeight03 = pFont03->LineH();
            xFLOAT lineHeight04 = pFont04->LineH();
            xFLOAT HeadersHeight = lineHeight04*2.f;

            controlsH = fightersH = stylesH = (xDWORD)lineHeight04;

            ////// Fighter demo

            glMatrixMode(GL_PROJECTION);
            glPushMatrix();
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();

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
            glClearDepth( 100.0f ); // Draw distance
            glClear(GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);

            glPolygonMode(GL_FRONT_AND_BACK, Config::PolygonMode);

            Front.Init(10.f, -10.0f, choosen[0]->P_center_Trfm.z, 9.0f, -9.0f, choosen[0]->P_center_Trfm.z, 0.0f, 0.0f, 1.0f);
            Front.FOV.InitViewport(0,(xDWORD)HeightHalf,(xDWORD)WidthFourth*3,(xDWORD)(HeightHalf-HeadersHeight));
            Front.FOV.InitOrthogonal();
            Front.Update(0.f);
            ViewportSet_GL(Front);

            GLfloat light_global_amb_color[]  = { 0.2f, 0.2f, 0.2f, 1.0f };
            glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_global_amb_color);
            GLfloat light_position[]   = { 10.0f, -10.0f, 10.0f, 0.0f };
            GLfloat light_amb_color[]  = { 0.1f, 0.1f, 0.1f, 1.0f };
            GLfloat light_dif_color[]  = { 0.9f, 0.9f, 0.9f, 1.0f };
            GLfloat light_spec_color[] = { 0.9f, 0.9f, 0.9f, 1.0f };
            glLightfv(GL_LIGHT0, GL_POSITION, light_position);
            glLightfv(GL_LIGHT0, GL_AMBIENT,  light_amb_color);  // environment
            glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_dif_color);  // direct light
            glLightfv(GL_LIGHT0, GL_SPECULAR, light_spec_color); // light on mirrors/metal
            glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION,  1.0f);
            glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.f);
            glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.f);
            glEnable(GL_LIGHT0);
            Shader::SetLightType(xLight_INFINITE);
            Shader::EnableTexturing(xState_Enable);

            WorldRenderGL wRender;
            RendererGL   &render = wRender.renderModel;

            wRender.FreeIfNeeded(*choosen[0]);
            choosen[0]->Render();
            wRender.FreeIfNeeded(*choosen[1]);
            choosen[1]->Render();

            render.RenderModel(*choosen[0]->ModelGr->xModelP, choosen[0]->ModelGr->instance, false, Front.FOV);
            render.RenderModel(*choosen[0]->ModelGr->xModelP, choosen[0]->ModelGr->instance, true,  Front.FOV);

            glViewport((xDWORD)WidthFourth,(xDWORD)HeightHalf,(xDWORD)WidthFourth*3,(xDWORD)(HeightHalf-HeadersHeight));

            render.RenderModel(*choosen[1]->ModelGr->xModelP, choosen[1]->ModelGr->instance, false, Front.FOV);
            render.RenderModel(*choosen[1]->ModelGr->xModelP, choosen[1]->ModelGr->instance, true,  Front.FOV);

            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);

            Shader::Suspend();
            Shader::EnableTexturing(xState_Disable);
            Shader::SetLightType(xLight_NONE);

            ////// Overlay

            glMatrixMode(GL_PROJECTION);
            glPopMatrix();
            glMatrixMode(GL_MODELVIEW);
            glPopMatrix();

            // Set projection
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(0, WidthFourth, Height, 0, 0, 100);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            int i;
            xFLOAT y, textLen;

            ////// Player config 1
            i = 0;
            glViewport(0,0,(xDWORD)WidthFourth,Height);
            // fix raster pos after viewport change, on some broken GPUs
            glBegin(GL_POINTS);
            glVertex2i(-1,-1);
            glEnd();

            glColor4f( 0.0f, 0.0f, 0.0f, 1.f );

            ////// Control mode
            y = HeadersHeight + lineHeight04;
            pFont04->Print(WidthFourth * 0.1f, y, 0.0f, "Controls:");
            controlsY = (xDWORD)y;
            y += lineHeight04;
            if (control[i] == 0)
                glColor3ub(222, 173, 192);
            else
                glColor3ub(0, 0, 0);
            pFont03->Print(WidthFourth * 0.1f, y, 0.0f, "ComBoard");
            y += lineHeight03;
            if (control[i] == 1)
                glColor3ub(222, 173, 192);
            else
                glColor3ub(0, 0, 0);
            pFont03->Print(WidthFourth * 0.1f, y, 0.0f, "ComBoard (auto move)");
            y += lineHeight03;
            if (control[i] == 2)
                glColor3ub(222, 173, 192);
            else
                glColor3ub(0, 0, 0);
            pFont03->Print(WidthFourth * 0.1f, y, 0.0f, "Camera (auto move)");

            ////// Fighters
            glColor4f( 0.0f, 0.0f, 0.0f, 1.f );
            y += lineHeight04*2;
            pFont04->Print(WidthFourth * 0.1f, y, 0.0f, "Fighters:");
            fightersY = (xDWORD)y;
            y += lineHeight04;
            for (size_t j = 0; j < players[i].size(); ++j)
            {
                if (choosen[i] == &players[i][j])
                    glColor3ub(222, 173, 192);
                else
                    glColor3ub(0, 0, 0);
                const char *name = players[i][j].Name.c_str();
                pFont03->Print(WidthFourth * 0.1f, y, 0.0f, name);
                y += lineHeight03;
            }

            ////// Fighting styles
            glColor4f( 0.0f, 0.0f, 0.0f, 1.f );
            y += lineHeight04;
            pFont04->Print(WidthFourth * 0.1f, y, 0.0f, "Fighting styles:");
            stylesY = (xDWORD)y;
            y += lineHeight04;
            for (size_t j = 0; j < choosen[i]->styles.size(); ++j)
            {
                if (choosen[i]->styles[j].FileName == choosen[i]->comBoard.FileName)
                    glColor3ub(222, 173, 192);
                else
                    glColor3ub(0, 0, 0);
                const char *name = choosen[i]->styles[j].Name.c_str();
                pFont03->Print(WidthFourth * 0.1f, y, 0.0f, name);
                y += lineHeight03;
            }

            ////// Player config 2
            i = 1;
            glViewport(Width-(xDWORD)WidthFourth,0,(xDWORD)WidthFourth,Height);
            // fix raster pos after viewport change, on some broken GPUs
            glBegin(GL_POINTS);
            glVertex2i(-1,-1);
            glEnd();

            glColor4f( 0.0f, 0.0f, 0.0f, 1.f );

            ////// Control mode
            y = HeadersHeight + lineHeight04;
            textLen = pFont04->Length("Controls:");
            pFont04->Print(WidthFourth - textLen - WidthFourth * 0.1f, y, 0.0f, "Controls:");
            controlsY = (xDWORD)y;
            y += lineHeight04;
            if (control[i] == 0)
                glColor3ub(222, 173, 192);
            else
                glColor3ub(0, 0, 0);
            textLen = pFont03->Length("ComBoard");
            pFont03->Print(WidthFourth - textLen - WidthFourth * 0.1f, y, 0.0f, "ComBoard");
            y += lineHeight03;
            if (control[i] == 1)
                glColor3ub(222, 173, 192);
            else
                glColor3ub(0, 0, 0);
            textLen = pFont03->Length("ComBoard (auto move)");
            pFont03->Print(WidthFourth - textLen - WidthFourth * 0.1f, y, 0.0f, "ComBoard (auto move)");
            y += lineHeight03;
            if (control[i] == 2)
                glColor3ub(222, 173, 192);
            else
                glColor3ub(0, 0, 0);
            textLen = pFont03->Length("Camera (auto move)");
            pFont03->Print(WidthFourth - textLen - WidthFourth * 0.1f, y, 0.0f, "Camera (auto move)");

            ////// Fighters
            glColor4f( 0.0f, 0.0f, 0.0f, 1.f );
            y += lineHeight04*2;
            textLen = pFont04->Length("Fighters:");
            pFont04->Print(WidthFourth - textLen - WidthFourth * 0.1f, y, 0.0f, "Fighters:");
            fightersY = (xDWORD)y;
            y += lineHeight04;
            for (size_t j = 0; j < players[i].size(); ++j)
            {
                if (choosen[i] == &players[i][j])
                    glColor3ub(222, 173, 192);
                else
                    glColor3ub(0, 0, 0);
                const char *name = players[i][j].Name.c_str();
                textLen = pFont03->Length(name);
                pFont03->Print(WidthFourth - textLen - WidthFourth * 0.1f, y, 0.0f, name);
                y += lineHeight03;
            }

            ////// Fighting styles
            glColor4f( 0.0f, 0.0f, 0.0f, 1.f );
            y += lineHeight04;
            textLen = pFont04->Length("Fighting styles:");
            pFont04->Print(WidthFourth - textLen - WidthFourth * 0.1f, y, 0.0f, "Fighting styles:");
            stylesY = (xDWORD)y;
            y += lineHeight04;
            for (size_t j = 0; j < choosen[i]->styles.size(); ++j)
            {
                if (choosen[i]->styles[j].FileName == choosen[i]->comBoard.FileName)
                    glColor3ub(222, 173, 192);
                else
                    glColor3ub(0, 0, 0);
                const char *name = choosen[i]->styles[j].Name.c_str();
                textLen = pFont03->Length(name);
                pFont03->Print(WidthFourth - textLen - WidthFourth * 0.1f, y, 0.0f, name);
                y += lineHeight03;
            }
        }

        void LoadPlayers()
        {
            players[0].clear();
            players[1].clear();

            std::ifstream in;

            const char *fileName = "Data/players.txt";

            in.open(Filesystem::GetFullPath(fileName).c_str());
            if (in.is_open())
            {
                std::string dir = Filesystem::GetParentDir(fileName);
                char buffer[255];
                int  len;

                enum LoadMode
                {
                    LoadMode_None,
                    LoadMode_Player
                } mode = LoadMode_None;

                SkeletizedObj *player1 = NULL;
                SkeletizedObj *player2 = NULL;

                while (in.good())
                {
                    in.getline(buffer, 255);
                    if (buffer[0] == 0 || buffer[0] == '#') continue;
                    len = strlen(buffer);
                    if (buffer[len - 1] == '\r') buffer[len - 1] = 0;

                    if (buffer[0] == '[')
                    {
                        if (StartsWith(buffer, "[player]"))
                        {
                            mode = LoadMode_Player;
                            players[0].push_back(SkeletizedObj());
                            players[1].push_back(SkeletizedObj());
                            player1 = &players[0].back();
                            player2 = &players[1].back();
                            player1->ApplyDefaults();
                            player2->ApplyDefaults();
                            continue;
                        }
                        mode = LoadMode_None;
                    }
                    if (mode == LoadMode_Player)
                    {
                        player1->LoadLine(buffer, dir);
                        player2->LoadLine(buffer, dir);
                    }
                }
            }
        }
    };

} } // namespace Scenes::Menu
