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

    struct PlayState : public BaseState {
        
        std::string                MapFile;

        Math::Cameras::CameraHuman Front;

        SkeletizedObj             *choosen[2];
        std::vector<SkeletizedObj> players[2];

        xRectangle MenuButton;
        xRectangle PlayButton;

        xDWORD widthHalf;
        xDWORD fightersY;
        xDWORD fightersH;
        xDWORD stylesY;
        xDWORD stylesH;

        virtual void Init(BaseState *parent)
        {
            BaseState::Init(parent);
            Name = "Play";
            LoadPlayers();
            FL_enabled = players[0].size();
            MapFile.clear();
        }
        virtual void Invalidate()
        {
            BaseState::Invalidate();

            WorldRenderGL renderer;
            for (int i = 0; i < 2; ++i)
                for (size_t j = 0; j < players[i].size(); ++j)
                    if (players[i][j].IsInitialized())
                        renderer.Invalidate(players[i][j]);
        }
        virtual void Clear()
        {
            BaseState::Clear();
            WorldRenderGL renderer;
            for (int i = 0; i < 2; ++i)
            {
                for (size_t j = 0; j < players[i].size(); ++j)
                    if (players[i][j].IsInitialized())
                    {
                        renderer.Free(players[i][j]);
                        players[i][j].Finalize();
                    }
                players[i].clear();
            }
        }
        
        void ChoosePlayer(xBYTE player, SkeletizedObj &model)
        {
            choosen[player] = &model;
            if (!model.IsInitialized())
            {
                if (model.modelFile.size() && model.fastModelFile.size())
                    model.Initialize(model.modelFile.c_str(), model.fastModelFile.c_str());
                else
                if (model.modelFile.size())
                    model.Initialize(model.modelFile.c_str());
                model.ControlType = SkeletizedObj::Control_ComBoardInput;
                model.FrameUpdate(0.1f);
            }
        }
        
        virtual void Enter()
        {
            ChoosePlayer(0, *players[0].begin());
            ChoosePlayer(1, *players[1].rbegin());
        }

        virtual void Exit()
        {
        }
        
        virtual bool Update(xFLOAT T_time)
        {
            if (!BaseState::Update(T_time))
            {
                if (g_InputMgr.GetInputStateAndClear(IC_LClick))
                {
                    xDWORD x = (xDWORD)g_InputMgr.mouseX, y = (xDWORD)g_InputMgr.mouseY;
                    xDWORD i = (x < widthHalf) ? 0 : 1;

                    if (fightersY < y && y < fightersY + fightersH * players[0].size())
                    {
                        xDWORD j = (y - fightersY) / fightersH;
                        ChoosePlayer(i, players[i][j]);
                    }
                    else
                    if (stylesY < y && y < stylesY + stylesH * choosen[0]->styles.size())
                    {
                        xDWORD j = (y - stylesY) / stylesH;
                        choosen[i]->comBoard.Load(choosen[i]->styles[j].FileName.c_str());
                    }
                    else
                    if (MenuButton.Contains(x,y))
                    {
                        g_InputMgr.SetInputState(IC_Reject, true);
                        BaseState::Update(0.f);
                        return true;
                    }
                    else
                    if (PlayButton.Contains(x,y))
                        g_InputMgr.SetInputState(IC_Accept, true);
                }
                if (g_InputMgr.GetInputStateAndClear(IC_Accept))
                {
                    SkeletizedObj *player1 = new SkeletizedObj(),
                                  *player2 = new SkeletizedObj();
                    *player1 = *choosen[0];
                    *player2 = *choosen[1];
                    player1->comBoard.ID_action_cur = player1->comBoard.StopAction.ID_action;
                    player1->FL_auto_movement = false;
                    player1->Tracker.Mode      = Math::Tracking::ObjectTracker::TRACK_OBJECT;
                    player1->Tracker.ID_object = 1;

                    player2->comBoard.ID_action_cur = player2->comBoard.StopAction.ID_action;
                    player2->FL_auto_movement = true;
                    player2->Tracker.Mode      = Math::Tracking::ObjectTracker::TRACK_OBJECT;
                    player2->Tracker.ID_object = 0;
                    
                    SkeletizedObj::camera_controled = player2;
                    g_CaptureInput.Finalize();
                    bool captureOK = g_CaptureInput.Initialize(SkeletizedObj::camera_controled->ModelGr_Get().xModelP->Spine);
                    SkeletizedObj::camera_controled->ControlType = (captureOK)
                        ? SkeletizedObj::Control_CaptureInput
                        : SkeletizedObj::Control_ComBoardInput;
                    
                    *choosen[0] = SkeletizedObj();
                    *choosen[1] = SkeletizedObj();
                    std::string map = MapFile;

                    // reload players
                    this->Clear();
                    SwitchState(*Parent);
                    this->Init(Parent);

                    g_Application.SetCurrentScene(new SceneGame(player1,player2,map), false);

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
                    player.FrameUpdate(T_time*0.5f);
                    player.MX_LocalToWorld_Set().row3.init(0,0,0,1);
                }
            }
            return true;
        }
        
        virtual void Render(const GLFont* pFont03, const GLFont* pFont04,
                            const GLFont* pFont05, const GLFont* pFont10,
                            xDWORD Width, xDWORD Height)
        {
            xDWORD HeightHalf = (xDWORD)(Height * 0.5f);
            xDWORD WidthHalf  = (xDWORD)(Width * 0.5f);

            xFLOAT lineHeight03 = pFont03->LineH();
            xFLOAT lineHeight05 = pFont05->LineH();
            xFLOAT HeadersHeight = lineHeight05*2.f;

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
            Front.FOV.InitViewport(0,HeightHalf,WidthHalf,HeightHalf-(xDWORD)HeadersHeight);
            Front.FOV.InitOrthogonal();
            Front.Update(0.f);
            ViewportSet_GL(Front);

            // lights at viewer position
            GLfloat light_global_amb_color[]  = { 0.2f, 0.2f, 0.2f, 1.0f };
            glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_global_amb_color);
            GLfloat light_position[]   = { -10.0f, -10.0f, 10.0f, 0.0f };
            GLfloat light_amb_color[]  = { 0.8f, 0.8f, 1.f, 1.f };
            GLfloat light_dif_color[]  = { 0.8f, 0.8f, 1.f, 1.f };
            GLfloat light_spec_color[] = { 0.8f, 0.8f, 1.f, 1.f };
            glLightfv(GL_LIGHT0, GL_POSITION, light_position);
            glLightfv(GL_LIGHT0, GL_AMBIENT,  light_amb_color);  // environment
            glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_dif_color);  // direct light
            glLightfv(GL_LIGHT0, GL_SPECULAR, light_spec_color); // light on mirrors/metal
            glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION,  1.0f);
            glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.f);
            glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.f);
            glEnable(GL_LIGHT0);
            GLShader::SetLightType(xLight_INFINITE);
            GLShader::EnableTexturing(xState_Enable);

            WorldRenderGL wRender;
            RendererGL   &render = wRender.renderModel;

            if (choosen[0]->FL_renderNeedsUpdate) wRender.Free(*choosen[0]);
            choosen[0]->FrameRender();
            if (choosen[1]->FL_renderNeedsUpdate) wRender.Free(*choosen[1]);
            choosen[1]->FrameRender();

            render.RenderModel(*choosen[0]->ModelGr->xModelP, choosen[0]->ModelGr->instance, false, Front.FOV);
            render.RenderModel(*choosen[0]->ModelGr->xModelP, choosen[0]->ModelGr->instance, true,  Front.FOV);

            glViewport(WidthHalf,HeightHalf,WidthHalf,HeightHalf-(xDWORD)HeadersHeight);
            
            render.RenderModel(*choosen[1]->ModelGr->xModelP, choosen[1]->ModelGr->instance, false, Front.FOV);
            render.RenderModel(*choosen[1]->ModelGr->xModelP, choosen[1]->ModelGr->instance, true,  Front.FOV);

            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);

            GLShader::Suspend();
            GLShader::EnableTexturing(xState_Disable);
            GLShader::SetLightType(xLight_NONE);

            ////// Overlay

            glMatrixMode(GL_PROJECTION);
            glPopMatrix();
            glMatrixMode(GL_MODELVIEW);
            glPopMatrix();

            widthHalf = WidthHalf;
            fightersH = stylesH = (xDWORD)lineHeight03;

            ////// Header & footer
            glViewport(0,0,Width,Height);
            glColor4f( 1.0f, 1.0f, 1.0f, 1.f );

            glBegin(GL_LINES);
            {
                glVertex2f((xFLOAT)WidthHalf, HeadersHeight);
                glVertex2f((xFLOAT)WidthHalf, Height-HeadersHeight);

                glVertex2f(0.f,           HeadersHeight);
                glVertex2f((xFLOAT)Width, HeadersHeight);

                glVertex2f(0.f,           Height-HeadersHeight);
                glVertex2f((xFLOAT)Width, Height-HeadersHeight);
            }
            glEnd();

            const char* title = "Select fighters";
            xFLOAT textLen = pFont05->Length(title);
            pFont05->PrintF((Width - textLen) * 0.5f, lineHeight05*1.25f, 0.0f, title);

            const char* menu = "Back";
            MenuButton = xRectangle(20.f, Height-lineHeight05*1.75f, pFont05->Length(menu), lineHeight05);
            if (MenuButton.Contains(g_InputMgr.mouseX, g_InputMgr.mouseY))
                glColor4f( 1.0f, 1.0f, 0.0f, 1.f );
            else
                glColor4f( 1.0f, 1.0f, 1.0f, 1.f );
            pFont05->PrintF(MenuButton.X, Height-lineHeight05*0.75f, 0.0f, menu);

            const char* play = "Play";
            textLen = pFont05->Length(play);
            PlayButton = xRectangle(Width - 20.f - textLen, Height-lineHeight05*1.75f, textLen, lineHeight05);
            if (PlayButton.Contains(g_InputMgr.mouseX, g_InputMgr.mouseY))
                glColor4f( 1.0f, 1.0f, 0.0f, 1.f );
            else
                glColor4f( 1.0f, 1.0f, 1.0f, 1.f );
            pFont05->PrintF(PlayButton.X, Height-lineHeight05*0.75f, 0.0f, play);

            ////// Player config
            for(int i = 0; i < 2; ++i)
            {
                glViewport(i*WidthHalf ,0,WidthHalf,Height);

                glColor4f( 1.0f, 1.0f, 1.0f, 1.f );
                
                ////// Control mode
                xFLOAT y = HeightHalf + lineHeight03;
                const char* controlType = i == 0 ? "Controls: ComBoard" : "Controls: Camera";
                xFLOAT textLen = pFont03->Length(controlType);
                pFont03->PrintF(WidthHalf * 0.1f, y, 0.0f, controlType);
                y += lineHeight03*2;

                ////// Fighters
                textLen = pFont03->Length("Fighters:");
                pFont03->PrintF(WidthHalf * 0.1f, y, 0.0f, "Fighters:");
                fightersY = (xDWORD)y;
                y += lineHeight03;
                for (size_t j = 0; j < players[i].size(); ++j)
                {
                    if (choosen[i] == &players[i][j])
                        glColor4f( 1.0f, 1.0f, 0.0f, 1.f );
                    else
                        glColor4f( 1.0f, 1.0f, 1.0f, 1.f );
                    const char *name = players[i][j].Name.c_str();
                    textLen = pFont03->Length(name);
                    pFont03->PrintF((WidthHalf - textLen) * 0.5f, y, 0.0f, name);
                    y += lineHeight03;
                }
                y += lineHeight03;

                ////// Fighting styles
                glColor4f( 1.0f, 1.0f, 1.0f, 1.f );
                textLen = pFont03->Length("Fighting styles:");
                pFont03->PrintF(WidthHalf * 0.1f, y, 0.0f, "Fighting styles:");
                stylesY = (xDWORD)y;
                y += lineHeight03;
                for (size_t j = 0; j < choosen[i]->styles.size(); ++j)
                {
                    if (choosen[i]->styles[j].FileName == choosen[i]->comBoard.FileName)
                        glColor4f( 1.0f, 1.0f, 0.0f, 1.f );
                    else
                        glColor4f( 1.0f, 1.0f, 1.0f, 1.f );
                    const char *name = choosen[i]->styles[j].Name.c_str();
                    textLen = pFont03->Length(name);
                    pFont03->PrintF((WidthHalf - textLen) * 0.5f, y, 0.0f, name);
                    y += lineHeight03;
                }
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
                            player1 = &*players[0].rbegin();
                            player2 = &*players[1].rbegin();
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
