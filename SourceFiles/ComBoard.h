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
        struct Key {
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
        std::string action;
        Key::eKey   key;
        xFLOAT      T_first;
        xFLOAT      T_last;
        xFLOAT      T_time;

        void init ()
        {
            ID_action = 0;
            key       = Key::Undefined;
            T_first = T_last = T_time = 0.f;
            action.clear();
        }
    };

    struct Action
    {
        xBYTE       ID;
        std::string name;
        std::string next;
        xBYTE       ID_next;
        xActionSet  anims;
        xFLOAT      T_duration;

        std::vector<Combo> L_combos;

        void init ()
        {
            name.clear();
            next.clear();
            T_duration = 0.f;
            L_combos.clear();
            anims.T_progress = 0;
            anims.L_actions.clear();
        }
    };

    std::vector<Action> L_actions;

    xBYTE  ID_action_cur;
    xFLOAT T_progress;

    void Init()
    {
        L_actions.clear();
        ID_action_cur = 0;
        T_progress    = 0.f;
    }

    void Update(xFLOAT T_delta);

    void UpdateIDs();
    void Load(char *fileName);
};

#endif
