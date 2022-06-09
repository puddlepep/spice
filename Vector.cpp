#include "Vector.hpp"
#include "glm/gtx/rotate_vector.hpp"

namespace Spice
{	

	Vector2Manager::Vector2Manager(float _x, float _y):
		VectorPtr(new glm::vec2(_x, _y)), m_KillVector(true), m_Instances(1) {}

	Vector2Manager::Vector2Manager(glm::vec2* _vector, bool _killVector):
		VectorPtr(_vector), m_KillVector(_killVector), m_Instances(1) {}

	void Vector2Manager::IncInstance()
	{
		m_Instances += 1;
	}

	void Vector2Manager::DecInstance()
	{
		m_Instances -= 1;

		if (m_Instances == 0)
		{
			if (m_KillVector) delete VectorPtr;
			delete this;
		}
	}



	LuaVector2::LuaVector2(Vector2Manager* _manager):
		Manager(_manager), Vector(_manager->VectorPtr)
	{
		Manager->IncInstance();
	}

	LuaVector2::LuaVector2(glm::vec2* _vector, bool _killVector) :
		Manager(new Vector2Manager(_vector, _killVector)), Vector(Manager->VectorPtr) {}

	LuaVector2::LuaVector2(float _x, float _y):
		Manager(new Vector2Manager(_x, _y)), Vector(Manager->VectorPtr) {}

	LuaVector2::~LuaVector2()
	{
		Manager->DecInstance();
	}



	int LuaVector2::lua_FromVec(lua_State* L, glm::vec2* _vec, bool _killVector)
	{
		LuaVector2* vector = static_cast<LuaVector2*>(lua_newuserdata(L, sizeof(LuaVector2)));
		new (vector) LuaVector2(_vec, _killVector);

		luaL_getmetatable(L, "Vec2MTT");
		lua_setmetatable(L, -2);

		return 1;
	}
	int LuaVector2::lua_FromManager(lua_State* L, Vector2Manager* _manager)
	{
		LuaVector2* vector = static_cast<LuaVector2*>(lua_newuserdata(L, sizeof(LuaVector2)));
		new (vector) LuaVector2(_manager);

		luaL_getmetatable(L, "Vec2MTT");
		lua_setmetatable(L, -2);

		return 1;
	}

	int LuaVector2::lua_New(lua_State* L)
	{
		int argAmt = lua_gettop(L);

		if (argAmt < 3)
		{
			float x = (argAmt > 0 && lua_isnumber(L, 1)) ? lua_tonumber(L, 1) : 0;
			float y = (argAmt > 1 && lua_isnumber(L, 2)) ? lua_tonumber(L, 2) : 0;

			LuaVector2* vector = static_cast<LuaVector2*>(lua_newuserdata(L, sizeof(LuaVector2)));
			new (vector) LuaVector2(x, y);

			luaL_getmetatable(L, "Vec2MTT");
			lua_setmetatable(L, -2);

			return 1;
		}
		else
		{
			LuaThrowSyntaxError(L, "Vec2(x = 0.0, y = 0.0) takes 0 - 2 parameters only!");
			return 0;
		}

	}
	int LuaVector2::lua_Rotated(lua_State* L)
	{
		if (lua_gettop(L) == 2)
		{
			if (lua_isuserdata(L, 1) && lua_isnumber(L, 2))
			{
				glm::vec2* vector = static_cast<LuaVector2*>(lua_touserdata(L, 1))->Vector;
				float angle_in_rad = lua_tonumber(L, 2);

				lua_pop(L, lua_gettop(L));

				glm::vec2 new_vec = glm::rotate(*vector, angle_in_rad);
				lua_pushnumber(L, new_vec.x);
				lua_pushnumber(L, new_vec.y);
				lua_New(L);

				return 1;
			}
			else
			{
				LuaThrowSyntaxError(L, "Vec2.Rotate(vec, angle)'s first parameter must be a Vec2, and its second must be a number!");
				return 0;
			}
		}
		else
		{
			LuaThrowSyntaxError(L, "Vec2.Rotate(vec, angle) requires 2 parameters!");
			return 0;
		}
	}
	int LuaVector2::lua_Length(lua_State* L)
	{
		if (lua_gettop(L) == 1)
		{
			if (lua_isuserdata(L, 1))
			{
				glm::vec2* vector = static_cast<LuaVector2*>(lua_touserdata(L, 1))->Vector;
				float length = sqrt((vector->x * vector->x) + (vector->y * vector->y));

				lua_pushnumber(L, length);
				return 1;
			}
			else
			{
				LuaThrowSyntaxError(L, "Vec2.Length(vec)'s first parameter must be a Vec2!");
				return 0;
			}
		}
		else
		{
			LuaThrowSyntaxError(L, "Vec2.Length(vec) requires 1 parameter!");
			return 0;
		}
	}
	int LuaVector2::lua_Normalized(lua_State* L)
	{
		if (lua_gettop(L) == 1)
		{
			if (lua_isuserdata(L, 1))
			{
				glm::vec2* vector = static_cast<LuaVector2*>(lua_touserdata(L, 1))->Vector;
				float length = sqrt((vector->x * vector->x) + (vector->y * vector->y));

				glm::vec2 normalized_vec(vector->x / length, vector->y / length);

				lua_pop(L, lua_gettop(L));

				lua_pushnumber(L, normalized_vec.x);
				lua_pushnumber(L, normalized_vec.y);
				lua_New(L);

				return 1;
			}
			else
			{
				LuaThrowSyntaxError(L, "Vec2.Normalized(vec)'s first parameter must be a Vec2!");
				return 0;
			}
		}
		else
		{
			LuaThrowSyntaxError(L, "Vec2.Normalized(vec) requires 1 parameter!");
			return 0;
		}
	}
	int LuaVector2::lua_Clone(lua_State* L)
	{
		if (lua_gettop(L) == 1)
		{
			if (lua_isuserdata(L, 1))
			{
				glm::vec2* vec = static_cast<LuaVector2*>(lua_touserdata(L, 1))->Vector;
				glm::vec2* new_vec = new glm::vec2(*vec);
				lua_FromVec(L, new_vec, true);
				return 1;
			}
		}
		else
		{
			LuaThrowSyntaxError(L, "Vec2.Clone(vec2) requires 1 parameter!");
			return 0;
		}
	}
	int LuaVector2::lua_Distance(lua_State* L)
	{
		if (lua_gettop(L) == 2)
		{
			if (lua_isuserdata(L, 1) && lua_isuserdata(L, 2))
			{
				glm::vec2* vec1 = static_cast<LuaVector2*>(lua_touserdata(L, 1))->Vector;
				glm::vec2* vec2 = static_cast<LuaVector2*>(lua_touserdata(L, 2))->Vector;

				glm::vec2 dir = *vec1 - *vec2;
				lua_pushnumber(L, sqrt((dir.x * dir.x) + (dir.y * dir.y)));
				
				return 1;
			}
			else
			{
				LuaThrowSyntaxError(L, "Vec2.Distance(vec, vec)'s parameters must be vectors!");
				return 0;
			}
		}
		else
		{
			LuaThrowSyntaxError(L, "Vec2.Distance(vec, vec) requires 2 parameters!");
			return 0;
		}
	}

	int LuaVector2::lua_Index(lua_State* L)
	{
		glm::vec2 *vector = static_cast<LuaVector2*>(lua_touserdata(L, 1))->Vector;
		std::string index = lua_tostring(L, 2);

		if (index == "x")
		{
			lua_pushnumber(L, vector->x);
			return 1;
		}
		else if (index == "y")
		{
			lua_pushnumber(L, vector->y);
			return 1;
		}
		else
		{
			lua_getglobal(L, "Vec2");
			lua_pushstring(L, index.c_str());
			lua_rawget(L, -2);

			return 1;
		}
	}
	int LuaVector2::lua_NewIndex(lua_State* L)
	{
		glm::vec2* vector = static_cast<LuaVector2*>(lua_touserdata(L, 1))->Vector;
		std::string index = lua_tostring(L, 2);

		if (lua_isnumber(L, 3))
		{
			float value = lua_tonumber(L, 3);

			if (index == "x")
				vector->x = value;

			else if (index == "y")
				vector->y = value;
		}
		else
		{
			LuaThrowSyntaxError(L, "Vec2 properties must be numbers!");
		}

		return 0;
	}
	int LuaVector2::lua_ToString(lua_State* L)
	{
		glm::vec2* vector = static_cast<LuaVector2*>(lua_touserdata(L, 1))->Vector;
		std::string str = "Vec2(" + std::to_string(vector->x) + ", " + std::to_string(vector->y) + ")";

		lua_pushstring(L, str.c_str());
		return 1;
	}
	int LuaVector2::lua_GC(lua_State* L)
	{
		LuaVector2* lua_vector = static_cast<LuaVector2*>(lua_touserdata(L, 1));
		lua_vector->~LuaVector2();

		return 0;
	}

	int LuaVector2::lua_Add(lua_State* L)
	{
		glm::vec2* v1 = static_cast<LuaVector2*>(lua_touserdata(L, 1))->Vector;
		glm::vec2 new_vector(0, 0);

		if (lua_isuserdata(L, 2))
		{
			glm::vec2* v2 = static_cast<LuaVector2*>(lua_touserdata(L, 2))->Vector;

			new_vector.x = v1->x + v2->x;
			new_vector.y = v1->y + v2->y;
		}
		else if (lua_isnumber(L, 2))
		{
			float n = lua_tonumber(L, 2);
			new_vector.x = v1->x + n;
			new_vector.y = v1->y + n;
		}

		lua_pop(L, lua_gettop(L));
		lua_pushnumber(L, new_vector.x);
		lua_pushnumber(L, new_vector.y);
		lua_New(L);
		return 1;
	}

	int LuaVector2::lua_Mul(lua_State* L)
	{
		glm::vec2* v1 = static_cast<LuaVector2*>(lua_touserdata(L, 1))->Vector;
		glm::vec2 new_vector(0, 0);

		if (lua_isuserdata(L, 2))
		{
			glm::vec2* v2 = static_cast<LuaVector2*>(lua_touserdata(L, 2))->Vector;

			new_vector.x = v1->x * v2->x;
			new_vector.y = v1->y * v2->y;
		}
		else if (lua_isnumber(L, 2))
		{
			float n = lua_tonumber(L, 2);
			new_vector.x = v1->x * n;
			new_vector.y = v1->y * n;
		}

		lua_pop(L, lua_gettop(L));
		lua_pushnumber(L, new_vector.x);
		lua_pushnumber(L, new_vector.y);
		lua_New(L);
		return 1;
	}

	int LuaVector2::lua_Sub(lua_State* L)
	{
		glm::vec2* v1 = static_cast<LuaVector2*>(lua_touserdata(L, 1))->Vector;
		glm::vec2 new_vector(0, 0);

		if (lua_isuserdata(L, 2))
		{
			glm::vec2* v2 = static_cast<LuaVector2*>(lua_touserdata(L, 2))->Vector;

			new_vector.x = v1->x - v2->x;
			new_vector.y = v1->y - v2->y;
		}
		else if (lua_isnumber(L, 2))
		{
			float n = lua_tonumber(L, 2);
			new_vector.x = v1->x - n;
			new_vector.y = v1->y - n;
		}

		lua_pop(L, lua_gettop(L));
		lua_pushnumber(L, new_vector.x);
		lua_pushnumber(L, new_vector.y);
		lua_New(L);
		return 1;
	}

	int LuaVector2::lua_Div(lua_State* L)
	{
		glm::vec2* v1 = static_cast<LuaVector2*>(lua_touserdata(L, 1))->Vector;
		glm::vec2 new_vector(0, 0);

		if (lua_isuserdata(L, 2))
		{
			glm::vec2* v2 = static_cast<LuaVector2*>(lua_touserdata(L, 2))->Vector;

			new_vector.x = v1->x / v2->x;
			new_vector.y = v1->y / v2->y;
		}
		else if (lua_isnumber(L, 2))
		{
			float n = lua_tonumber(L, 2);
			new_vector.x = v1->x / n;
			new_vector.y = v1->y / n;
		}

		lua_pop(L, lua_gettop(L));
		lua_pushnumber(L, new_vector.x);
		lua_pushnumber(L, new_vector.y);
		lua_New(L);
		return 1;
	}

	int LuaVector2::lua_Unm(lua_State* L)
	{
		glm::vec2* vector = static_cast<LuaVector2*>(lua_touserdata(L, 1))->Vector;
		glm::vec2 new_vector(-vector->x, -vector->y);

		lua_pop(L, lua_gettop(L));
		lua_pushnumber(L, new_vector.x);
		lua_pushnumber(L, new_vector.y);
		lua_New(L);
		return 1;
	}

	void LuaVector2::lua_RegisterVec2(lua_State* L)
	{
		static const luaL_Reg Vec2Reg[] =
		{
			{"new", lua_New},
			{"Rotated", lua_Rotated},
			{"Length", lua_Length},
			{"Normalized", lua_Normalized},
			{"Distance", lua_Distance},
			{"Clone", lua_Clone},
			{NULL, NULL}
		};

		luaL_newlib(L, Vec2Reg);
		lua_setglobal(L, "Vec2");

		luaL_newmetatable(L, "Vec2MTT");

		lua_pushstring(L, "__index");
		lua_pushcfunction(L, lua_Index);
		lua_settable(L, -3);

		lua_pushstring(L, "__newindex");
		lua_pushcfunction(L, lua_NewIndex);
		lua_settable(L, -3);

		lua_pushstring(L, "__tostring");
		lua_pushcfunction(L, lua_ToString);
		lua_settable(L, -3);

		lua_pushstring(L, "__add");
		lua_pushcfunction(L, lua_Add);
		lua_settable(L, -3);

		lua_pushstring(L, "__mul");
		lua_pushcfunction(L, lua_Mul);
		lua_settable(L, -3);

		lua_pushstring(L, "__sub");
		lua_pushcfunction(L, lua_Sub);
		lua_settable(L, -3);

		lua_pushstring(L, "__div");
		lua_pushcfunction(L, lua_Div);
		lua_settable(L, -3);

		lua_pushstring(L, "__unm");
		lua_pushcfunction(L, lua_Unm);
		lua_settable(L, -3);

		lua_pushstring(L, "__gc");
		lua_pushcfunction(L, lua_GC);
		lua_settable(L, -3);
	}
}
