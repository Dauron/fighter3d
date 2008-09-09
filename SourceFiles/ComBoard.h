#ifndef __incl_ComBoard_h
#define __incl_ComBoard_h

#include <string>
#include <vector>
#include "../Models/lib3dx/xAction.h"

class ComBoard
{
public:
    struct AutoHint
    {
        enum Type {
          HINT_NONE  = -1,
          HINT_STEP  =  0,
          HINT_BACK  =  1,
          HINT_LEFT  =  2,
          HINT_RIGHT =  3,
          HINT_COUNT =  4
        };

        xBYTE       ID_action;
        std::string SN_action;
        xFLOAT      S_max_change;
        bool        FL_breakable;

        void Clear ()
        {
            ID_action = 0;
            SN_action.clear();
            FL_breakable = true;
            S_max_change = 0.f;
        }
    };

    struct Combo
    {
        struct Keys {
            enum eKey {
                Undefined      = 0,

                FirstKey       = 1,

                LeftFirst      = 1,

                Left           = 1,
                LeftPunch      = 2,
                LeftKick       = 3,
                LeftHandGuard  = 4,
                LeftLegGuard   = 5,

                LeftLast       = 5,

                RightFirst     = 6,

                Right          = 6,
                RightPunch     = 7,
                RightKick      = 8,
                RightHandGuard = 9,
                RightLegGuard  = 10,

                RightLast      = 10,

                Forward        = 11,
                Backward       = 12,

                LastKey        = 12
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

        void Clear ()
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
        bool        FL_mirror;

        std::vector<Combo> L_combos;

        void Clear ()
        {
            SN_name.clear();
            SN_next.clear();
            T_duration = 0.f;
            L_combos.clear();
            Anims.T_progress = 0;
            Anims.L_actions.clear();
            FL_pos_rotation = false;
            FL_mirror = false;
        }

        void Destroy ()
        {
            Anims.Destroy();
        }
    };

    struct Mirror {
        xBYTE ID_bone1;
        xBYTE ID_bone2;
    };

    typedef std::vector<Action>   Vec_Action;
    typedef std::vector<AutoHint> Vec_Hint;
    typedef std::vector<Mirror>   Vec_Mirror;

    std::string    FileName;

    Vec_Hint       L_hint[AutoHint::HINT_COUNT];
    AutoHint       StopAction;
    AutoHint::Type AutoAction;

    Vec_Action     L_actions;
    xBYTE          ID_action_cur;
    xFLOAT         T_progress;

    bool           FL_mirror;
    Vec_Mirror     L_mirror;

    xMatrix        MX_shift;

    void Destroy();

    xMatrix GetActionRotation()
    {
        if (L_actions.size() == 0) return xMatrix::Identity();
        return GetActionRotation(L_actions[ID_action_cur]);
    }

    bool         AutoMovement  (const xVector3 &NW_aim, const xVector3 &NW_dst, xFLOAT T_delta);
    void         Update(xFLOAT T_delta, bool FL_keyboard_on);
    xQuaternion *GetTransformations(xBYTE I_bones);

    void Load(const char *fileName);

private:
    xMatrix GetActionRotation(Action &action);
    void    PostActionTransformation(Action &action, bool FL_pos_shift);
    bool    SetBestAction (AutoHint::Type ActionType, xFLOAT S_dest);

    void UpdateIDs();
};

#endif
