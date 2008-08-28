#ifndef __incl_InputMgr_h
#define __incl_InputMgr_h

#include <string>
#include <map>
#include "../../Utils/Singleton.h"
#include "InputCodes.h"
#include "VirtualKeys.h"

#define BUFFER_LENGTH 256

#define g_InputMgr InputMgr::GetSingleton()

class InputMgr : public Singleton<InputMgr>
{
    typedef std::map<int, std::string> TKeyCodeMap;
    typedef std::map<int, int> TInputMap;
    typedef std::map<std::string, TInputMap> TScenesMap;

    TKeyCodeMap _KeyCodeMap;
    TInputMap*  _InputMap;
    TScenesMap  _ScenesMap;
    bool        _KeysState[IC_LAST_CODE+1];

public:
    std::string Buffer;

    int mouseX;
    int mouseY;
    int mouseWheel;
    bool enable;

    // Append character to buffer
    void AppendBuffer(unsigned char charCode)
    {
        if (charCode == 8 || charCode == 13 || charCode == 27)
            return;
        if (Buffer.length() == BUFFER_LENGTH)
            Buffer.clear();
        Buffer += charCode;
    }
    // Set given key state
    void SetKeyState(int keyCode, bool state)     { SetInputState(GetInputCode(keyCode), state); }
    // Set given input state
    void SetInputState(int inputCode, bool state) { _KeysState[inputCode] = state; }
    // Get given input state
    bool GetInputState(int inputCode)             { return enable && _KeysState[inputCode]; }
    bool GetInputStateAndClear(int inputCode)
    {
        if (!enable) return false;
        bool state = _KeysState[inputCode];
        _KeysState[inputCode] = false;
        return state;
    }

    void SetScene(const char *scene)
    {
        _InputMap = &_ScenesMap[scene];
        memset(_KeysState, 0, IC_LAST_CODE+1);
    }

    // Get / Set the keyCode to inputCode mapping
    int GetInputCode(int keyCode)
    {
        if (_InputMap == NULL) return IC_Undefined;

        std::map<int, int>::iterator iter = _InputMap->find(keyCode);
        if (iter != _InputMap->end())
            return iter->second;
        return IC_Undefined;
    }
    int GetKeyCode(int inputCode)
    {
        if (_InputMap == NULL) return 0;

        std::map<int, int>::iterator iter = _InputMap->begin();
        for (; iter != _InputMap->end(); ++iter)
            if (iter->second == inputCode)
                return iter->first;
        return 0;
    }
    void SetInputCode(int keyCode, int inputCode)
    {
        if (_InputMap) (*_InputMap)[keyCode] = inputCode;
    }
    void SetInputCodeIfKeyIsFree(int keyCode, int inputCode)
    {
        if (GetInputCode(keyCode) != IC_Undefined)  return;
        if (_InputMap) (*_InputMap)[keyCode] = inputCode;
    }
    void ClearMap()
    {
        _InputMap = NULL;
        _ScenesMap.clear();
        _KeyCodeMap.clear();
    }
    void LoadMap(const char *fileName);
    void SaveMap(const char *fileName);

    std::string GetKeyName(int keyCode)
    {
        std::map<int, std::string>::iterator iter = _KeyCodeMap.find(keyCode);
        if (iter != _KeyCodeMap.end())
            return iter->second;

        std::string res;
        res += keyCode;
        res += '\0';
        return res;
    }
    void LoadKeyCodeMap(const char *fileName);

    InputMgr()
    {
        enable = true;
        _InputMap = NULL;
        Buffer.reserve(BUFFER_LENGTH);
        LoadMap("Data/keyboard.txt");
        LoadKeyCodeMap("Data/keys.txt");
    }
};

#endif
