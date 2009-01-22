// ---------------------------------------------------------------------------
// FILE NAME            : LuaScript.h
// ---------------------------------------------------------------------------
// DESCRIPTION :
//
// Scripting base class
//
// ---------------------------------------------------------------------------
// VERSION              : 1.00
// DATE                 : 1-Sep-2005
// AUTHOR               : Richard Shephard
// ---------------------------------------------------------------------------
// LIBRARY INCLUDE FILES

#ifndef __LUA_SCRIPT_BASE_H__
#define __LUA_SCRIPT_BASE_H__

#include "luainc.h"
#include "LuaVirtualMachine.h"

class CLuaScript
{
public:
   CLuaScript (CLuaVirtualMachine& vm);
   virtual ~CLuaScript (void);

   // Compile script into Virtual Machine
   bool CompileFile (const char *strFilename);
   bool CompileBuffer (unsigned char *pbBuffer, size_t szLen);

   // Register function with Lua
   int RegisterFunction (const char *strFuncName);

   // Selects a Lua Script function to call
   bool SelectScriptFunction (const char *strFuncName);
   void PopParams(int num);
   void AddParam (int iInt);
   void AddParam (float fFloat);
   void AddParam (char *string);

   // Runs the loaded script
   bool Go (int nReturns = 0);

   // Checks on Virtual Machine script
   bool ScriptHasFunction (const char *strScriptName);

   // Method indexing check
   int methods (void) { return m_nMethods; }

   // When the script calls a class method, this is called
   virtual int ScriptCalling (CLuaVirtualMachine& vm, int iFunctionNumber) = 0;

   // When the script function has returns
   virtual void HandleReturns (CLuaVirtualMachine& vm, const char *strFunc) = 0;

   CLuaVirtualMachine& vm (void) { return m_vm; }

protected:
   int m_nMethods;
   CLuaVirtualMachine& m_vm;
   int m_iThisRef;
   int m_nArgs;
   const char *m_strFunctionName;
};

#include <vector>

template <typename T>
class CLuaCall : public CLuaScript
{
public:
   CLuaCall (CLuaVirtualMachine& vm) : CLuaScript(vm) {}

   typedef int (T::*lua_CMethod)(CLuaVirtualMachine &vm);

   // When the script calls a class method, this is called
   virtual int ScriptCalling (CLuaVirtualMachine& vm, int iFunctionNumber)
   {
       //assert(iFunctionNumber < methods());
       lua_CMethod f = reg_methods[iFunctionNumber-1];
       return (((T*)this)->*f)(vm);
   }

   // Register function with Lua
   int RegisterFunction (lua_CMethod method, const char *strFuncName)
   {
       reg_methods.push_back(method);
       return CLuaScript::RegisterFunction(strFuncName);
   }

private:
    std::vector<lua_CMethod> reg_methods;
};

#endif // __LUA_SCRIPT_BASE_H__
