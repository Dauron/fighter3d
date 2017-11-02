#ifndef __incl_InputMgr_h
#define __incl_InputMgr_h

#include <string>
#include <map>
#include "../../Utils/Utils.h"
#include "../../Utils/Singleton.h"
#include "VirtualKeys.h"

#define BUFFER_LENGTH 256
#define NUM_KEYS      256
#define IC_Undefined    0

#define g_InputMgr InputMgr::GetSingleton()

typedef unsigned char byte;

class InputMgr : public Singleton<InputMgr>
{
    typedef std::map<byte, std::string> TKeyCodeNameMap;
    struct TKeyInputMap
    {
        int  KeyCode2Index[NUM_KEYS];
        int *InputCode2Index;
        int  LastIndex;

        TKeyInputMap  () : InputCode2Index(NULL) {}
        ~TKeyInputMap () { Destroy(); }

        void Create(int iCodeCount)
        {
            assert( !InputCode2Index );
            InputCode2Index = new int[iCodeCount];
            memset (KeyCode2Index,   0, sizeof(int)*NUM_KEYS);
            memset (InputCode2Index, 0, sizeof(int)*iCodeCount);
            LastIndex = 0;
        }
        void Destroy()
        { if (InputCode2Index) { delete InputCode2Index; InputCode2Index = NULL; }; }
    };
    typedef std::map<std::string, TKeyInputMap> TScenesMap;

    TKeyCodeNameMap KeyCodeNameMap;
    TKeyInputMap*   InputMap;
    TScenesMap      ScenesMap;

    int         I_CodeCount;
    bool        FL_KeysState [256];
    byte       *FL_IndexState;//[IC_CODE_COUNT];

    int KeyCode2Index(byte kCode)
    {
        assert (InputMap);
        return InputMap->KeyCode2Index[kCode];
    }
    int InputCode2Index(int iCode)
    {
        assert (InputMap);
        return InputMap->InputCode2Index[iCode];
    }
    int Index2InputCode(int index)
    {
        for (int iCode = 0; iCode < I_CodeCount; ++iCode)
            if ( InputCode2Index(iCode) == index)
                return iCode;
        return IC_Undefined;
    }
    int Index2FirstKeyCode(int index)
    {
        for (int kCode = 0; kCode < NUM_KEYS; ++kCode)
            if ( KeyCode2Index(kCode) == index)
                return kCode;
        return 0;
    }

public:
    std::string Buffer;

    int  mouseX;
    int  mouseY;
    int  mouseWheel;
    bool FL_enable;

    // Append character to buffer
    void AppendBuffer(byte charCode)
    {
        if (charCode == 13 || charCode == 27) // skip backspace, enter, escape
            return;
        if (charCode == 8)
        {
            if (Buffer.length())
                Buffer.resize(Buffer.length()-1);
            return;
        }
        if (Buffer.length() == BUFFER_LENGTH)
            Buffer.clear();
        Buffer += charCode;
    }

    void KeyDown_Set(byte kCode, bool down) {
        bool &kState = FL_KeysState[kCode];
        if ((kState && down) || (!kState && !down))
            return;
        kState = down;

        if (!InputMap) return;
        int index = KeyCode2Index(kCode);
        if (!index) return;
        byte &iState = FL_IndexState[ index ];
        if (down)   ++iState;
        else
        if (iState) --iState;
    }
    bool KeyDown_Get(byte kCode)              { return FL_KeysState[ kCode ]; }

    void InputDown_Set(int iCode, bool down)  {
        if (!InputMap) return;
        int index = InputCode2Index(iCode);
        if (!index) return;
        byte &iState = FL_IndexState[ index ];
        if (down && !iState) iState = 1;
        else
        if (!down)           iState = 0;
    }
    bool InputDown_Get(int iCode)
    { return InputMap && FL_enable && FL_IndexState[ InputCode2Index(iCode) ]; }
    bool InputDown_GetAndRaise(int iCode)
    {
        if (!InputMap || !FL_enable) return false;
        int index = InputCode2Index(iCode);
        if (!index) return false;
        byte &iState = FL_IndexState[ index ];
        bool  rState = iState;
        iState = 0;
        return rState;
    }

    void SetScene(const char *scene, bool FL_dont_process_buttons = true)
    {
        TKeyInputMap &map = ScenesMap[scene];
        if (InputMap == &map) return;
        InputMap = &map;
        if (!map.InputCode2Index) map.Create(I_CodeCount);

        FL_enable = true;
        Buffer.clear();
        memset(FL_IndexState, 0, (InputMap->LastIndex+1) * sizeof(byte));

        if (FL_dont_process_buttons) return;

        for (int kCode = 0; kCode < NUM_KEYS; ++kCode)
            if (FL_KeysState[kCode])
                ++FL_IndexState[ KeyCode2Index(kCode) ];
    }
    void AllKeysUp()
    {
        memset(FL_KeysState,  0, NUM_KEYS * sizeof(bool));
        if (InputMap)
            memset(FL_IndexState, 0, (InputMap->LastIndex+1) * sizeof(byte));
    }

    // Get / Set the keyCode to inputCode mapping
    int  Key2InputCode(byte kCode)
    {
        return Index2InputCode( KeyCode2Index(kCode) );
    }
    byte Input2KeyCode(int iCode)
    {
        return Index2FirstKeyCode( InputCode2Index(iCode) );
    }
    void Key2InputCode_Set(byte kCode, int iCode);
    void Key2InputCode_SetIfKeyFree(byte kCode, int iCode)
    {
        int kIndex = KeyCode2Index(kCode);
        if (kIndex) return;
        Key2InputCode_Set(kCode, iCode);
    }

    void LoadMap(const char *fileName);
    void SaveMap(const char *fileName);

    std::string GetKeyName(byte kCode)
    {
        std::map<byte, std::string>::iterator iter = KeyCodeNameMap.find(kCode);
        if (iter != KeyCodeNameMap.end())
            return iter->second;
        char c[2] = { (char)kCode, 0 };
        return c;
    }
    byte GetKeyCode(const std::string &kName)
    {
        std::map<byte, std::string>::iterator
            KC_curr = KeyCodeNameMap.begin(),
            KC_last = KeyCodeNameMap.end();
        for (; KC_curr != KC_last; ++KC_curr)
            if (KC_curr->second == kName)
                return KC_curr->first;
        if (kName.size() != 1) return 0;
        return kName[0];
    }
    void LoadKeyCodeMap(const char *fileName);

    void ClearMappings()
    {
        InputMap = NULL;
        ScenesMap.clear();
    }
    void Clear()
    {
        I_CodeCount   = 0;
        FL_IndexState = NULL;
        KeyCodeNameMap.clear();
        ClearMappings();
    }

    void Create(int iCodeCount)
    {
        FL_enable = true;
        InputMap  = NULL;
        Buffer.reserve(BUFFER_LENGTH);

        I_CodeCount   = iCodeCount;
        FL_IndexState = new byte[iCodeCount];
        memset(FL_KeysState,  0, NUM_KEYS * sizeof(bool));
    }
    void Destroy()
    {
        if (FL_IndexState) delete[] FL_IndexState;
        Clear();
    }

    InputMgr()
    { FL_IndexState = NULL; }
    ~InputMgr()
    { Destroy(); }
};

#endif
