#include "PlayState.h"
#include "CreditsState.h"

class EditModelState : public BaseState {

    virtual void Init(BaseState *parent)
    {
        BaseState::Init(parent);
        Name = "Model editor";
        FL_enabled = false;
    }

};

class EditMapState : public BaseState {

    virtual void Init(BaseState *parent)
    {
        BaseState::Init(parent);
        Name = "Map editor";
        FL_enabled = false;
    }

};

class OptionsState : public BaseState {

    virtual void Init(BaseState *parent)
    {
        BaseState::Init(parent);
        Name = "Options";
        FL_enabled = false;
    }

};

class ExitState : public BaseState {

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

#include "MainState.h"
