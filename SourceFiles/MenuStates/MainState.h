// included in MenuStates.h

namespace Scenes { namespace Menu {

    struct MainState : public BaseState {

        xBYTE selected;

        std::vector<xRectangle> buttons;

        HTexture backGround;
        HTexture marker;
        bool     FL_mouseOver;

        virtual void Init(BaseState *parent)
        {
            Name = "Camera Fighter";
            SubStates.push_back(new SelectMapState());
            SubStates.push_back(new EditModelState());
            SubStates.push_back(new EditMapState());
            SubStates.push_back(new OptionsState());
            SubStates.push_back(new CreditsState());
            SubStates.push_back(new ExitState());

            SubStates[0]->SubStates.push_back(new PlayState());

            buttons.resize(SubStates.size());

            BaseState::Init(parent);

            selected = -1;
            do {
                ++selected;
                if (selected >= SubStates.size()) selected = 0;
            }
            while (!SubStates[selected]->FL_enabled);

            backGround = g_TextureMgr.GetTexture("Data/textures/menu/main.tga");
            marker     = g_TextureMgr.GetTexture("Data/textures/menu/dot.tga");
        }

        virtual void Clear()
        {
            BaseState::Clear();
            g_TextureMgr.Release(backGround);
            g_TextureMgr.Release(marker);
            backGround = HTexture();
            marker     = HTexture();
        }

        virtual bool Update(xFLOAT T_time)
        {
            if (!BaseState::Update(T_time))
            {
                bool res = false;
                InputMgr &im = g_InputMgr;
                if (im.InputDown_GetAndRaise(IC_MoveUp))
                {
                    res = true;
                    do {
                        --selected;
                        if (selected == xBYTE_MAX) selected = SubStates.size()-1;
                    }
                    while (!SubStates[selected]->FL_enabled);
                }
                if (im.InputDown_GetAndRaise(IC_MoveDown))
                {
                    res = true;
                    do {
                        ++selected;
                        if (selected >= SubStates.size()) selected = 0;
                    }
                    while (!SubStates[selected]->FL_enabled);
                }
                if (im.InputDown_GetAndRaise(IC_Accept))
                {
                    SwitchState(*SubStates[selected]);
                    return true;
                }

                if (im.InputDown_GetAndRaise(IC_LClick) && FL_mouseOver)
                {
                    SwitchState(*SubStates[selected]);
                    return true;
                }
                return res;
            }
            return true;
        }

        virtual void Render(const Graphics::OGL::Font* pFont03, const Graphics::OGL::Font* pFont04,
                            xDWORD Width, xDWORD Height)
        {
            int mouseX = g_InputMgr.mouseX;
            int mouseY = g_InputMgr.mouseY;

            glColor4f( 1.0f, 1.0f, 1.0f, 1.f );
            glEnable (GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_TEXTURE_2D);

            xFLOAT scale = Height * 0.7f / 1024.f;
            xFLOAT left  = 0.f;
            xFLOAT top   = 0.f;

            if (!backGround.IsNull())
            {
                xFLOAT iWidth  = scale * g_TextureMgr.GetWidth(backGround);
                xFLOAT iHeight = scale * g_TextureMgr.GetHeight(backGround);

                left = Width  - iWidth  - 170.f*scale;
                top  = Height - iHeight - 150.f*scale;

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

            xFLOAT textLeft   = left + 525.f * scale;
            xFLOAT textRight  = left + 790.f * scale;
            xFLOAT textTop    = top  + 177.f * scale;
            xFLOAT textHeight = 53.5f * scale;

            FL_mouseOver = false;
            if (mouseX > textLeft && mouseX < textRight)
            {
                if (mouseY > textTop && mouseY < textTop + 6 * textHeight)
                {
                    FL_mouseOver = true;
                    int i = (int)((mouseY - textTop) / textHeight);
                    if (SubStates[i]->FL_enabled)
                        selected = i;
                }
            }

            if (!marker.IsNull())
            {
                g_TextureMgr.BindTexture(marker);

                xFLOAT y = textTop + textHeight * selected;
                xFLOAT iWidth = 32 * scale;
                glBegin(GL_QUADS);
                {
                    glTexCoord2f(0.01f,0.99f);
                    glVertex2f(textRight, y);
                    glTexCoord2f(0.99f,0.99f);
                    glVertex2f(textRight+iWidth, y);
                    glTexCoord2f(0.99f,0.01f);
                    glVertex2f(textRight+iWidth, y+iWidth);
                    glTexCoord2f(0.01f,0.01f);
                    glVertex2f(textRight, y+iWidth);
                }
                glEnd();
            }

            glDisable(GL_TEXTURE_2D);
            glDisable(GL_BLEND);
        }
    };

} } // namespace Scenes::Menu
