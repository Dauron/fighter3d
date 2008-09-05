#include <fstream>
#include "InputMgr.h"

#ifdef WIN32
#pragma warning(disable : 4996) // deprecated
#endif

void InputMgr::LoadMap(const char *fileName)
{
    std::ifstream in;

    in.open(fileName);
    if (in.is_open())
    {
        ClearMap();
        _InputMap = NULL;

        char buff[255];
        while (in.good())
        {
            in.getline(buff, 255);
            if (buff[0] == '[')
            {
                _InputMap = &_ScenesMap[buff];
                if (!_InputMap->InputCode2Index) _InputMap->Create(_iCodeCount);
            }
            else
            {
                if (_InputMap)
                {
                    int kCode;
                    int iCode;

                    sscanf(buff, "%d\t%d", &kCode, &iCode);
                    Key2InputCode_Set(kCode, iCode);
                }
            }
        }
        in.close();
    }
}

void InputMgr::SaveMap(const char *fileName)
{
    std::ofstream out;

    out.open(fileName);
    if (out.is_open())
    {
        TScenesMap::iterator iter;
        for (iter = _ScenesMap.begin(); iter != _ScenesMap.end(); ++iter)
        {
            out << iter->first << '\n';

            for (int kCode = 0; kCode < NUM_KEYS; ++kCode)
            {
                int index = KeyCode2Index(kCode);
                if (index)
                {
                    int iCode = Index2InputCode(index);
                    out << kCode << '\t' << iCode << '\n';
                }
            }
        }
        out.close();
    }
}

void InputMgr::LoadKeyCodeMap(const char *fileName)
{
    std::ifstream in;

    in.open(fileName);
    if (in.is_open())
    {
        _KeyCodeMap.clear();

        char buff[255];
        int keyCode;
        char keyName[255];

        while (in.good())
        {
            in.getline(buff, 255);
            if (buff[0] == '\0' || buff[0] == '#') continue;

            sscanf(buff, "%d\t%s", &keyCode, keyName);
            _KeyCodeMap[keyCode] = keyName;
        }
        in.close();
    }
}
