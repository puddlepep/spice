#include "Shape.hpp"

namespace Spice
{

	Shape::Shape(std::vector<glm::vec2> points)
	{
		m_VBuffer = new VertexBuffer(&points[0], points.size() * 2 * sizeof(float));
		m_VBuffer->Unbind();

		m_Points = points;
	}

	Shape::~Shape()
	{
		delete m_VBuffer;
	}

	void Shape::Reform(std::vector<glm::vec2> points)
	{
		m_VBuffer->NewData(&points[0], points.size() * 2 * sizeof(float));
		m_VBuffer->Unbind();

		m_Points = points;
	}

	int Shape::lua_GetPoints(lua_State* L)
	{
		Shape* shape = (Shape*)lua_touserdata(L, 1);
		
		lua_newtable(L);
		for (int i = 0; i < shape->GetPoints().size(); i++)
		{
			glm::vec2 p = shape->GetPoints()[i];

			//LuaVector::lua_New(L, p);
			lua_rawseti(L, -2, i + 1);
		}

		return 1;
	}

	int Shape::lua_Reform(lua_State* L)
	{
		int argAmt = lua_gettop(L);

		if (argAmt > 1)
		{
			Shape* shape = (Shape*)lua_touserdata(L, 1);
			std::vector<glm::vec2> points;
			int table = 2;

			if (lua_type(L, table) == LUA_TTABLE)
			{
				lua_pushnil(L);
				while (lua_next(L, table) != 0) {
					/* uses 'key' (at index -2) and 'value' (at index -1) */

					int vtype = lua_type(L, -1);
					if (vtype == LUA_TUSERDATA)
					{
						glm::vec2* vec = (glm::vec2*) lua_touserdata(L, -1);
						points.push_back(*vec);
					}
					else
					{
						LuaThrowSyntaxError(L, "Shape.Reform(shape, points)'s second parameter must be a table of Vec2 points!");
						return 0;
					}

					/* removes 'value'; keeps 'key' for next iteration */
					lua_pop(L, 1);
				}
				lua_pop(L, 1);

				if (points.size() == 0)
					return 0;

				shape->Reform(points);

				return 1;
			}
			else
			{
				LuaThrowSyntaxError(L, "Shape.Reform(shape, points)'s second parameter must be a table of Vec2 points!");
				return 0;
			}
		}
		else
		{
			LuaThrowSyntaxError(L, "Shape.Reform(shape, points) requires 2 parameters!");
			return 0;
		}
	}

	int Shape::lua_Shape(lua_State* L)
	{
		int argAmt = lua_gettop(L);

		if (argAmt > 0)
		{
			std::vector<glm::vec2> points;
			int table = 1;

			if (lua_type(L, table) == LUA_TTABLE)
			{
				lua_pushnil(L);
				while (lua_next(L, table) != 0) {
					/* uses 'key' (at index -2) and 'value' (at index -1) */

					int vtype = lua_type(L, -1);
					if (vtype == LUA_TUSERDATA)
					{
						glm::vec2* vec = (glm::vec2*) lua_touserdata(L, -1);
						points.push_back(*vec);
					}
					else
					{
						LuaThrowSyntaxError(L, "Shape.new(points)'s only parameter must be a table of Vec2 points!");
						return 0;
					}

					/* removes 'value'; keeps 'key' for next iteration */
					lua_pop(L, 1);
				}
				lua_pop(L, 1);

				if (points.size() == 0)
					return 0;

				Shape* shape = (Shape*)lua_newuserdata(L, sizeof(Shape));
				new (shape) Shape(points);

				luaL_getmetatable(L, "ShapeMTT");
				lua_setmetatable(L, -2);

				return 1;
			}
			else
			{
				LuaThrowSyntaxError(L, "Shape.new(points)'s only parameter must be a table of Vec2 points!");
				return 0;
			}
		}
		else
		{
			LuaThrowSyntaxError(L, "Shape.new(points) requires 1 parameter!");
			return 0;
		}
	}

	int Shape::lua_Index(lua_State* L)
	{
		Shape* shape = (Shape*)lua_touserdata(L, 1);
		std::string index = lua_tostring(L, 2);

		lua_getglobal(L, "Shape");
		lua_pushstring(L, index.c_str());
		lua_rawget(L, -2);

		return 1;
	}

	int Shape::lua_ToString(lua_State* L)
	{
		Shape* shape = (Shape*)lua_touserdata(L, 1);
		std::string str = "Shape, points: " + std::to_string(shape->GetPoints().size());

		lua_pushstring(L, str.c_str());
		return 1;
	}

	void Shape::lua_RegisterShape(lua_State* L)
	{
		static const luaL_Reg ShapeReg[] =
		{
			{"new", lua_Shape},
			{"Reform", lua_Reform},
			{"GetPoints", lua_GetPoints},
			{NULL, NULL}
		};

		luaL_newlib(L, ShapeReg);
		lua_setglobal(L, "Shape");

		luaL_newmetatable(L, "ShapeMTT");

		lua_pushstring(L, "__index");
		lua_pushcfunction(L, lua_Index);
		lua_settable(L, -3);

		lua_pushstring(L, "__tostring");
		lua_pushcfunction(L, lua_ToString);
		lua_settable(L, -3);
	}
}
