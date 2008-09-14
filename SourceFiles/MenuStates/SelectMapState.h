// included in MenuStates.h

#include <fstream>

#include "../../Graphics/Textures/TextureMgr.h"
#include "../../Utils/Filesystem.h"

#include "PlayState.h"

namespace Scenes { namespace Menu {

    struct SelectMapState : public BaseState {

        struct Map
        {
            std::string Name;
            std::string File;
            std::string Image;

            HTexture    Texture;
        };

        std::vector<Map> L_maps;

        xBYTE selected;

        xRectangle BackButton;
        xRectangle NextButton;

        xDWORD widthHalf;
        xDWORD mapsY;
        xDWORD mapsH;

        HTexture backGround;
        HTexture nextBtn;
        HTexture backBtn;

        virtual void Init(BaseState *parent)
        {
            BaseState::Init(parent);
            Name = "Play";
            LoadMaps();
            selected = 0;
            FL_enabled = L_maps.size();

            backGround = g_TextureMgr.GetTexture("Data/textures/menu/map.tga");
            nextBtn = g_TextureMgr.GetTexture("Data/textures/menu/next.tga");
            backBtn = g_TextureMgr.GetTexture("Data/textures/menu/back.tga");
        }

        virtual void Clear()
        {
            BaseState::Clear();

            for (size_t i = 0; i < L_maps.size(); ++i)
                if (!L_maps[i].Texture.IsNull())
                {
                    g_TextureMgr.Release(L_maps[i].Texture);
                    L_maps[i].Texture = HTexture();
                }
            L_maps.clear();
            g_TextureMgr.Release(backGround);
            nextBtn = backBtn = backGround = HTexture();
        }

        virtual bool Update(xFLOAT T_time)
        {
            if (!BaseState::Update(T_time))
            {
                if (g_InputMgr.InputDown_GetAndRaise(IC_LClick))
                {
                    xDWORD x = (xDWORD)g_InputMgr.mouseX, y = (xDWORD)g_InputMgr.mouseY;

                    if (mapsY < y && y < mapsY + mapsH * L_maps.size())
                        selected = (y - mapsY) / mapsH;
                    else
                    if (BackButton.Contains(x,y))
                    {
                        g_InputMgr.InputDown_Set(IC_Reject, true);
                        BaseState::Update(0.f);
                        return true;
                    }
                    else
                    if (NextButton.Contains(x,y))
                        g_InputMgr.InputDown_Set(IC_Accept, true);
                }
                if (g_InputMgr.InputDown_GetAndRaise(IC_Accept))
                {
                    ((PlayState*)SubStates[0])->MapFile = L_maps[selected].File;
                    SwitchState(*SubStates[0]);
                    return true;
                }
                if (g_InputMgr.InputDown_GetAndRaise(IC_MoveUp))
                {
                    --selected;
                    if (selected == xBYTE_MAX) selected = L_maps.size()-1;
                    return true;
                }
                if (g_InputMgr.InputDown_GetAndRaise(IC_MoveDown))
                {
                    ++selected;
                    if (selected >= L_maps.size()) selected = 0;
                    return true;
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
                xFLOAT left = Width  - iWidth  - 170.f*scale;
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
                NextButton.W = 2.f * scale * g_TextureMgr.GetWidth(nextBtn);
                NextButton.X = Width - NextButton.W - 100.f * scale;
                NextButton.H = 2.f * scale * g_TextureMgr.GetHeight(nextBtn);
                NextButton.Y = Height - NextButton.H - 50.f * scale;
                g_TextureMgr.BindTexture(nextBtn);
                glBegin(GL_QUADS);
                {
                    glTexCoord2f(0.01f,0.99f);
                    glVertex2f(NextButton.X, NextButton.Y);
                    glTexCoord2f(0.99f,0.99f);
                    glVertex2f(NextButton.X+NextButton.W, NextButton.Y);
                    glTexCoord2f(0.99f,0.01f);
                    glVertex2f(NextButton.X+NextButton.W, NextButton.Y+NextButton.H);
                    glTexCoord2f(0.01f,0.01f);
                    glVertex2f(NextButton.X, NextButton.Y+NextButton.H);
                }
                glEnd();
            }

            if (!backBtn.IsNull())
            {
                BackButton.W = 2.f * scale * g_TextureMgr.GetWidth(backBtn);
                BackButton.X = 100.f * scale;
                BackButton.H = 2.f * scale * g_TextureMgr.GetHeight(backBtn);
                BackButton.Y = Height - BackButton.H - 50.f * scale;
                g_TextureMgr.BindTexture(backBtn);
                glBegin(GL_QUADS);
                {
                    glTexCoord2f(0.01f,0.99f);
                    glVertex2f(BackButton.X, BackButton.Y);
                    glTexCoord2f(0.99f,0.99f);
                    glVertex2f(BackButton.X+BackButton.W, BackButton.Y);
                    glTexCoord2f(0.99f,0.01f);
                    glVertex2f(BackButton.X+BackButton.W, BackButton.Y+BackButton.H);
                    glTexCoord2f(0.01f,0.01f);
                    glVertex2f(BackButton.X, BackButton.Y+BackButton.H);
                }
                glEnd();
            }

            widthHalf = Width / 2;
            xFLOAT lineHeight03 = pFont03->LineH();
            xFLOAT lineHeight04 = pFont04->LineH();
            xFLOAT HeadersHeight = lineHeight04*2.f;

            if (L_maps[selected].Image.size() && !L_maps[selected].Texture.IsNull())
            {
                g_TextureMgr.BindTexture(L_maps[selected].Texture);
                xFLOAT W = (xFLOAT) g_TextureMgr.GetWidth(L_maps[selected].Texture);
                xFLOAT H = (xFLOAT) g_TextureMgr.GetHeight(L_maps[selected].Texture);

                if (W > widthHalf)
                {
                    H *= widthHalf / W;
                    W = (xFLOAT)widthHalf;
                }
                xFLOAT maxH = (Height - HeadersHeight) * 0.5f;
                if (H > maxH)
                {
                    W *= maxH / H;
                    H = maxH;
                }

                xFLOAT x = Width - 50.f * scale - W;
                xFLOAT y = HeadersHeight * 0.5f;

                glBegin(GL_QUADS);
                {
                    glTexCoord2f(0.01f,0.99f);
                    glVertex2f(x, y);
                    glTexCoord2f(0.99f,0.99f);
                    glVertex2f(x+W, y);
                    glTexCoord2f(0.99f,0.01f);
                    glVertex2f(x+W, y+H);
                    glTexCoord2f(0.01f,0.01f);
                    glVertex2f(x, y+H);
                }
                glEnd();
            }

            glDisable(GL_TEXTURE_2D);
            glDisable(GL_BLEND);

            glColor4f( 0.0f, 0.0f, 0.0f, 1.f );
            xFLOAT y = HeadersHeight + lineHeight04;
            mapsY = (xDWORD)(y + lineHeight03);
            mapsH = (xDWORD)lineHeight03;
            ////// Maps
            pFont04->Print(widthHalf * 0.1f, y, 0.0f, "Maps:");
            y += lineHeight04;

            for(size_t i = 0; i < L_maps.size(); ++i)
            {
                if (selected == i)
                    glColor3ub(222, 173, 192);
                else
                    glColor3ub(0, 0, 0);
                const char *name = L_maps[i].Name.c_str();
                pFont03->Print(widthHalf * 0.1f, y, 0.0f, name);
                y += lineHeight03;
            }
        }

        void LoadMaps()
        {
            std::ifstream in;

            const char *fileName = "Data/maps.txt";

            in.open(Filesystem::GetFullPath(fileName).c_str());
            if (in.is_open())
            {
                std::string dir = Filesystem::GetParentDir(fileName);
                char buffer[255];
                int  len;

                enum LoadMode
                {
                    LoadMode_None,
                    LoadMode_Map
                } mode = LoadMode_None;

                Map *map = NULL;

                while (in.good())
                {
                    in.getline(buffer, 255);
                    if (buffer[0] == 0 || buffer[0] == '#') continue;
                    len = strlen(buffer);
                    if (buffer[len - 1] == '\r') buffer[len - 1] = 0;

                    if (buffer[0] == '[')
                    {
                        if (StartsWith(buffer, "[map]"))
                        {
                            mode = LoadMode_Map;
                            L_maps.push_back(Map());
                            map = &L_maps.back();
                            continue;
                        }
                        mode = LoadMode_None;
                    }
                    if (mode == LoadMode_Map)
                    {
                        if (StartsWith(buffer, "name"))
                        {
                            map->Name = ReadSubstring(buffer+4);
                            continue;
                        }
                        if (StartsWith(buffer, "file"))
                        {
                            map->File = "Data/";
                            map->File = Filesystem::GetFullPath( map->File + ReadSubstring(buffer+4) );
                            continue;
                        }
                        if (StartsWith(buffer, "img"))
                        {
                            map->Image = "Data/";
                            map->Image = Filesystem::GetFullPath( map->Image + ReadSubstring(buffer+3) );
                            map->Texture = g_TextureMgr.GetTexture(map->Image.c_str());
                            continue;
                        }
                    }
                }
            }
        }
    };

} } // namespace Scenes::Menu
