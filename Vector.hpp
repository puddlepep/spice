#pragma once
#include "LuaWrapper.hpp"
#include "dep/glm/glm.hpp"

namespace Spice
{

	class Vector2Manager
	{
	private:
		int m_Instances;
		bool m_KillVector;

	public:

		glm::vec2* VectorPtr;

		Vector2Manager(float _x, float _y);
		Vector2Manager(glm::vec2* _vector, bool _killVector);

		void IncInstance();
		void DecInstance();

	};

	class LuaVector2
	{
	public:

		Vector2Manager* Manager;
		glm::vec2* Vector;

		LuaVector2(Vector2Manager* _manager);				// Attaches provided manager.
		LuaVector2(glm::vec2* _vector, bool _killVector);   // Creates a new manager, then attaches vec2
		LuaVector2(float _x, float _y);						// Creates a new manager AND vec2
		~LuaVector2();

		static int lua_FromVec(lua_State* L, glm::vec2* _vec, bool _killVector);
		static int lua_FromManager(lua_State* L, Vector2Manager* _manager);

		static int lua_New(lua_State* L);
		static int lua_Rotated(lua_State* L);
		static int lua_Length(lua_State* L);
		static int lua_Normalized(lua_State* L);
		static int lua_Distance(lua_State* L);
		static int lua_Clone(lua_State* L);

		static int lua_Index(lua_State* L);
		static int lua_NewIndex(lua_State* L);
		static int lua_ToString(lua_State* L);
		static int lua_GC(lua_State* L);

		static int lua_Add(lua_State* L);
		static int lua_Mul(lua_State* L);
		static int lua_Sub(lua_State* L);
		static int lua_Div(lua_State* L);
		static int lua_Unm(lua_State* L);

		static void lua_RegisterVec2(lua_State* L);

	};
}
