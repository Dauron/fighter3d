#ifndef __incl_ComBoard_h
#define __incl_ComBoard_h

#include <string>
#include <vector>
#include "../Models/lib3dx/xAction.h"

class ComBoard
{
public:
    struct Combo
    {
        struct Keys {
            enum eKey {
                Undefined      = 0,

                LeftPunch      = 1,
                LeftKick       = 2,
                LeftHandGuard  = 3,
                LeftLegGuard   = 4,

                RightPunch     = 5,
                RightKick      = 6,
                RightHandGuard = 7,
                RightLegGuard  = 8,

                Forward        = 9,
                Backward       = 10,
                Left           = 11,
                Right          = 12
            };
        };

        xBYTE       ID_action;
        std::string SN_action;
        Keys::eKey  Key;
        xFLOAT      T_first;
        xFLOAT      T_last;
        xFLOAT      T_time;
        bool        FL_time_shift;
        bool        FL_pos_shift;

        void Init ()
        {
            ID_action = 0;
            Key       = Keys::Undefined;
            T_first = T_last = T_time = 0.f;
            SN_action.clear();
            FL_time_shift   = true;
            FL_pos_shift    = true;
        }
    };

    struct Action
    {
        xBYTE       ID;
        std::string SN_name;
        std::string SN_next;
        xBYTE       ID_next;
        xActionSet  Anims;
        xFLOAT      T_duration;
        bool        FL_pos_rotation;

        std::vector<Combo> L_combos;

        void Init ()
        {
            SN_name.clear();
            SN_next.clear();
            T_duration = 0.f;
            L_combos.clear();
            Anims.T_progress = 0;
            Anims.L_actions.clear();
            FL_pos_rotation = false;
        }

        void Free ()
        {
            Anims.Free();
        }
    };

    typedef std::vector<Action> Vec_Action;

    Vec_Action L_actions;

    xBYTE   ID_action_cur;
    xFLOAT  T_progress;
    xMatrix MX_shift;

    void Init()
    {
        if (L_actions.size())
        {
            Vec_Action::iterator A_curr = L_actions.begin(),
                                 A_last = L_actions.end();
            for (; A_curr != A_last; ++A_curr)
                A_curr->Free();
            L_actions.clear();
        }
        ID_action_cur = 0;
        T_progress    = 0.f;
        MX_shift.identity();
    }

    void Update(xFLOAT T_delta);
    void Load(char *fileName);

private:
    void UpdateIDs();
};

#endif
