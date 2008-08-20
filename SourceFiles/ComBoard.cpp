#include <fstream>
#include <map>
#include "../Utils/Filesystem.h"
#include "../AppFramework/Input/InputMgr.h"

#include "ComBoard.h"

void ComBoard :: Update(xFLOAT T_delta)
{
    if (L_actions.size() == 0) return;

    Action &action = L_actions[ID_action_cur];

    T_progress += T_delta;

    if (action.T_duration > 0.f && action.T_duration < T_progress)
    {
        ID_action_cur = action.ID_next;
        T_progress    = T_progress - action.T_duration;
        Update(0.f);
        return;
    }

    action.anims.T_progress = (xDWORD) T_progress;

    for (int i = 0; i < action.L_combos.size(); ++i)
    {
        Combo &combo = action.L_combos[i];

        if (combo.key == Combo::Key::Undefined) continue;
        if (combo.T_first > T_progress) return;
        if (combo.T_last > 0.f && combo.T_last  < T_progress) continue;

        if (g_InputMgr.GetInputStateAndClear(IC_CB_LeftPunch + combo.key - Combo::Key::LeftPunch))
        {
            ID_action_cur = combo.ID_action;
            T_progress    = (combo.T_last <= 0.f)
                ? combo.T_time
                : combo.T_time + T_progress - combo.T_first;
            Update(0.f);
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
        map[L_actions[i].name] = i;
    }

    for (int i = 0; i < L_actions.size(); ++i)
    {
        if (L_actions[i].next.length())
        {
            found = map.find(L_actions[i].next);
            if (found != map.end())
                L_actions[i].ID_next = found->second;
            else
                L_actions[i].ID_next = 0;
        }
        else
            L_actions[i].ID_next = 0;

        for (int j = 0; j < L_actions[i].L_combos.size(); ++j)
        {
            found = map.find(L_actions[i].L_combos[j].action);
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
                    if (combo.action.length())
                        action.L_combos.push_back(combo);
                    if (action.name.length())
                        L_actions.push_back(action);
                    action.init();
                    combo.init();
                    mode = LoadMode_Action;
                    continue;
                }
                mode = LoadMode_None;
                continue;
            }
            if (*buffer == '(')
            {
                if (mode != LoadMode_None && StartsWith(buffer, "(combo)"))
                {
                    if (combo.action.length())
                        action.L_combos.push_back(combo);
                    combo.init();
                    mode = LoadMode_Combo;
                    continue;
                }
                continue;
            }
            if (mode == LoadMode_Action)
            {
                if (StartsWith(buffer, "name"))
                {
                    char name[255];
                    sscanf(buffer, "name\t%s", name);
                    action.name = name;
                    continue;
                }
                if (StartsWith(buffer, "anim"))
                {
                    int start = 0, end = 0;
                    char file[255];
                    sscanf(buffer, "anim\t%s\t%d\t%d", file, &start, &end);
                    std::string animFile = Filesystem::GetFullPath(dir + "/" + file);
                    if (end <= start)
                        action.anims.AddAnimation(animFile.c_str(), start);
                    else
                        action.anims.AddAnimation(animFile.c_str(), start, end);
                    continue;
                }
                if (StartsWith(buffer, "time"))
                {
                    float time;
                    sscanf(buffer, "time\t%f", &time);
                    action.T_duration = time;
                    continue;
                }
                if (StartsWith(buffer, "post"))
                {
                    char name[255];
                    sscanf(buffer, "name\t%s", name);
                    action.next = name;
                    continue;
                }
            }
            if (mode == LoadMode_Combo)
            {
                if (StartsWith(buffer, "action"))
                {
                    char name[255];
                    sscanf(buffer, "action\t%s", name);
                    combo.action = name;
                    continue;
                }
                if (StartsWith(buffer, "key"))
                {
                    char name[255];
                    sscanf(buffer, "key\t%s", name);
                    if (StartsWith(name, "LeftPunch"))      combo.key = Combo::Key::LeftPunch;
                    else
                    if (StartsWith(name, "LeftKick"))       combo.key = Combo::Key::LeftKick;
                    else
                    if (StartsWith(name, "LeftHandGuard"))  combo.key = Combo::Key::LeftHandGuard;
                    else
                    if (StartsWith(name, "LeftLegGuard"))   combo.key = Combo::Key::LeftLegGuard;
                    else
                    if (StartsWith(name, "RightPunch"))     combo.key = Combo::Key::RightPunch;
                    else
                    if (StartsWith(name, "RightKick"))      combo.key = Combo::Key::RightKick;
                    else
                    if (StartsWith(name, "RightHandGuard")) combo.key = Combo::Key::RightHandGuard;
                    else
                    if (StartsWith(name, "RightLegGuard"))  combo.key = Combo::Key::RightLegGuard;
                    else
                    if (StartsWith(name, "Forward"))        combo.key = Combo::Key::Forward;
                    else
                    if (StartsWith(name, "Backward"))       combo.key = Combo::Key::Backward;
                    else
                    if (StartsWith(name, "Left"))           combo.key = Combo::Key::Left;
                    else
                    if (StartsWith(name, "Right"))          combo.key = Combo::Key::Right;
                    else                                    combo.key = Combo::Key::Undefined;
                    continue;
                }
                if (StartsWith(buffer, "first"))
                {
                    float time;
                    sscanf(buffer, "first\t%f", &time);
                    combo.T_first = time;
                    continue;
                }
                if (StartsWith(buffer, "last"))
                {
                    float time;
                    sscanf(buffer, "last\t%f", &time);
                    combo.T_last = time;
                    continue;
                }
                if (StartsWith(buffer, "time"))
                {
                    float time;
                    sscanf(buffer, "time\t%f", &time);
                    combo.T_time = time;
                    continue;
                }
            }
        }
        if (combo.action.length())
            action.L_combos.push_back(combo);
        if (action.name.length())
            L_actions.push_back(action);

        in.close();

        UpdateIDs();
    }
}
