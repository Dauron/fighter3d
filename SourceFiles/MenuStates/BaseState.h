#ifndef __incl_Scenes_MenuStates_BaseState_h
#define __incl_Scenes_MenuStates_BaseState_h

#include "../../AppFramework/Input/InputMgr.h"
#include "InputCodes.h"
#include <string>
#include <vector>
#include "../../Math/xMath.h"

namespace Scenes { namespace Menu {

    class BaseState {
    public:
        typedef std::vector<BaseState*> Vec_State;

        std::string Name;
        bool        FL_enabled;

        BaseState  *Parent;
        Vec_State   SubStates;

        virtual void Init(BaseState *parent)
        {
            Parent = parent;
            FL_enabled = true;
            Vec_State::iterator SS_curr = SubStates.begin(),
                                SS_last = SubStates.end();
            for (; SS_curr != SS_last; ++SS_curr)
                (**SS_curr).Init(this);
        }
        virtual void Invalidate() {}
        virtual void Clear()
        {
            if (Current == this)
            {
                Exit();
                Current = NULL;
            }

            Vec_State::iterator SS_curr = SubStates.begin(),
                                SS_last = SubStates.end();
            for (; SS_curr != SS_last; ++SS_curr)
            {
                (**SS_curr).Clear();
                delete *SS_curr;
            }
            SubStates.clear();
        }

        virtual bool Update(xFLOAT T_time)
        {
            if (g_InputMgr.GetInputStateAndClear(IC_Reject))
            {
                if (Parent)           SwitchState(*Parent);
                else
                if (SubStates.size()) SwitchState(*SubStates.back());
                //else                  SwitchState(*this);
                return true;
            }
            return false;
        }

        virtual void Enter() {}
        virtual void Exit()  {}

        virtual void Render(const GLFont* pFont03, const GLFont* pFont04,
                        const GLFont* pFont05, const GLFont* pFont10,
                        xDWORD Width, xDWORD Height)
        {}

        static void SwitchState(BaseState &state)
        {
            if (Current) Current->Exit();
            Current = &state;
            state.Enter();
        }

        static BaseState* Current_Get() { return Current; }

    private:
        static BaseState* Current;
    };

} } // namespace Scenes::Menu

#endif
