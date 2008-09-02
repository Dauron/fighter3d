#ifndef __incl_Scenes_MenuStates_MenuStates_h
#define __incl_Scenes_MenuStates_MenuStates_h

#include "BaseState.h"
#include "SelectMapState.h"
#include "EditModelState.h"
#include "CreditsState.h"

namespace Scenes { namespace Menu {

    struct EditMapState : public BaseState {

        virtual void Init(BaseState *parent)
        {
            BaseState::Init(parent);
            Name = "Map editor";
            FL_enabled = false;
        }

    };

    struct OptionsState : public BaseState {

        virtual void Init(BaseState *parent)
        {
            BaseState::Init(parent);
            Name = "Options";
            FL_enabled = false;
        }

    };

    struct ExitState : public BaseState {

        virtual void Init(BaseState *parent)
        {
            BaseState::Init(parent);
            Name = "Exit";
        }

        virtual void Enter()
        {
            g_Application.Terminate();
        }
    };

} } // namespace Scenes::Menu

#include "MainState.h"

#endif
