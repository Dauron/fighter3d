#ifndef __LUA_LuaMgr_h
#define __LUA_LuaMgr_h

#include "../Utils/Singleton.h"

#include "LuaVirtualMachine.h"
#include "LuaScript.h"
#include "LuaDebugger.h"
#include "luainc.h"

#define g_LuaMgr LuaMgr::GetSingleton()

struct LuaScriptData
{
    virtual int PushParams(lua_State *state) = 0;
};

class LuaMgr : public Singleton<LuaMgr>
{
public:
    CLuaVirtualMachine vm;
    CLuaDebugger       dbg;

public:
    LuaMgr() : dbg(vm)
    {
        vm.InitialiseVM ();
        dbg.Initialize();
        dbg.SetCount (10);
    }
};

inline void lua_pushPoint(lua_State *state, xPoint3 P_point)
{
    lua_createtable(state, 3, 0);
    lua_pushnumber (state, (lua_Number) P_point.x);
    lua_setfield   (state, -2, "x");
    lua_pushnumber (state, (lua_Number) P_point.y);
    lua_setfield   (state, -2, "y");
    lua_pushnumber (state, (lua_Number) P_point.z);
    lua_setfield   (state, -2, "z");
}

inline void lua_readPoint(lua_State *state, xPoint3 &P_point)
{
    lua_getfield(state, -1, "x");
    lua_getfield(state, -2, "y");
    lua_getfield(state, -3, "z");
    P_point.x = (xFLOAT) lua_tonumber(state, -3);
    P_point.y = (xFLOAT) lua_tonumber(state, -2);
    P_point.z = (xFLOAT) lua_tonumber(state, -1);
    lua_pop(state, 3);
}

#endif
