#ifndef __incl_Math_Tracking_ObjectTracker_h
#define __incl_Math_Tracking_ObjectTracker_h

#include "TrackedObject.h"
#include <vector>
#include <string>

#include "../../Lua/LuaMgr.h"
#include "../../Lua/LuaScript.h"

namespace Math { namespace Tracking {
    using namespace ::Math::Tracking;

    typedef std::vector<TrackedObject*> Vec_TrackedObject;

    struct TrackingSet
    {
        Vec_TrackedObject L_objects;
    };

    struct ObjectTracker;

    typedef void (*TrackingScript)(ObjectTracker &tracker, LuaScriptData *data);

    struct ObjectTracker : private CLuaCall<ObjectTracker>
    {
        enum TrackingMode {
            TRACK_NOTHING,
            TRACK_OBJECT,
            TRACK_SUBOBJECT,
            TRACK_ALL_CENTER,
            TRACK_CUSTOM_SCRIPT
        };

        TrackingMode   Mode;
        std::string    ScriptName;
        TrackingScript Script;
        LuaScriptData *ScriptData;

        TrackingSet  *Targets;
        xBYTE         ID_object;
        xBYTE         ID_subobject;
        xVector3      NW_destination_shift;

        xPoint3       P_destination;

        ObjectTracker() : CLuaCall<ObjectTracker>(g_LuaMgr.vm) {}

        void Init(bool FL_init_empty = true)
        {
            Targets = (FL_init_empty) ? NULL : new TrackingSet();
            Mode    = TRACK_NOTHING;
            ScriptName.clear();
            Script     = NULL;
            ScriptData = NULL;
            P_destination.zero();
            NW_destination_shift.zero();
            ID_object = ID_subobject = 0;
        }

        void UpdateDestination();
        void InterpolatePosition(xPoint3 &P_current,
                                 const xPoint3 &P_center,
                                 xFLOAT W_progress);

        void Free()
        {
            if (Targets) { delete Targets; Targets = NULL; }
        }

        void HandleReturns (CLuaVirtualMachine& vm, const char *strFunc)
        {
        }

        void InitScript(std::string name, std::string path)
        {
            CompileFile (path.c_str());
            ScriptName = name;

            RegisterFunction (&ObjectTracker::lua_Count,        "count");
            RegisterFunction (&ObjectTracker::lua_GetFirst,     "getFirst");
            RegisterFunction (&ObjectTracker::lua_GetNext,      "getNext");
            RegisterFunction (&ObjectTracker::lua_GetCurrent,   "getCurrent");
            RegisterFunction (&ObjectTracker::lua_GetTarget,    "getTarget");
            RegisterFunction (&ObjectTracker::lua_GetSubObject, "getSubobject");
        }

        void CallScript()
        {
            lua_State *state = (lua_State *) vm();

            lua_rawgeti (state, LUA_REGISTRYINDEX, m_iThisRef);

            lua_pushPoint  (state, P_destination);
            lua_setfield   (state, -2, "P_destination");

            lua_pushinteger(state, (lua_Integer) ID_object);
            lua_setfield   (state, -2, "ID_object");
            lua_pushinteger(state, (lua_Integer) ID_subobject);
            lua_setfield   (state, -2, "ID_subobject");

            lua_pushPoint  (state, NW_destination_shift);
            lua_setfield   (state, -2, "NW_destination_shift");

            lua_pop(state, 1);

            SelectScriptFunction (ScriptName.c_str());
            if (ScriptData)
                PopParams(ScriptData->PushParams(state));
            Go();

            lua_rawgeti (state, LUA_REGISTRYINDEX, m_iThisRef);
            lua_getfield(state, -1, "P_destination");
            lua_readPoint(state, P_destination);
            lua_pop(state, 2);
        }

    private:
        int Iter;

        int lua_Count (CLuaVirtualMachine& vm)
        {
            lua_pushinteger ((lua_State *) vm, (lua_Integer)Targets->L_objects.size());
            return 1;
        }

        int lua_GetCurrent(CLuaVirtualMachine& vm)
        {
            lua_State *state = (lua_State *) vm;

            if (Iter > -1 && Iter < int(Targets->L_objects.size()))
            {
                TrackedObject &target = *Targets->L_objects[Iter];

                lua_createtable(state, 2, 0);

                lua_pushPoint  (state, target.P_center_Trfm);
                lua_setfield   (state, -2, "P_center_Trfm");

                lua_pushnumber (state, (lua_Number) target.S_radius);
                lua_setfield   (state, -2, "S_radius");
            }
            else
                lua_pushnil(state);

            return 1;
        }

        int lua_GetTarget(CLuaVirtualMachine& vm)
        {
            Iter = (int)lua_tointeger((lua_State *) vm, 1);
            return lua_GetCurrent(vm);
        }

        int lua_GetSubObject(CLuaVirtualMachine& vm)
        {
            lua_State *state = (lua_State *) vm;

            if (Iter > -1 && Iter < int(Targets->L_objects.size()))
            {
                int subObject = (int)lua_tointeger((lua_State *) vm, 1);

                TrackedObject &target = *Targets->L_objects[Iter];
                TrackedObject &subTarget = /*target;//*/target.GetSubObject(subObject);

                lua_createtable(state, 2, 0);

                lua_pushPoint  (state, subTarget.P_center_Trfm);
                lua_setfield   (state, -2, "P_center_Trfm");

                lua_pushnumber (state, (lua_Number) subTarget.S_radius);
                lua_setfield   (state, -2, "S_radius");
            }
            else
                lua_pushnil(state);

            return 1;
        }

        int lua_GetFirst(CLuaVirtualMachine& vm)
        {
            Iter = 0;
            return lua_GetCurrent(vm);
        }

        int lua_GetNext(CLuaVirtualMachine& vm)
        {
            if (Iter > -1) ++Iter;
            return lua_GetCurrent(vm);
        }
    };

} } // namespace Math.Tracking

#endif
