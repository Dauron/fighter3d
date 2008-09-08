#include <fstream>
#include "InputMgr.h"

#ifdef WIN32
#pragma warning(disable : 4996) // deprecated
#endif

void InputMgr :: Key2InputCode_Set(byte kCode, int iCode)
{
    int kIndex = KeyCode2Index(kCode);
    int iIndex = InputCode2Index(iCode);

    // replace existing key mapping
    if (kIndex)
    {
        if (iIndex == kIndex)
            return;

        InputMap->KeyCode2Index[kCode] = 0;         // remove this kCode from search results
        byte kIndexOld = Index2FirstKeyCode(kIndex); // were there more mappings for this old Input Code?
        int  iCodeOld  = Index2InputCode(kIndex);    // get old mapped Input Code
        assert (iCodeOld && iCode != iCodeOld);

        // it was the only keyCode->oldInputCode mapping
        if (!kIndexOld)
        {
            // so we may reuse its index
            if (!iIndex)
            {
                InputMap->KeyCode2Index[kCode]   = kIndex;
                InputMap->InputCode2Index[iCode] = kIndex;
                FL_IndexState[kIndex]   = 0;
                return;
            }
            // we already have our own index, we must free the unused one
            if (iIndex)
            {
                InputMap->KeyCode2Index[kCode] = iIndex;

                int lastIndex = InputMap->LastIndex;
                --InputMap->LastIndex;
                FL_IndexState[kIndex] = FL_IndexState[lastIndex];
                InputMap->InputCode2Index[ Index2InputCode(lastIndex) ] = kIndex;
                for (int kCode = 0; kCode < NUM_KEYS; ++kCode)
                    if (KeyCode2Index(kCode) == lastIndex)
                        InputMap->KeyCode2Index[kCode] = kIndex;
            }
        }

        // the index is still occupied, we may zero kIndex and proceed normal path
        kIndex = 0;
    }

    if (!iIndex)
    {
        ++InputMap->LastIndex;
        InputMap->KeyCode2Index[kCode]     = InputMap->LastIndex;
        InputMap->InputCode2Index[iCode]   = InputMap->LastIndex;
        FL_IndexState[InputMap->LastIndex] = 0;
        return;
    }

    InputMap->KeyCode2Index[kCode] = iIndex;
}

void InputMgr :: LoadMap(const char *fileName)
{
    assert(I_CodeCount);

    std::ifstream in;

    in.open(fileName);
    if (in.is_open())
    {
        ClearMappings();

        char buff[255];
        while (in.good())
        {
            in.getline(buff, 255);
            if (buff[0] == 0 || buff[0] == '#') continue;
            size_t len = strlen(buff);
            if (buff[len - 1] == '\r') buff[len - 1] = 0;

            if (buff[0] == '[')
            {
                InputMap = &ScenesMap[buff];
                if (!InputMap->InputCode2Index) InputMap->Create(I_CodeCount);
            }
            else
            {
                if (InputMap)
                {
                    const char *params = buff;
                    int kCode = GetKeyCode(ReadSubstring(buff, params));
                    if (!kCode) continue;
                    int iCode;
                    sscanf(params, "%d", &iCode);
                    Key2InputCode_Set(kCode, iCode);
                }
            }
        }
        in.close();
    }
}

void InputMgr :: SaveMap(const char *fileName)
{
    std::ofstream out;

    out.open(fileName);
    if (out.is_open())
    {
        TScenesMap::iterator iter;
        for (iter = ScenesMap.begin(); iter != ScenesMap.end(); ++iter)
        {
            out << iter->first << '\n';
            InputMap = &iter->second;

            for (int kCode = 0; kCode < NUM_KEYS; ++kCode)
            {
                int index = KeyCode2Index(kCode);
                if (index)
                {
                    int iCode = Index2InputCode(index);
                    out << GetKeyName(kCode) << '\t' << iCode << '\n';
                }
            }
        }
        out.close();
    }
}

void InputMgr :: LoadKeyCodeMap(const char *fileName)
{
    std::ifstream in;

    in.open(fileName);
    if (in.is_open())
    {
        KeyCodeNameMap.clear();

        char buff[255];
        int keyCode;
        char keyName[255];

        while (in.good())
        {
            in.getline(buff, 255);
            if (buff[0] == '\0' || buff[0] == '#') continue;

            sscanf(buff, "%d %s", &keyCode, keyName);
            KeyCodeNameMap[keyCode] = keyName;
        }
        in.close();
    }
}
