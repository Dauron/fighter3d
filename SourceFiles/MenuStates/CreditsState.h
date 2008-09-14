// included in MenuStates.h

namespace Scenes { namespace Menu {

    struct CreditsState : public BaseState {

        HTexture backGround;

        virtual void Init(BaseState *parent)
        {
            BaseState::Init(parent);
            Name = "Credits";
            backGround = g_TextureMgr.GetTexture("Data/textures/menu/credit.tga");
        }

        virtual void Clear()
        {
            BaseState::Clear();
            g_TextureMgr.Release(backGround);
            backGround = HTexture();
        }

        virtual bool Update(xFLOAT T_time)
        {
            if (g_InputMgr.InputDown_GetAndRaise(IC_Accept) ||
                g_InputMgr.InputDown_GetAndRaise(IC_LClick) ||
                g_InputMgr.InputDown_GetAndRaise(IC_RClick) )
                g_InputMgr.InputDown_Set(IC_Reject, true);
            if (!BaseState::Update(T_time))
            {
                bool res = false;
                return res;
            }
            return true;
        }

        virtual void Render(const Graphics::OGL::Font* pFont03, const Graphics::OGL::Font* pFont04,
                            const Graphics::OGL::Font* pFont05, const Graphics::OGL::Font* pFont10,
                            xDWORD Width, xDWORD Height)
        {
            if (backGround.IsNull()) return;

            glColor4f( 1.0f, 1.0f, 1.0f, 1.f );
            glEnable (GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_TEXTURE_2D);

            xFLOAT scale   = Height * 0.7f / 1024.f;
            xFLOAT iWidth  = scale * g_TextureMgr.GetWidth(backGround);
            xFLOAT iHeight = scale * g_TextureMgr.GetHeight(backGround);
            xFLOAT left = Width  - iWidth  - 170.f*scale;
            xFLOAT top  = Height - iHeight - 150.f*scale;

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

            glDisable(GL_TEXTURE_2D);
            glDisable(GL_BLEND);
        }
    };

} } // namespace Scenes::Menu
