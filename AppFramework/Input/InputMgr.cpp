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

        _InputMap->KeyCode2Index[kCode] = 0;         // remove this kCode from search results
        byte kIndexOld = Index2FirstKeyCode(kIndex); // were there more mappings for this old Input Code?
        int  iCodeOld  = Index2InputCode(kIndex);    // get old mapped Input Code
        assert (iCodeOld && iCode != iCodeOld);

        // it was the only keyCode->oldInputCode mapping
        if (!kIndexOld)
        {
            // so we may reuse its index
            if (!iIndex)
            {
                _InputMap->KeyCode2Index[kCode]   = kIndex;
                _InputMap->InputCode2Index[iCode] = kIndex;
                _IndexState[kIndex]   = false;
                return;
            }
            // we already have our own index, we must free the unused one
            if (iIndex)
            {
                _InputMap->KeyCode2Index[kCode] = iIndex;

                int lastIndex = _InputMap->LastIndex;
                --_InputMap->LastIndex;
                _IndexState[kIndex] = _IndexState[lastIndex];
                _InputMap->InputCode2Index[ Index2InputCode(lastIndex) ] = kIndex;
                for (int kCode = 0; kCode < NUM_KEYS; ++kCode)
                    if (KeyCode2Index(kCode) == lastIndex)
                        _InputMap->KeyCode2Index[kCode] = kIndex;
            }
        }

        // the index is still occupied, we may zero kIndex and proceed normal path
        kIndex = 0;
    }

    if (!iIndex)
    {
        ++_InputMap->LastIndex;
        _InputMap->KeyCode2Index[kCode]   = _InputMap->LastIndex;
        _InputMap->InputCode2Index[iCode] = _InputMap->LastIndex;
        _IndexState[_InputMap->LastIndex]   = false;
        return;
    }

    _InputMap->KeyCode2Index[kCode] = iIndex;
}

void InputMgr :: LoadMap(const char *fileName)
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

void InputMgr :: SaveMap(const char *fileName)
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

void InputMgr :: LoadKeyCodeMap(const char *fileName)
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
