#include <fstream>
#include <map>
#include "../Utils/Filesystem.h"
#include "../AppFramework/Input/InputMgr.h"
#include "InputCodes.h"

#include "ComBoard.h"

void ComBoard :: Destroy()
{
    FileName = Filesystem::GetFullPath("Data/models/anims/karate.txt");

    if (L_actions.size())
    {
        Vec_Action::iterator A_curr = L_actions.begin(),
                             A_last = L_actions.end();
        for (; A_curr != A_last; ++A_curr)
            A_curr->Destroy();
        L_actions.clear();
    }
    ID_action_cur = 0;
    T_progress    = 0.f;
    MX_shift.identity();

    L_mirror.clear();
    FL_mirror = false;

    for (int i = 0; i < AutoHint::HINT_COUNT; ++i)
        L_hint[i].clear();
    StopAction.Clear();
    AutoAction = AutoHint::HINT_NONE;
}

xMatrix ComBoard :: GetActionRotation(Action &action)
{
    if (!action.FL_pos_rotation) return xMatrix::Identity();

    xDWORD T_progress = action.Anims.T_progress;
    xQuaternion bone1 = action.Anims.GetTransformation(1);
    if (action.FL_mirror)
    {
        bone1.y = -bone1.y;
        bone1.z = -bone1.z;
    }

    action.Anims.T_progress = 0;
    xQuaternion bone_f1 = action.Anims.GetTransformation(1);
    action.Anims.T_progress = T_progress;

    xPoint3 N_zero; N_zero.init(0.f, -1.f, 0.f);
    xPoint3 N_first = bone_f1.rotate(N_zero);
    xPoint3 N_last  = bone1.rotate(N_zero);

    xFLOAT W_cos = xVector3::DotProduct(N_first, N_last);
    xFLOAT W_angle = acos(W_cos);
    xFLOAT W_rot_dir = Sign(N_first.x*N_last.y - N_first.y*N_last.x);

    if (FL_mirror) W_rot_dir = -W_rot_dir;

    return xMatrixRotateRad(0.f,0.f,W_rot_dir*W_angle);
}

void ComBoard :: PostActionTransformation(Action &action, bool FL_pos_shift)
{
    if (FL_pos_shift)
    {
        xVector3 NW_shift = action.Anims.GetTransformation(0).vector3;
        if (FL_mirror) NW_shift.x = -NW_shift.x;
        MX_shift.postTranslateT(NW_shift);
    }
    if (action.FL_pos_rotation)
        MX_shift.preMultiply(GetActionRotation(action));
    if (action.FL_mirror)
        FL_mirror = !FL_mirror;
}

void ComBoard :: Update(xFLOAT T_delta, bool FL_keyboard_on)
{
    MX_shift.identity();
    if (L_actions.size() == 0) return;

    Action *action = &L_actions[ID_action_cur];

    T_progress += T_delta;

    while (action->T_duration > 0.f && action->T_duration < T_progress)
    {
        action->Anims.T_progress = (xDWORD) action->T_duration;
        PostActionTransformation(*action, true);

        ID_action_cur = action->ID_next;
        T_progress    = T_progress - action->T_duration;
    }

    action->Anims.T_progress = (xDWORD) T_progress;

    if (!FL_keyboard_on) return;

    for (xBYTE i = 0; i < action->L_combos.size(); ++i)
    {
        Combo &combo = action->L_combos[i];

        if (combo.Key == Combo::Keys::Undefined) continue;
        if (combo.T_first > T_progress) return;
        if (combo.T_last > 0.f && combo.T_last  < T_progress) continue;

        int key = combo.Key;
        if (FL_mirror)
            if (key >= Combo::Keys::LeftFirst && key <= Combo::Keys::LeftLast)
                key += Combo::Keys::RightFirst - Combo::Keys::LeftFirst;
            else
            if (key >= Combo::Keys::RightFirst && key <= Combo::Keys::RightLast)
                key += Combo::Keys::LeftFirst - Combo::Keys::RightFirst;
        if (g_InputMgr.InputDown_GetAndRaise(IC_CB_ComboSet0 + key - Combo::Keys::FirstKey))
        {
            AutoAction = AutoHint::HINT_NONE;
            PostActionTransformation(*action, combo.FL_pos_shift);

            ID_action_cur = combo.ID_action;
            T_progress    = (!combo.FL_time_shift || combo.T_last <= 0.f)
                ? combo.T_time
                : combo.T_time + T_progress - combo.T_first;
            L_actions[ID_action_cur].Anims.T_progress = (xDWORD) T_progress;
            //Update(0.f);
            return;
        }
    }
}

xQuaternion *ComBoard :: GetTransformations()
{
    if (L_actions.size() == 0) return NULL;

    xQuaternion *bones = L_actions[ID_action_cur].Anims.GetTransformations();
    if (!FL_mirror || !bones) return bones;

    bones[0].x = -bones[0].x;

    xQuaternion *B_curr = bones + 1,
                *B_last = bones + I_bones;

    for (; B_curr != B_last; ++B_curr)
    {
        B_curr->y = -B_curr->y;
        B_curr->z = -B_curr->z;
    }

    Vec_Mirror::iterator MB_curr = L_mirror.begin(),
                         MB_last = L_mirror.end();
    for (; MB_curr != MB_last; ++MB_curr)
    {
        xQuaternion swp = bones[MB_curr->ID_bone1];
        bones[MB_curr->ID_bone1] = bones[MB_curr->ID_bone2];
        bones[MB_curr->ID_bone2] = swp;
    }

    return bones;
}

bool ComBoard :: SetBestAction(AutoHint::Type ActionType, xFLOAT S_dest)
{
    int    I_best = -1;
    xFLOAT S_best = xFLOAT_HUGE_POSITIVE;

    Vec_Hint &hints = L_hint[ActionType];
    Vec_Hint::iterator AH_curr = hints.begin(),
                       AH_last = hints.end();
    for (xBYTE i = 0; AH_curr != AH_last; ++AH_curr, ++i)
    {
        xFLOAT S_curr = AH_curr->S_max_change - S_dest;
        if (S_curr <= 0.f || (AH_curr->FL_breakable && S_curr > 0.f))
        {
            S_curr = fabs(S_curr);
            if (S_curr < S_best)
            {
                S_best = S_curr;
                I_best = i;
            }
        }
    }

    if (I_best >= 0)
    {
        ID_action_cur = hints[I_best].ID_action;
        T_progress    = 0.f;
        L_actions[ID_action_cur].Anims.T_progress = 0;
        AutoAction = ActionType;
        return true;
    }
    return false;
}

bool ComBoard :: AutoMovement(const xVector3 &NW_aim, const xVector3 &NW_dst, xFLOAT T_delta)
{
    MX_shift.identity();
    if (L_actions.size() == 0) return false;

    xVector3 N_aim = xVector3::Normalize(NW_aim);
    xVector3 N_dst = xVector3::Normalize(NW_dst);

    xFLOAT W_cos     = xVector3::DotProduct(N_aim, N_dst);
    xFLOAT W_rot_dir = Sign(NW_aim.x*NW_dst.y - NW_aim.y*NW_dst.x);
    xFLOAT W_angle   = acos(W_cos);
    xFLOAT S_dist    = NW_dst.length() - NW_aim.length();

    if ( (AutoAction == AutoHint::HINT_STEP && S_dist <  0.1f) ||
         (AutoAction == AutoHint::HINT_BACK && S_dist > -0.1f) ||
         (!FL_mirror && (
          (AutoAction == AutoHint::HINT_LEFT  && W_angle * W_rot_dir <  EPSILON3) ||
          (AutoAction == AutoHint::HINT_RIGHT && W_angle * W_rot_dir > -EPSILON3)
          ) ) ||
         (FL_mirror && (
          (AutoAction == AutoHint::HINT_LEFT  && W_angle * W_rot_dir > -EPSILON3) ||
          (AutoAction == AutoHint::HINT_RIGHT && W_angle * W_rot_dir <  EPSILON3)
          ) ) )
    {
        PostActionTransformation(L_actions[ID_action_cur], true);
        ID_action_cur = StopAction.ID_action;
        T_progress    = 0.f;
        L_actions[ID_action_cur].Anims.T_progress = 0;
        AutoAction = AutoHint::HINT_NONE;
    }

    // small auto rotation
    if (W_angle > EPSILON3 && W_angle < DEGTORAD(5))
    {
        //W_angle *= T_time * 0.5f;
        xFLOAT W_scale = T_delta * PI * 0.25f;
        W_angle = ((W_scale > W_angle) ? W_angle : W_scale) * 0.5f;
        xQuaternion QT_rot; QT_rot.init(0.f,0.f,W_rot_dir*sin(W_angle),cos(W_angle));
        MX_shift.preMultiply(xMatrixFromQuaternionT(QT_rot));
    }

    // We cannot break other actions in progress
    if (ID_action_cur != StopAction.ID_action) return false;

    // Try to auto rotate
    if (W_angle >= DEGTORAD(5))
    {
        if (FL_mirror) W_rot_dir = -W_rot_dir;
        if (SetBestAction(W_rot_dir > 0.f ? AutoHint::HINT_LEFT : AutoHint::HINT_RIGHT, W_angle))
            return true;
    }

    // Try to auto step
    if (S_dist < 0.5f && S_dist > -0.5f) return false;
    return SetBestAction(S_dist > 0.f ? AutoHint::HINT_STEP : AutoHint::HINT_BACK, S_dist);
}

void ComBoard :: UpdateIDs()
{
    std::map<std::string, xBYTE> map;
    std::map<std::string, xBYTE>::iterator found;

    for (xBYTE i = 0; i < L_actions.size(); ++i)
    {
        L_actions[i].ID = i;
        map[L_actions[i].SN_name] = i;
    }

    for (xBYTE i = 0; i < L_actions.size(); ++i)
    {
        if (L_actions[i].SN_next.length())
        {
            found = map.find(L_actions[i].SN_next);
            if (found != map.end())
                L_actions[i].ID_next = found->second;
            else
                L_actions[i].ID_next = 0;
        }
        else
            L_actions[i].ID_next = 0;

        for (xBYTE j = 0; j < L_actions[i].L_combos.size(); ++j)
        {
            found = map.find(L_actions[i].L_combos[j].SN_action);
            if (found != map.end())
                L_actions[i].L_combos[j].ID_action = found->second;
            else
                L_actions[i].L_combos[j].ID_action = 0;
        }
    }

    for (xBYTE i = 0; i < AutoHint::HINT_COUNT; ++i)
        for (xBYTE j = 0; j < L_hint[i].size(); ++j)
        {
            found = map.find(L_hint[i][j].SN_action);
            if (found != map.end())
                L_hint[i][j].ID_action = found->second;
            else
                L_hint[i][j].ID_action = 0;
        }

    found = map.find(StopAction.SN_action);
    if (found != map.end())
        StopAction.ID_action = found->second;
    else
        StopAction.ID_action = 0;
}

void ComBoard :: Load(const char *fileName, xBYTE I_bones)
{
    Destroy();
    this->I_bones = I_bones;
    FileName = Filesystem::GetFullPath(fileName);

    std::ifstream in;
    in.open(FileName.c_str());
    if (in.is_open())
    {
        std::string dir = Filesystem::GetParentDir(fileName);
        char buffer[255];
        int  len;

        AutoHint hint;
        Action   action;
        Combo    combo;
        action.Clear();
        combo.Clear();

        enum LoadMode
        {
            LoadMode_None,
            LoadMode_Auto,
            LoadMode_Mirror,
            LoadMode_Action,
            LoadMode_Combo
        } mode = LoadMode_None;

        while (in.good())
        {
            in.getline(buffer, 255);
            if (buffer[0] == 0 || buffer[0] == '#') continue;
            len = strlen(buffer);
            if (buffer[len - 1] == '\r') buffer[len - 1] = 0;

            if (*buffer == '[')
            {
                if (StartsWith(buffer, "[action]"))
                {
                    if (combo.SN_action.length())
                        action.L_combos.push_back(combo);
                    if (action.SN_name.length())
                        L_actions.push_back(action);
                    else
                        action.Destroy();
                    action.Clear();
                    combo.Clear();
                    mode = LoadMode_Action;
                    continue;
                }
                if (StartsWith(buffer, "[auto]"))
                {
                    mode = LoadMode_Auto;
                    continue;
                }
                if (StartsWith(buffer, "[mirror]"))
                {
                    mode = LoadMode_Mirror;
                    continue;
                }
                mode = LoadMode_None;
                continue;
            }
            if (mode == LoadMode_Action || mode == LoadMode_Combo)
            {
                if (*buffer == '(')
                {
                    if (mode != LoadMode_None && StartsWith(buffer, "(combo)"))
                    {
                        if (combo.SN_action.length())
                            action.L_combos.push_back(combo);
                        combo.Clear();
                        mode = LoadMode_Combo;
                        continue;
                    }
                    continue;
                }
            }
            if (mode == LoadMode_Auto)
            {
                if (StartsWith(buffer, "stop"))
                {
                    StopAction.SN_action = ReadSubstring(buffer+4);
                    continue;
                }
                if (StartsWith(buffer, "step"))
                {
                    const char* params = NULL;
                    hint.SN_action = ReadSubstring(buffer+4, params);
                    if (params)
                    {
                        int flag = 1;
                        sscanf(params, "%f %d", &hint.S_max_change, &flag);
                        hint.FL_breakable = flag;
                    }
                    L_hint[AutoHint::HINT_STEP].push_back(hint);
                    continue;
                }
                if (StartsWith(buffer, "back"))
                {
                    const char* params = NULL;
                    hint.SN_action = ReadSubstring(buffer+4, params);
                    if (params)
                    {
                        int flag = 1;
                        sscanf(params, "%f %d", &hint.S_max_change, &flag);
                        hint.FL_breakable = flag;
                    }
                    L_hint[AutoHint::HINT_BACK].push_back(hint);
                    continue;
                }
                if (StartsWith(buffer, "left"))
                {
                    const char* params = NULL;
                    hint.SN_action = ReadSubstring(buffer+4, params);
                    if (params)
                    {
                        int flag = 1;
                        sscanf(params, "%f %d", &hint.S_max_change, &flag);
                        hint.FL_breakable = flag;
                    }
                    L_hint[AutoHint::HINT_LEFT].push_back(hint);
                    continue;
                }
                if (StartsWith(buffer, "right"))
                {
                    const char* params = NULL;
                    hint.SN_action = ReadSubstring(buffer+5, params);
                    if (params)
                    {
                        int flag = 1;
                        sscanf(params, "%f %d", &hint.S_max_change, &flag);
                        hint.FL_breakable = flag;
                    }
                    L_hint[AutoHint::HINT_RIGHT].push_back(hint);
                    continue;
                }
            }
            if (mode == LoadMode_Mirror)
            {
                if (StartsWith(buffer, "bones"))
                {
                    int b1, b2;
                    sscanf(buffer+5, "%d %d", &b1, &b2);
                    Mirror mirror;
                    mirror.ID_bone1 = b1;
                    mirror.ID_bone2 = b2;
                    L_mirror.push_back(mirror);
                    continue;
                }
            }
            if (mode == LoadMode_Action)
            {
                if (StartsWith(buffer, "name"))
                {
                    action.SN_name = ReadSubstring(buffer+4);
                    continue;
                }
                if (StartsWith(buffer, "anim"))
                {
                    const char* params = NULL;
                    std::string animFile = Filesystem::GetFullPath(dir + "/" + ReadSubstring(buffer+4, params));

                    int start = 0, end = 0;
                    if (params)
                        sscanf(params, "%d %d", &start, &end);

                    if (end <= start)
                        action.Anims.AddAnimation(animFile.c_str(), start);
                    else
                        action.Anims.AddAnimation(animFile.c_str(), start, end);
                    continue;
                }
                if (StartsWith(buffer, "time"))
                {
                    float time;
                    sscanf(buffer+4, "%f", &time);
                    action.T_duration = time;
                    continue;
                }
                if (StartsWith(buffer, "post"))
                {
                    action.SN_next = ReadSubstring(buffer+4);
                    continue;
                }
                if (StartsWith(buffer, "rotate"))
                {
                    int val;
                    sscanf(buffer+6, "%d", &val);
                    action.FL_pos_rotation = val != 0;
                    continue;
                }
                if (StartsWith(buffer, "mirror"))
                {
                    int val;
                    sscanf(buffer+6, "%d", &val);
                    action.FL_mirror = val;
                    continue;
                }
            }
            if (mode == LoadMode_Combo)
            {
                if (StartsWith(buffer, "action"))
                {
                    combo.SN_action = ReadSubstring(buffer+6);
                    continue;
                }
                if (StartsWith(buffer, "key"))
                {
                    char name[255];
                    sscanf(buffer+3, "%s", name);
                    if (StartsWith(name, "LeftPunch"))      combo.Key = Combo::Keys::LeftPunch;
                    else
                    if (StartsWith(name, "LeftKick"))       combo.Key = Combo::Keys::LeftKick;
                    else
                    if (StartsWith(name, "LeftHandGuard"))  combo.Key = Combo::Keys::LeftHandGuard;
                    else
                    if (StartsWith(name, "LeftLegGuard"))   combo.Key = Combo::Keys::LeftLegGuard;
                    else
                    if (StartsWith(name, "RightPunch"))     combo.Key = Combo::Keys::RightPunch;
                    else
                    if (StartsWith(name, "RightKick"))      combo.Key = Combo::Keys::RightKick;
                    else
                    if (StartsWith(name, "RightHandGuard")) combo.Key = Combo::Keys::RightHandGuard;
                    else
                    if (StartsWith(name, "RightLegGuard"))  combo.Key = Combo::Keys::RightLegGuard;
                    else
                    if (StartsWith(name, "Forward"))        combo.Key = Combo::Keys::Forward;
                    else
                    if (StartsWith(name, "Backward"))       combo.Key = Combo::Keys::Backward;
                    else
                    if (StartsWith(name, "Left"))           combo.Key = Combo::Keys::Left;
                    else
                    if (StartsWith(name, "Right"))          combo.Key = Combo::Keys::Right;
                    else                                    combo.Key = Combo::Keys::Undefined;
                    continue;
                }
                if (StartsWith(buffer, "first"))
                {
                    float time;
                    sscanf(buffer+5, "%f", &time);
                    combo.T_first = time;
                    continue;
                }
                if (StartsWith(buffer, "last"))
                {
                    float time;
                    sscanf(buffer+4, "%f", &time);
                    combo.T_last = time;
                    continue;
                }
                if (StartsWith(buffer, "time"))
                {
                    float time;
                    sscanf(buffer+4, "%f", &time);
                    combo.T_time = time;
                    continue;
                }
                if (StartsWith(buffer, "prec"))
                {
                    int val;
                    sscanf(buffer+4, "%d", &val);
                    combo.FL_time_shift = val == 0;
                    continue;
                }
                if (StartsWith(buffer, "shift"))
                {
                    int val;
                    sscanf(buffer+5, "%d", &val);
                    combo.FL_pos_shift = val != 0;
                    continue;
                }
            }
        }
        if (combo.SN_action.length())
            action.L_combos.push_back(combo);
        if (action.SN_name.length())
            L_actions.push_back(action);
        else
            action.Destroy(); // Release animations

        in.close();

        UpdateIDs();
    }
}
