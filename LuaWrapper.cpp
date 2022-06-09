#include "LuaWrapper.hpp"

bool TryLua(lua_State* L, int _result, std::string _source)
{
	if (_result != LUA_OK)
	{
		std::string error = lua_tostring(L, -1);

		if (_source == "")
			std::cout << "[LUA] Error: " << error << std::endl;
		else
			std::cout << "[LUA] Error at " << _source << ": " << error << std::endl;

		return false;
	}

	return true;
}

lua_Debug LuaGetDebugInfo(lua_State* L)
{
	lua_Debug info;
	lua_getstack(L, 1, &info);
	lua_getinfo(L, "nSl", &info);

	return info;
}

void LuaThrowSyntaxError(lua_State* L, std::string _error)
{
	lua_Debug info = LuaGetDebugInfo(L);
	std::stringstream ss;
	ss << "Syntax Error at " << info.source << ":" << info.currentline << ": " << _error << "\n";

	lua_pushstring(L, ss.str().c_str());
	lua_error(L);
}

namespace Spice
{

	Script::Script(Object* _parent)
	{
		Parent = _parent;
	}

	bool Script::LoadScript(std::string _filepath)
	{
		m_RunBefore = false;
		HasEnginePtr = false;
		m_ScriptErrored = false;

		FuncProcess = 0;
		FuncUI = 0;
		FuncClose = 0;

		if (L != nullptr)
			lua_close(L);

		L = nullptr;

		if (std::filesystem::exists(_filepath))
		{
			m_Filepath = _filepath;

			L = luaL_newstate();
			RegisterScript(L, Parent);
			
			int result = luaL_loadfile(L, _filepath.c_str());
			if (result == LUA_ERRFILE)
			{
				m_ScriptErrored = true;
				return false;
			}

			if (result != LUA_OK)
			{
				std::string error = lua_tostring(L, -1);
				std::cout << "[LUA] Error: " << error << std::endl;
				lua_pop(L, 1);
				
				m_ScriptErrored = true;
			}

			return true;
		}
		else
		{
			m_ScriptErrored = true;
			return false;
		}
	}

	bool Script::Run()
	{
		if (L == nullptr || m_ScriptErrored) return false;

		if (!m_RunBefore)
		{
			m_RunBefore = true;

			if (TryLua(L, lua_pcall(L, 0, 0, 0), *Filepath))
			{

				lua_getglobal(L, "_process");
				FuncProcess = (!lua_isnil(L, -1)) ? lua_gettop(L) : 0;

				lua_getglobal(L, "_ui");
				FuncUI = (!lua_isnil(L, -1)) ? lua_gettop(L) : 0;

				lua_getglobal(L, "_close");
				FuncClose = (!lua_isnil(L, -1)) ? lua_gettop(L) : 0;

			}
			else
			{
				m_ScriptErrored = true;
				m_RunBefore = false;
				return false;
			}
		}

		if (FuncProcess != 0)
		{
			lua_pushvalue(L, FuncProcess);
			if (!TryLua(L, lua_pcall(L, 0, 0, 0))) m_ScriptErrored = true;
		}

		if (FuncUI != 0)
		{
			lua_pushvalue(L, FuncUI);
			if (!TryLua(L, lua_pcall(L, 0, 0, 0))) m_ScriptErrored = true;
		}

		return true;

	}

	bool Script::RunClose()
	{
		if (L == nullptr || m_ScriptErrored) return false;

		m_RunBefore = false;

		if (FuncClose != 0)
		{
			lua_pushvalue(L, FuncClose);
			if (!TryLua(L, lua_pcall(L, 0, 0, 0))) m_ScriptErrored = true;
		}

		return Reload();
	}

	bool Script::Reload()
	{
		if (L == nullptr) return false;

		return LoadScript(m_Filepath);
	}

}
