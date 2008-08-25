#include <fstream>
#include <map>
#include "../Utils/Filesystem.h"
#include "../AppFramework/Input/InputMgr.h"

#include "ComBoard.h"

void ComBoard :: Update(xFLOAT T_delta)
{
    if (L_actions.size() == 0) return;

    MX_shift.identity();

    Action *action = &L_actions[ID_action_cur];

    T_progress += T_delta;

    while (action->T_duration > 0.f && action->T_duration < T_progress)
    {
        xQuaternion* bones = action->Anims.GetTransformations(), *bones_f;
        if (action->FL_pos_rotation)
        {
            action->Anims.T_progress = 0;
            bones_f = action->Anims.GetTransformations();
        }

        ID_action_cur = action->ID_next;
        T_progress    = T_progress - action->T_duration;

        MX_shift.postTranslateT(bones[0].vector3);
        if (action->FL_pos_rotation)
        {
            xPoint3 N_zero; N_zero.init(0.f, -1.f, 0.f);
            xPoint3 N_first = bones_f[1].rotate(N_zero);
            xPoint3 N_last  = bones[1].rotate(N_zero);

            xFLOAT W_cos = xVector3::DotProduct(N_first, N_last);
            xFLOAT W_angle = acos(W_cos);
            xFLOAT W_rot_dir = Sign(N_first.x*N_last.y - N_first.y*N_last.x);

            MX_shift.preMultiply(xMatrixRotateRad(0.f,0.f,W_rot_dir*W_angle));
            delete[] bones_f;
        }
        delete[] bones;
    }

    action->Anims.T_progress = (xDWORD) T_progress;

    for (int i = 0; i < action->L_combos.size(); ++i)
    {
        Combo &combo = action->L_combos[i];

        if (combo.Key == Combo::Keys::Undefined) continue;
        if (combo.T_first > T_progress) return;
        if (combo.T_last > 0.f && combo.T_last  < T_progress) continue;

        if (g_InputMgr.GetInputStateAndClear(IC_CB_LeftPunch + combo.Key - Combo::Keys::LeftPunch))
        {
            xQuaternion *bones, *bones_f;

            if (combo.FL_pos_shift || action->FL_pos_rotation)
            {
                bones = action->Anims.GetTransformations();
                if (action->FL_pos_rotation)
                {
                    action->Anims.T_progress = 0;
                    bones_f = action->Anims.GetTransformations();
                }
            }

            ID_action_cur = combo.ID_action;
            T_progress    = (!combo.FL_time_shift || combo.T_last <= 0.f)
                ? combo.T_time
                : combo.T_time + T_progress - combo.T_first;
            //Update(0.f);

            if (combo.FL_pos_shift)
                MX_shift.postTranslateT(bones[0].vector3);
            if (action->FL_pos_rotation)
            {
                xPoint3 N_zero; N_zero.init(0.f, -1.f, 0.f);
                xPoint3 N_first = bones_f[1].rotate(N_zero);
                xPoint3 N_last  = bones[1].rotate(N_zero);

                xFLOAT W_cos = xVector3::DotProduct(N_first, N_last);
                xFLOAT W_angle = acos(W_cos);
                xFLOAT W_rot_dir = Sign(N_first.x*N_last.y - N_first.y*N_last.x);

                MX_shift.preMultiply(xMatrixRotateRad(0.f,0.f,W_rot_dir*W_angle));
                delete[] bones_f;
            }
            if (combo.FL_pos_shift || action->FL_pos_rotation)
                delete[] bones;
            return;
        }
    }
}

void ComBoard :: UpdateIDs()
{
    std::map<std::string, xBYTE> map;
    std::map<std::string, xBYTE>::iterator found;

    for (int i = 0; i < L_actions.size(); ++i)
    {
        L_actions[i].ID = i;
        map[L_actions[i].SN_name] = i;
    }

    for (int i = 0; i < L_actions.size(); ++i)
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

        for (int j = 0; j < L_actions[i].L_combos.size(); ++j)
        {
            found = map.find(L_actions[i].L_combos[j].SN_action);
            if (found != map.end())
                L_actions[i].L_combos[j].ID_action = found->second;
            else
                L_actions[i].L_combos[j].ID_action = 0;
        }
    }
}

void ComBoard :: Load(char *fileName)
{
    Init();

    std::ifstream in;
    in.open(Filesystem::GetFullPath(fileName).c_str());
    if (in.is_open())
    {
        std::string dir = Filesystem::GetParentDir(fileName);
        char buffer[255];
        int  len;

        Action action;
        Combo  combo;

        enum LoadMode
        {
            LoadMode_None,
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
                        action.Free();
                    action.Init();
                    combo.Init();
                    mode = LoadMode_Action;
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
                        combo.Init();
                        mode = LoadMode_Combo;
                        continue;
                    }
                    continue;
                }
            }
            if (mode == LoadMode_Action)
            {
                if (StartsWith(buffer, "name"))
                {
                    char name[255];
                    sscanf(buffer+4, "%s", name);
                    action.SN_name = name;
                    continue;
                }
                if (StartsWith(buffer, "anim"))
                {
                    int start = 0, end = 0;
                    char file[255];
                    sscanf(buffer+4, "%s\t%d\t%d", file, &start, &end);
                    std::string animFile = Filesystem::GetFullPath(dir + "/" + file);
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
                    char name[255];
                    sscanf(buffer+4, "%s", name);
                    action.SN_next = name;
                    continue;
                }
                if (StartsWith(buffer, "rotate"))
                {
                    int val;
                    sscanf(buffer+6, "%d", &val);
                    action.FL_pos_rotation = val != 0;
                    continue;
                }
            }
            if (mode == LoadMode_Combo)
            {
                if (StartsWith(buffer, "action"))
                {
                    char name[255];
                    sscanf(buffer+6, "%s", name);
                    combo.SN_action = name;
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
            action.Free(); // Release animations

        in.close();

        UpdateIDs();
    }
}
