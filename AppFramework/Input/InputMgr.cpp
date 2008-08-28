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
                _InputMap = &_ScenesMap[buff];
            else
            {
                if (_InputMap)
                {
                    int keyCode;
                    int inputCode;

                    sscanf(buff, "%d\t%d", &keyCode, &inputCode);
                    (*_InputMap)[keyCode] = inputCode;
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

            TInputMap::iterator iter2;
            for (iter2 = iter->second.begin(); iter2 != iter->second.end(); ++iter2)
                out << iter2->first << '\t' << iter2->second << '\n';
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
