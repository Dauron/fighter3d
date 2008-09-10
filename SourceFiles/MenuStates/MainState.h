// included in MenuStates.h

namespace Scenes { namespace Menu {

    struct MainState : public BaseState {

        xBYTE selected;

        std::vector<xRectangle> buttons;

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

                if (im.InputDown_GetAndRaise(IC_LClick))
                {
                    int mouseX = im.mouseX;
                    int mouseY = im.mouseY;

                    for (size_t i = 0; i < buttons.size(); ++i)
                        if (buttons[i].Contains(mouseX, mouseY))
                        {
                            if (SubStates[i]->FL_enabled) SwitchState(*SubStates[i]);
                            return true;
                        }
                }
                return res;
            }
            return true;
        }

        virtual void Render(const GLFont* pFont03, const GLFont* pFont04,
                        const GLFont* pFont05, const GLFont* pFont10,
                        xDWORD Width, xDWORD Height)
        {
            xFLOAT lineHeight05 = pFont05->LineH();
            xFLOAT lineHeight10 = pFont10->LineH();

            xFLOAT textLen = pFont10->Length(Name.c_str());

            glColor4f( 1.0f, 0.0f, 0.0f, 1.f );
            pFont10->Print((Width - textLen) * 0.5f, lineHeight10, 0.0f, Name.c_str());

            int mouseX = g_InputMgr.mouseX;
            int mouseY = g_InputMgr.mouseY;

            xFLOAT y = (Height - lineHeight05 * SubStates.size()) * 0.5f;
            int    i = 0;
            Vec_State::iterator SS_curr = SubStates.begin(),
                                SS_last = SubStates.end();
            for (; SS_curr != SS_last; ++SS_curr, ++i)
            {
                BaseState &state = **SS_curr;

                textLen = pFont05->Length(state.Name.c_str());
                buttons[i] = xRectangle((Width - textLen) * 0.5f, y+lineHeight05*(i-1), textLen, lineHeight05);

                if (!state.FL_enabled)
                    glColor4f( 0.6f, 0.6f, 0.6f, 1.f );
                else
                if (i == selected || buttons[i].Contains(mouseX, mouseY))
                    glColor4f( 1.0f, 1.0f, 0.0f, 1.f );
                else
                    glColor4f( 1.0f, 1.0f, 1.0f, 1.f );
                pFont05->Print(buttons[i].X, y+lineHeight05*i, 0.0f, state.Name.c_str());
            }
        }
    };

} } // namespace Scenes::Menu
