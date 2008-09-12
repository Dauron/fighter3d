// included in MenuStates.h

namespace Scenes { namespace Menu {

    struct CreditsState : public BaseState {

        virtual void Init(BaseState *parent)
        {
            BaseState::Init(parent);
            Name = "Credits";
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
            xFLOAT lineHeight04 = pFont04->LineH();
            xFLOAT lineHeight05 = pFont05->LineH();

            xFLOAT totalHeight = lineHeight04*3+lineHeight05*3;
            xFLOAT y = (Height-totalHeight)*0.5f;

            xFLOAT textLen = pFont04->Length("programming");
            glColor4f( 1.0f, 1.0f, 1.0f, 1.f );
            pFont04->Print((Width - textLen) * 0.5f, y, 0.0f, "programming");
            textLen = pFont05->Length("Dariusz Maciejewski");
            glColor4f( 1.0f, 1.0f, 0.0f, 1.f );
            pFont05->Print((Width - textLen) * 0.5f, y+lineHeight04, 0.0f, "Dariusz Maciejewski");

            textLen = pFont04->Length("supervision");
            glColor4f( 1.0f, 1.0f, 1.0f, 1.f );
            pFont04->Print((Width - textLen) * 0.5f, y+lineHeight05+lineHeight04, 0.0f, "supervision");
            textLen = pFont05->Length("Przemyslaw Rokita");
            glColor4f( 1.0f, 1.0f, 0.0f, 1.f );
            pFont05->Print((Width - textLen) * 0.5f, y+lineHeight05+lineHeight04*2, 0.0f, "Przemyslaw Rokita");

            textLen = pFont04->Length("art");
            glColor4f( 1.0f, 1.0f, 1.0f, 1.f );
            pFont04->Print((Width - textLen) * 0.5f, y+lineHeight05*2+lineHeight04*2, 0.0f, "art");
            textLen = pFont05->Length("Eliza Mielnicka");
            glColor4f( 1.0f, 1.0f, 0.0f, 1.f );
            pFont05->Print((Width - textLen) * 0.5f, y+lineHeight05*2+lineHeight04*3, 0.0f, "Eliza Mielnicka");
            textLen = pFont05->Length("Dariusz Maciejewski");
            pFont05->Print((Width - textLen) * 0.5f, y+lineHeight05*3+lineHeight04*3, 0.0f, "Dariusz Maciejewski");
        }
    };

} } // namespace Scenes::Menu
