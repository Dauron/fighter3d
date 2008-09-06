// included in MenuStates.h

#include <fstream>

#include "../../Graphics/OGL/Textures/TextureMgr.h"
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

        virtual void Init(BaseState *parent)
        {
            BaseState::Init(parent);
            Name = "Play";
            LoadMaps();
            selected = 0;
            FL_enabled = L_maps.size();
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
        }

        virtual bool Update(xFLOAT T_time)
        {
            if (!BaseState::Update(T_time))
            {
                if (g_InputMgr.GetInputStateAndClear(IC_LClick))
                {
                    xDWORD x = (xDWORD)g_InputMgr.mouseX, y = (xDWORD)g_InputMgr.mouseY;

                    if (mapsY < y && y < mapsY + mapsH * L_maps.size())
                        selected = (y - mapsY) / mapsH;
                    else
                    if (BackButton.Contains(x,y))
                    {
                        g_InputMgr.SetInputState(IC_Reject, true);
                        BaseState::Update(0.f);
                        return true;
                    }
                    else
                    if (NextButton.Contains(x,y))
                        g_InputMgr.SetInputState(IC_Accept, true);
                }
                if (g_InputMgr.GetInputStateAndClear(IC_Accept))
                {
                    ((PlayState*)SubStates[0])->MapFile = L_maps[selected].File;
                    SwitchState(*SubStates[0]);
                    return true;
                }
                if (g_InputMgr.GetInputStateAndClear(IC_MoveUp))
                {
                    --selected;
                    if (selected == xBYTE_MAX) selected = L_maps.size()-1;
                    return true;
                }
                if (g_InputMgr.GetInputStateAndClear(IC_MoveDown))
                {
                    ++selected;
                    if (selected >= L_maps.size()) selected = 0;
                    return true;
                }
            }
            return true;
        }

        virtual void Render(const GLFont* pFont03, const GLFont* pFont04,
                            const GLFont* pFont05, const GLFont* pFont10,
                            xDWORD Width, xDWORD Height)
        {
            xDWORD WidthHalf  = (xDWORD)(Width * 0.5f);

            xFLOAT lineHeight03 = pFont03->LineH();
            xFLOAT lineHeight05 = pFont05->LineH();
            xFLOAT HeadersHeight = lineHeight05*2.f;

            widthHalf = WidthHalf;

            glColor4f( 1.0f, 1.0f, 1.0f, 1.f );

            glBegin(GL_LINES);
            {
                glVertex2f(0.f,           HeadersHeight);
                glVertex2f((xFLOAT)Width, HeadersHeight);

                glVertex2f(0.f,           Height-HeadersHeight);
                glVertex2f((xFLOAT)Width, Height-HeadersHeight);
            }
            glEnd();

            const char* title = "Select map";
            xFLOAT textLen = pFont05->Length(title);
            pFont05->PrintF((Width - textLen) * 0.5f, lineHeight05*1.25f, 0.0f, title);

            const char* menu = "Back";
            BackButton = xRectangle(20.f, Height-lineHeight05*1.75f, pFont05->Length(menu), lineHeight05);
            if (BackButton.Contains(g_InputMgr.mouseX, g_InputMgr.mouseY))
                glColor4f( 1.0f, 1.0f, 0.0f, 1.f );
            else
                glColor4f( 1.0f, 1.0f, 1.0f, 1.f );
            pFont05->PrintF(BackButton.X, Height-lineHeight05*0.75f, 0.0f, menu);

            const char* play = "Next";
            textLen = pFont05->Length(play);
            NextButton = xRectangle(Width - 20.f - textLen, Height-lineHeight05*1.75f, textLen, lineHeight05);
            if (NextButton.Contains(g_InputMgr.mouseX, g_InputMgr.mouseY))
                glColor4f( 1.0f, 1.0f, 0.0f, 1.f );
            else
                glColor4f( 1.0f, 1.0f, 1.0f, 1.f );
            pFont05->PrintF(NextButton.X, Height-lineHeight05*0.75f, 0.0f, play);

            glColor4f( 1.0f, 1.0f, 1.0f, 1.f );
            xFLOAT y = HeadersHeight + lineHeight03*2;
            mapsY = (xDWORD)y;
            mapsH = (xDWORD)lineHeight03;
            ////// Maps
            textLen = pFont03->Length("Maps:");
            pFont03->PrintF(WidthHalf * 0.1f, y, 0.0f, "Maps:");
            y += lineHeight03;

            for(size_t i = 0; i < L_maps.size(); ++i)
            {
                if (selected == i)
                    glColor4f( 1.0f, 1.0f, 0.0f, 1.f );
                else
                    glColor4f( 1.0f, 1.0f, 1.0f, 1.f );
                const char *name = L_maps[i].Name.c_str();
                textLen = pFont03->Length(name);
                pFont03->PrintF(WidthHalf * 0.2f, y, 0.0f, name);
                y += lineHeight03;
            }

            if (L_maps[selected].Image.size())
            {
                if (!g_TextureMgr.IsHandleValid(L_maps[selected].Texture))
                    return;

                glEnable(GL_TEXTURE_2D);
                g_TextureMgr.BindTexture(L_maps[selected].Texture);
                xFLOAT W = (xFLOAT) g_TextureMgr.GetWidth(L_maps[selected].Texture);
                xFLOAT H = (xFLOAT) g_TextureMgr.GetHeight(L_maps[selected].Texture);

                if (W > widthHalf)
                {
                    H *= widthHalf / W;
                    W = (xFLOAT)widthHalf;
                }
                if (H > Height - HeadersHeight*2 - 20)
                {
                    W *= (Height - HeadersHeight*2 - 20) / H;
                    H = Height - HeadersHeight*2 - 20;
                }

                xFLOAT x = widthHalf + (widthHalf - W) * 0.5f - 10;
                xFLOAT y = Height - HeadersHeight - H - 10;

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

                glDisable(GL_TEXTURE_2D);
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
