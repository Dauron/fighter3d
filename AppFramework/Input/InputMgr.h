#ifndef __incl_InputMgr_h
#define __incl_InputMgr_h

#include <string>
#include <map>
#include "../../Utils/Singleton.h"
#include "InputCodes.h"
#include "VirtualKeys.h"

#define BUFFER_LENGTH 256
#define NUM_KEYS      256
#define IC_Undefined    0

#define g_InputMgr InputMgr::GetSingleton()

class InputMgr : public Singleton<InputMgr>
{
    typedef unsigned char byte;
    typedef std::map<byte, std::string> TKeyCodeMap;
    struct TInputMap
    {
        int  KeyCode2Index[NUM_KEYS];
        int *InputCode2Index;
        int  Indices;

        TInputMap  () : InputCode2Index(NULL) {}
        ~TInputMap () { Destroy(); }

        void Create(int iCodeCount)
        {
            assert( !InputCode2Index );
            InputCode2Index = new int[iCodeCount];
            memset (KeyCode2Index,   0, sizeof(int)*NUM_KEYS);
            memset (InputCode2Index, 0, sizeof(int)*iCodeCount);
            Indices = 0;
        }
        void Destroy()
        { if (InputCode2Index) { delete InputCode2Index; InputCode2Index = NULL; }; }
    };
    typedef std::map<std::string, TInputMap> TScenesMap;

    TKeyCodeMap _KeyCodeMap;
    TInputMap*  _InputMap;
    TScenesMap  _ScenesMap;

    int         _iCodeCount;
    bool        _KeysState [256];
    bool       *_IndexState;//[IC_CODE_COUNT];

    int KeyCode2Index(byte kCode)
    {
        assert (_InputMap);
        return _InputMap->KeyCode2Index[kCode];
    }
    int InputCode2Index(int iCode)
    {
        assert (_InputMap);
        return _InputMap->InputCode2Index[iCode];
    }
    int Index2InputCode(int index)
    {
        assert (_InputMap);
        for (int iCode = 0; iCode < _iCodeCount; ++iCode)
            if ( InputCode2Index(iCode) == index)
                return iCode;
        return IC_Undefined;
    }
    int Index2FirstKeyCode(int index)
    {
        assert (_InputMap);
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
        if (charCode == 8 || charCode == 13 || charCode == 27)
            return;
        if (Buffer.length() == BUFFER_LENGTH)
            Buffer.clear();
        Buffer += charCode;
    }
    // Set given key state
    void SetKeyState(byte kCode, bool state) {
        bool &kState = _KeysState[kCode];
        if ((kState && state) || (!kState && !state))
            return;
        kState = state;

        if (!_InputMap) return;
        _IndexState[ KeyCode2Index(kCode) ] = state;
    }
    // Get given key state
    bool GetKeyState(byte kCode)             { return _InputMap && _KeysState[ kCode ]; }
    // Set given input state
    void SetInputState(int iCode, bool state) { if (!_InputMap) return; _IndexState[ InputCode2Index(iCode) ] = state; }
    // Get given input state
    bool GetInputState(int iCode)             { return _InputMap && FL_enable && _IndexState[ InputCode2Index(iCode) ]; }
    bool GetInputStateAndClear(int iCode)
    {
        if (!_InputMap || !FL_enable) return false;
        int  index = InputCode2Index(iCode);
        bool state = _IndexState[ index ];
        _IndexState[ index ] = false;
        return state;
    }

    void SetScene(const char *scene, bool FL_dont_process_buttons = true)
    {
        TInputMap *map = &_ScenesMap[scene];
        if (_InputMap == map) return;
        _InputMap = map;
        if (!map->InputCode2Index) map->Create(_iCodeCount);

        FL_enable = true;
        Buffer.clear();
        memset(_IndexState, 0, _InputMap->Indices * sizeof(bool));

        if (FL_dont_process_buttons) return;

        for (int kCode = 0; kCode < NUM_KEYS; ++kCode)
            if (_KeysState[kCode])
                _IndexState[ KeyCode2Index(kCode) ] = true;
    }
    void AllKeysUp()
    {
        memset(g_InputMgr._KeysState, 0, NUM_KEYS * sizeof(bool));
        memset(_IndexState, 0, _InputMap->Indices * sizeof(bool));
    }

    // Get / Set the keyCode to inputCode mapping
    int Key2InputCode(byte kCode)
    {
        return Index2InputCode( KeyCode2Index(kCode) );
    }
    byte Input2KeyCode(int iCode)
    {
        return Index2FirstKeyCode( InputCode2Index(iCode) );
    }
    void Key2InputCode_Set(byte kCode, int iCode)
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
                    _IndexState[_InputMap->Indices]   = false;
                    return;
                }
                // we already have our own index, we must free the unused one
                if (iIndex)
                {
                    _InputMap->KeyCode2Index[kCode] = iIndex;

                    int lastIndex = --_InputMap->Indices;
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
            _InputMap->KeyCode2Index[kCode]   = _InputMap->Indices;
            _InputMap->InputCode2Index[iCode] = _InputMap->Indices;
            _IndexState[_InputMap->Indices]   = false;
            ++_InputMap->Indices;
            return;
        }

        _InputMap->KeyCode2Index[kCode] = iIndex;
    }
    void Key2InputCode_SetIfKeyFree(byte kCode, int iCode)
    {
        int kIndex = KeyCode2Index(kCode);
        if (kIndex) return;
        Key2InputCode_Set(kCode, iCode);
    }
    void ClearMap()
    {
        _InputMap = NULL;
        _ScenesMap.clear();
        _KeyCodeMap.clear();
    }
    void LoadMap(const char *fileName);
    void SaveMap(const char *fileName);

    std::string GetKeyName(int kCode)
    {
        std::map<byte, std::string>::iterator iter = _KeyCodeMap.find(kCode);
        if (iter != _KeyCodeMap.end())
            return iter->second;

        std::string res;
        res += kCode;
        res += '\0';
        return res;
    }
    void LoadKeyCodeMap(const char *fileName);

    static void Create(int iCodeCount)
    {
        Singleton::Create();

        g_InputMgr.FL_enable = true;
        g_InputMgr._InputMap = NULL;
        g_InputMgr.Buffer.reserve(BUFFER_LENGTH);
        g_InputMgr.LoadMap("Data/keyboard.txt");
        g_InputMgr.LoadKeyCodeMap("Data/keys.txt");

        g_InputMgr._iCodeCount = iCodeCount;
        g_InputMgr._IndexState = new bool[iCodeCount];
        memset(g_InputMgr._KeysState,  0, NUM_KEYS * sizeof(bool));
    }

    InputMgr()
    { _IndexState = NULL; }
    ~InputMgr()
    { if (_IndexState) { delete[] _IndexState; _IndexState = NULL; } }
};

#endif
