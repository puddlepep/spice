#pragma once
#include "dep/lua544/include/lua.hpp"
#include <sstream>

#include "Color.hpp"
#include <iostream>
#include <filesystem>

bool TryLua(lua_State* L, int _result, std::string _source = "");

lua_Debug LuaGetDebugInfo(lua_State* L);
void LuaThrowSyntaxError(lua_State* L, std::string _error);

namespace Spice
{
	class Engine;
	class Object;

	void RegisterScript(lua_State* L, Object* _object);

	class Script
	{

	private:
		bool m_RunBefore = false;
		bool m_ScriptErrored = false;
		std::string m_Filepath = "";

	public:

		int FuncProcess = 0;
		int FuncUI      = 0;
		int FuncClose   = 0;

		bool HasEnginePtr = false;

		Script(Object* _parent);

		Object* Parent = nullptr;
		std::string* Filepath = new std::string("");
		bool* Active = new bool(true);

		lua_State* L = nullptr;

		bool LoadScript(std::string _filepath);
		bool Run();
		bool RunClose();

		bool Reload();

	};

}
