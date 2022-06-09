#include "Component.hpp"
#include "Object.hpp"

#include <iostream>

namespace Spice
{

	#pragma region Parameter

	Parameter Parameter::Deserialize(lua_State* L)
	{
		lua_pushstring(L, "datatype");
		lua_gettable(L, -2);
		DataType type = static_cast<DataType>(lua_tonumber(L, -1));
		lua_pop(L, 1);

		lua_pushstring(L, "name");
		lua_gettable(L, -2);
		std::string name = lua_tostring(L, -1);
		lua_pop(L, 1);

		lua_pushstring(L, "killdata");
		lua_gettable(L, -2);
		bool killdata = lua_toboolean(L, -1);
		lua_pop(L, 1);

		lua_pushstring(L, "data");
		lua_gettable(L, -2);

		void* data = nullptr;
		switch (type)
		{
		case DataType::None:
		break;
		case DataType::Bool:
			data = new bool(lua_toboolean(L, -1));
		break;
		case DataType::Float:
			data = new float(lua_tonumber(L, -1));
		break;
		case DataType::String:
			data = new std::string(lua_tostring(L, -1));
		break;
		case DataType::Filepath:
			data = new std::string(lua_tostring(L, -1));
		break;

		case DataType::Vec2:
		{
			glm::vec2* v = new glm::vec2();

			lua_rawgeti(L, -1, 1);
			v->x = lua_tonumber(L, -1);
			lua_pop(L, 1);

			lua_rawgeti(L, -1, 2);
			v->y = lua_tonumber(L, -1);
			lua_pop(L, 1);

			data = v;
		}
		break;

		case DataType::Color:
		{
			Color* col = new Color(0, 0, 0, 1);

			lua_rawgeti(L, -1, 1);
			col->r = lua_tonumber(L, -1);
			lua_pop(L, 1);

			lua_rawgeti(L, -1, 2);
			col->g = lua_tonumber(L, -1);
			lua_pop(L, 1);

			lua_rawgeti(L, -1, 3);
			col->b = lua_tonumber(L, -1);
			lua_pop(L, 1);

			lua_rawgeti(L, -1, 4);
			col->a = lua_tonumber(L, -1);
			lua_pop(L, 1);
			
			data = col;
		}
		break;

		case DataType::Vec2Array:
		{
			std::vector<glm::vec2>* points = new std::vector<glm::vec2>();

			lua_pushnil(L);
			while (lua_next(L, -2) != 0)
			{
				lua_pushnumber(L, 1);
				lua_gettable(L, -2);
				float x = lua_tonumber(L, -1);
				lua_pop(L, 1);

				lua_pushnumber(L, 2);
				lua_gettable(L, -2);
				float y = lua_tonumber(L, -1);
				lua_pop(L, 1);

				points->push_back(glm::vec2(x, y));

				lua_pop(L, 1);
			}

			data = points;
		}
		break;

		}

		lua_pop(L, 1);

		return Parameter(name, type, data, killdata);
	}

	void Parameter::Serialize(lua_State* L)
	{
		int table = lua_gettop(L);

		lua_pushstring(L, "datatype");
		lua_pushnumber(L, static_cast<int>(Type));
		lua_rawset(L, table);

		lua_pushstring(L, "name");
		lua_pushstring(L, Name.c_str());
		lua_rawset(L, table);

		lua_pushstring(L, "killdata");
		lua_pushboolean(L, Settings.KillData);
		lua_rawset(L, table);

		lua_pushstring(L, "data");
		lua_PushData(L);
		lua_rawset(L, table);
	}

	void Parameter::lua_PushData(lua_State* L, bool _makeUsable)
	{
		switch (Type)
		{
		case DataType::None:
			lua_pushnil(L);
			break;

		case DataType::Bool:
			lua_pushboolean(L, *static_cast<bool*>(Data));
			break;

		case DataType::Float:
			lua_pushnumber(L, *static_cast<float*>(Data));
			break;

		case DataType::Vec2:
		{
			glm::vec2* vec = static_cast<glm::vec2*>(Data);
			if (_makeUsable)
			{
				LuaVector2::lua_FromVec(L, vec, false);
			}
			else
			{
				lua_newtable(L);
				lua_pushnumber(L, vec->x);
				lua_rawseti(L, -2, 1);

				lua_pushnumber(L, vec->y);
				lua_rawseti(L, -2, 2);
			}
		}
		break;

		case DataType::Vec2Array:
		{
			std::vector<glm::vec2>* points = static_cast<std::vector<glm::vec2>*>(Data);

			if (_makeUsable)
			{
				LuaComponent::lua_NewVec2Array(L, points);
			}
			else
			{
				lua_newtable(L);
				int tbl_data = lua_gettop(L);
				{
					for (int i = 0; i < points->size(); i++)
					{
						lua_newtable(L);
						lua_pushnumber(L, (*points)[i].x);
						lua_rawseti(L, -2, 1);

						lua_pushnumber(L, (*points)[i].y);
						lua_rawseti(L, -2, 2);

						lua_rawseti(L, tbl_data, i + 1);
					}
				}
			}

		}
		break;

		case DataType::Color:
		{
			Color* color = static_cast<Color*>(Data);

			if (_makeUsable)
			{
				LuaColor::lua_FromColor(L, color, false);
			}
			else
			{
				lua_newtable(L);
				lua_pushnumber(L, color->r);
				lua_rawseti(L, -2, 1);

				lua_pushnumber(L, color->g);
				lua_rawseti(L, -2, 2);

				lua_pushnumber(L, color->b);
				lua_rawseti(L, -2, 3);

				lua_pushnumber(L, color->a);
				lua_rawseti(L, -2, 4);
			}

		}
		break;

		case DataType::String:
			lua_pushstring(L, static_cast<std::string*>(Data)->c_str());
			break;

		case DataType::Filepath:
			lua_pushstring(L, static_cast<std::string*>(Data)->c_str());
			break;

		default:
			lua_pushnil(L);
			break;

		}
	}

	void Parameter::lua_TrySetData(lua_State* L)
	{
		switch (Type)
		{
		case DataType::Bool:
		{
			if (lua_isboolean(L, -1))
			{
				bool* _data = static_cast<bool*>(Data);
				*_data = lua_toboolean(L, -1);
			}
		}
		break;

		case DataType::Float:
		{
			if (lua_isnumber(L, -1))
			{
				float* _data = static_cast<float*>(Data);
				*_data = lua_tonumber(L, -1);
			}
		}
		break;

		case DataType::Vec2:
		{
			if (lua_isuserdata(L, -1))
			{
				glm::vec2* _data = static_cast<glm::vec2*>(Data);
				*_data = *static_cast<LuaVector2*>(lua_touserdata(L, -1))->Vector;
			}
		}
		break;

		case DataType::Vec2Array:
		{
			if (lua_istable(L, -1))
			{
				lua_pushstring(L, "__data");
				lua_rawget(L, -1);

				if (!lua_isnil(L, -1))
				{
					std::vector<glm::vec2>* points = static_cast<std::vector<glm::vec2>*>(Data);
					std::vector<glm::vec2>* new_points = static_cast<std::vector<glm::vec2>*>(lua_touserdata(L, -1));

					points->clear();
					for (int i = 0; i < new_points->size(); i++)
						points->push_back((*new_points)[i]);
				}
				lua_pop(L, 1);
			}
		}
		break;

		case DataType::Color:
		{
			if (lua_isuserdata(L, -1))
			{
				Color* _data = static_cast<Color*>(Data);
				*_data = *static_cast<LuaColor*>(lua_touserdata(L, -1))->ColorPtr;
			}
		}
		break;

		case DataType::String:
		{
			if (lua_isstring(L, -1))
			{
				std::string* _data = static_cast<std::string*>(Data);
				*_data = lua_tostring(L, -1);
			}
		}
		break;

		case DataType::Filepath:
		{
			if (lua_isstring(L, -1))
			{
				std::string* _data = static_cast<std::string*>(Data);
				*_data = lua_tostring(L, -1);
			}
		}
		break;
		}
	}

	Parameter Parameter::lua_TryCreateParameter(lua_State* L)
	{
		int value = lua_gettop(L);

		std::string type = lua_tostring(L, -2);
		std::string name = lua_tostring(L, -3);

		if (type == "bool")
		{
			return Parameter(name, DataType::Bool, new bool(lua_toboolean(L, value)), true);
		}

		else if (type == "float")
		{
			return Parameter(name, DataType::Float, new float(lua_tonumber(L, value)), true);
		}

		else if (type == "vec2")
		{
			if (lua_isuserdata(L, value))
			{
				glm::vec2* vec = static_cast<LuaVector2*>(lua_touserdata(L, value))->Vector;
				return Parameter(name, DataType::Vec2, new glm::vec2(*vec), true);
			}
		}

		else if (type == "vec2array")
		{
			if (lua_istable(L, value))
			{
				std::vector<glm::vec2>* points = new std::vector<glm::vec2>();

				lua_pushnil(L);
				while (lua_next(L, value) != 0)
				{
					if (lua_isuserdata(L, -1))
					{
						glm::vec2* vec = static_cast<LuaVector2*>(lua_touserdata(L, -1))->Vector;
						points->push_back(*vec);
					}

					lua_pop(L, 1);
				}

				return Parameter(name, DataType::Vec2Array, points, true);
			}
		}

		else if (type == "color")
		{
			if (lua_isuserdata(L, value))
			{
				Color* color = static_cast<LuaColor*>(lua_touserdata(L, value))->ColorPtr;
				return Parameter(name, DataType::Color, new Color(*color), true);
			}
		}

		else if (type == "string")
		{
			return Parameter(name, DataType::String, new std::string(lua_tostring(L, value)), DataSettings(true));
		}

		else if (type == "filepath")
		{
			return Parameter(name, DataType::String, new std::string(lua_tostring(L, value)), DataSettings(true));
		}

		return Parameter();
	}

	#pragma endregion

	#pragma region Lua


	LuaComponent::LuaComponent(Component* _component)
	{
		Comp = _component;
	}

	LuaComponent::~LuaComponent()
	{
		delete Comp;
	}

	int LuaComponent::lua_New(lua_State* L, Component* _component)
	{
		LuaComponent* lcomp = static_cast<LuaComponent*>(lua_newuserdata(L, sizeof(LuaComponent)));
		new (lcomp) LuaComponent(_component);

		luaL_getmetatable(L, "ComponentMTT");
		lua_setmetatable(L, -2);

		return 1;
	}

	int LuaComponent::lua_Index(lua_State* L)
	{
		Component* comp = static_cast<Component*>(static_cast<LuaComponent*>(lua_touserdata(L, 1))->Comp);
		std::string index = lua_tostring(L, 2);

		if (index == "name")
		{
			lua_pushstring(L, comp->GetType().c_str());
			return 1;
		}
		else if (index == "parent")
		{
			LuaObject::lua_New(L, comp->Parent);
			return 1;
		}

		for (Parameter parameter : comp->Parameters)
		{
			std::string name = parameter.Name;
			DataType type = parameter.Type;

			if (!parameter.Settings.KillData)
				continue;

			if (index == name)
			{
				if (type == DataType::String)
				{
					lua_pushstring(L, static_cast<std::string*>(parameter.Data)->c_str());
					return 1;
				}
				else if (type == DataType::Bool)
				{
					lua_pushboolean(L, *static_cast<bool*>(parameter.Data));
					return 1;
				}
				else if (type == DataType::Float)
				{
					lua_pushnumber(L, *static_cast<float*>(parameter.Data));
					return 1;
				}
				else if (type == DataType::Vec2)
				{
					LuaVector2::lua_FromVec(L, static_cast<glm::vec2*>(parameter.Data), false);
					return 1;
				}
				else if (type == DataType::Color)
				{
					LuaColor::lua_FromColor(L, static_cast<Color*>(parameter.Data), false);
					return 1;
				}
				else if (type == DataType::Vec2Array)
				{
					lua_NewVec2Array(L, static_cast<std::vector<glm::vec2>*>(parameter.Data));
					return 1;
				}
				else
				{
					LuaThrowSyntaxError(L, "Failed to identify type of component parameter.");
					return 0;
				}
			}
		}

		return 0;
		
	}

	int LuaComponent::lua_NewVec2Array(lua_State* L, std::vector<glm::vec2>* _points)
	{
		lua_newtable(L);
		lua_newtable(L);

		lua_pushstring(L, "__newindex");
		lua_pushcfunction(L, LuaComponent::lua_Vec2ArrayNewIndex);
		lua_settable(L, -3);

		lua_pushstring(L, "__index");
		lua_pushcfunction(L, LuaComponent::lua_Vec2ArrayIndex);
		lua_settable(L, -3);

		lua_pushstring(L, "__len");
		lua_pushcfunction(L, LuaComponent::lua_Vec2ArrayLen);
		lua_settable(L, -3);

		lua_setmetatable(L, -2);

		lua_pushstring(L, "__data");
		lua_pushlightuserdata(L, _points);
		lua_rawset(L, -3);

		return 1;
	}

	int LuaComponent::lua_Vec2ArrayIndex(lua_State* L)
	{
		if (lua_type(L, 2) == LUA_TNUMBER)
		{
			lua_pushstring(L, "__data");
			lua_rawget(L, 1);

			int n = lua_tonumber(L, 2) - 1;
			std::vector<glm::vec2>* data = static_cast<std::vector<glm::vec2>*>(lua_touserdata(L, -1));

			if (n >= 0 && n < data->size())
			{
				glm::vec2* vec = &(*data)[n];

				LuaVector2::lua_FromVec(L, vec, false);
				return 1;
			}
		}
		else
		{
			LuaThrowSyntaxError(L, "Vec2Arrays must be indexed by a number.");
			return 0;
		}

		return 0;
	}

	int LuaComponent::lua_Vec2ArrayNewIndex(lua_State* L)
	{
		if (lua_type(L, 2) == LUA_TNUMBER)
		{
			if (lua_type(L, 3) == LUA_TUSERDATA)
			{
				lua_pushstring(L, "__data");
				lua_rawget(L, 1);

				int n = lua_tonumber(L, 2) - 1;
				std::vector<glm::vec2>* data = static_cast<std::vector<glm::vec2>*>(lua_touserdata(L, -1));

				if (n >= data->size())
				{
					glm::vec2* vec = static_cast<LuaVector2*>(lua_touserdata(L, 3))->Vector;
					data->push_back(glm::vec2(vec->x, vec->y));
				}
				else if (n >= 0)
				{
					glm::vec2* vec = static_cast<LuaVector2*>(lua_touserdata(L, 3))->Vector;
					glm::vec2* source_vec = &(*data)[n];

					*source_vec = *vec;
				}
			}
			else if (lua_type(L, 3) == LUA_TNIL)
			{
				lua_pushstring(L, "__data");
				lua_rawget(L, 1);

				int n = lua_tonumber(L, 2) - 1;
				std::vector<glm::vec2>* data = static_cast<std::vector<glm::vec2>*>(lua_touserdata(L, -1));

				if (n >= 0 && n < data->size())
				{
					if (data->size() == 1)
					{
						(*data)[n] = glm::vec2(0, 0);
					}
					else
					{
						data->erase(data->begin() + n);
					}
				}
			}
		}
		else
		{
			LuaThrowSyntaxError(L, "ShapeRenderer.points must be indexed by a number.");
		}

		return 0;
	}

	int LuaComponent::lua_Vec2ArrayLen(lua_State* L)
	{
		lua_pushstring(L, "__data");
		lua_rawget(L, 1);

		std::vector<glm::vec2>* data = static_cast<std::vector<glm::vec2>*>(lua_touserdata(L, -1));
		lua_pushnumber(L, data->size());

		return 1;
	}

	int LuaComponent::lua_NewIndex(lua_State* L)
	{
		Component* comp = static_cast<Component*>(static_cast<LuaComponent*>(lua_touserdata(L, 1))->Comp);
		std::string index = lua_tostring(L, 2);

		if (index == "name")
			LuaThrowSyntaxError(L, "Component names cannot be modified.");

		if (index == "parent")
			LuaThrowSyntaxError(L, "Component parents cannot be modified.");

		for (Parameter parameter : comp->Parameters)
		{
			std::string name = parameter.Name;
			DataType type = parameter.Type;

			if (!parameter.Settings.KillData)
				continue;

			if (index == name)
			{
				if (lua_isuserdata(L, 3))
				{
					if (type == DataType::Vec2)
					{
						glm::vec2* vec = static_cast<LuaVector2*>(lua_touserdata(L, 3))->Vector;
						glm::vec2* obj_vec = static_cast<glm::vec2*>(parameter.Data);

						*obj_vec = *vec;
					}
					else if (type == DataType::Color)
					{
						Color* col = static_cast<LuaColor*>(lua_touserdata(L, 3))->ColorPtr;
						Color* obj_col = static_cast<Color*>(parameter.Data);

						*obj_col = *col;
					}
				}
				else if (lua_type(L, 3) == LUA_TBOOLEAN && type == DataType::Bool)
				{
					bool* val = static_cast<bool*>(parameter.Data);
					*val = lua_toboolean(L, 3);
				}
				else if (lua_type(L, 3) == LUA_TNUMBER && type == DataType::Float)
				{
					float* val = static_cast<float*>(parameter.Data);
					*val = lua_tonumber(L, 3);
				}
				else if (lua_type(L, 3) == LUA_TSTRING && type == DataType::String)
				{
					std::string* str = static_cast<std::string*>(parameter.Data);
					*str = lua_tostring(L, 3);
				}
				else if (lua_type(L, 3) == LUA_TTABLE && type == DataType::Vec2Array)
				{
					std::vector<glm::vec2>* points = static_cast<std::vector<glm::vec2>*>(parameter.Data);
					points->clear();

					lua_pushnil(L);
					while (lua_next(L, 3) != 0)
					{	
						if (lua_isuserdata(L, -1))
						{
							glm::vec2* vec = static_cast<LuaVector2*>(lua_touserdata(L, -1))->Vector;
							points->push_back(*vec);
						}

						lua_pop(L, 1);
					}
				}
				else
				{
					LuaThrowSyntaxError(L, index + " cannot accept the type of value given.");
				}
			}
		}

		return 0;
	}

	int LuaComponent::lua_ToString(lua_State* L)
	{
		Component* comp = static_cast<Component*>(static_cast<LuaComponent*>(lua_touserdata(L, 1))->Comp);
		std::string str = comp->GetType();

		lua_pushstring(L, str.c_str());
		return 1;
	}

	void LuaComponent::lua_RegisterComponent(lua_State* L)
	{
		luaL_newmetatable(L, "ComponentMTT");

		lua_pushstring(L, "__index");
		lua_pushcfunction(L, LuaComponent::lua_Index);
		lua_settable(L, -3);

		lua_pushstring(L, "__newindex");
		lua_pushcfunction(L, LuaComponent::lua_NewIndex);
		lua_settable(L, -3);

		lua_pushstring(L, "__tostring");
		lua_pushcfunction(L, LuaComponent::lua_ToString);
		lua_settable(L, -3);
	}

	#pragma endregion

	#pragma region Base Component

	std::string Component::GetClassType() { return "Base"; }
	std::string Component::GetType() { return "Base"; }

	Component::~Component()
	{
		for (Parameter parameter : Parameters)
		{
			if (parameter.Settings.KillData) delete parameter.Data;
		}
	}

	#pragma endregion

	#pragma region Transform

	std::string Transform::GetClassType() { return "Transform"; }
	std::string Transform::GetType() { return "Transform"; }

	Transform::Transform(Object* _parent)
	{
		Parent = _parent;

		Position = new glm::vec2(0, 0);
		Rotation = new float(0.0f);
		Scale = new glm::vec2(1, 1);

		AddParameter<DataType::Vec2>("position", Position, { true });
		AddParameter<DataType::Float>("rotation", Rotation, { true });
		AddParameter<DataType::Vec2>("scale", Scale, { true });
	}

	void Transform::Serialize(lua_State* L)
	{
		int table = lua_gettop(L);

		lua_pushstring(L, "__type");
		lua_pushstring(L, "Transform");
		lua_rawset(L, table);

		lua_pushstring(L, "x");
		lua_pushnumber(L, Position->x);
		lua_rawset(L, table);

		lua_pushstring(L, "y");
		lua_pushnumber(L, Position->y);
		lua_rawset(L, table);

		lua_pushstring(L, "scale_x");
		lua_pushnumber(L, Scale->x);
		lua_rawset(L, table);

		lua_pushstring(L, "scale_y");
		lua_pushnumber(L, Scale->y);
		lua_rawset(L, table);

		lua_pushstring(L, "rotation");
		lua_pushnumber(L, *Rotation);
		lua_rawset(L, table);
	}

	void Transform::Deserialize(lua_State* L)
	{
		lua_pushnil(L);
		while (lua_next(L, -2) != 0)
		{
			std::string key = lua_tostring(L, -2);
			if (key == "x") Position->x = lua_tonumber(L, -1);
			if (key == "y") Position->y = lua_tonumber(L, -1);
			if (key == "rotation") *Rotation = lua_tonumber(L, -1);
			if (key == "scale_x") Scale->x = lua_tonumber(L, -1);
			if (key == "scale_y") Scale->y = lua_tonumber(L, -1);

			lua_pop(L, 1);
		}
	}

	#pragma endregion
	
	#pragma region ShapeRenderer

	std::string ShapeRenderer::GetClassType() { return "ShapeRenderer"; }
	std::string ShapeRenderer::GetType() { return "ShapeRenderer"; }

	ShapeRenderer::ShapeRenderer(Object* _parent)
	{
		Parent = _parent;

		ShapeData = new Shape({ {-50, -50}, {50, -50}, {50, 50}, {-50, 50} });
		m_Points = new std::vector<glm::vec2>(ShapeData->GetPoints());
		Visible = new bool(true);
		LineMode = new bool(false);
		LineWidth = new float(1.0f);
		ColorData = new Color(1, 1, 1, 1);
		Layer = new float(0);

		AddParameter<DataType::Bool>("visible", Visible, { true });
		AddParameter<DataType::Bool>("line_mode", LineMode, { true });
		AddParameter<DataType::Float>("line_width", LineWidth, { true });
		AddParameter<DataType::Color>("color", ColorData, { true });
		AddParameter<DataType::Float>("layer", Layer, { true });
		AddParameter<DataType::Separator>("", nullptr, { false });
		AddParameter<DataType::Vec2ArrayRenderer>("", m_Points, { false });
		AddParameter<DataType::Vec2Array>("points", m_Points, { true });
	}

	void ShapeRenderer::OnTick(bool _onGame, bool _onEditor)
	{
		Transform* transform = Parent->GetComponent<Transform>();

		if (ShapeData->GetPoints() != *m_Points)
		{
			if (m_Points->size() == 0)
				m_Points->push_back(glm::vec2(0, 0));

			ShapeData->Reform(*m_Points);
		}

		Camera* chosen_cam = nullptr;
		if (_onGame) chosen_cam = Render::MainCamera;
		if (_onEditor) chosen_cam = Render::EditorCamera;

		if (chosen_cam != nullptr && *Visible)
		{
			glm::vec2 position = (transform != nullptr) ? *transform->Position : glm::vec2(0, 0);
			glm::vec2 scale = (transform != nullptr) ? *transform->Scale : glm::vec2(1, 1);
			float rotation = (transform != nullptr) ? -*transform->Rotation : 0.0f;

			Render::DrawShape(*chosen_cam, *ShapeData, Render::DefaultShader, position, rotation, scale, !(*LineMode), *ColorData, *LineWidth);
		}
	}

	void ShapeRenderer::Serialize(lua_State* L)
	{
		int table = lua_gettop(L);

		lua_pushstring(L, "__type");
		lua_pushstring(L, "ShapeRenderer");
		lua_rawset(L, table);

		lua_pushstring(L, "visible");
		lua_pushboolean(L, *Visible);
		lua_rawset(L, table);

		lua_pushstring(L, "layer");
		lua_pushnumber(L, *Layer);
		lua_rawset(L, table);

		lua_pushstring(L, "line_mode");
		lua_pushboolean(L, *LineMode);
		lua_rawset(L, table);

		lua_pushstring(L, "line_width");
		lua_pushnumber(L, *LineWidth);
		lua_rawset(L, table);

		lua_pushstring(L, "points");
		lua_newtable(L);
		int tbl_points = lua_gettop(L);
		{
			lua_pushstring(L, "__type");
			lua_pushstring(L, "vec2array");
			lua_rawset(L, tbl_points);

			lua_pushstring(L, "__data");
			lua_newtable(L);
			int tbl_data = lua_gettop(L);
			{
				for (int i = 0; i < m_Points->size(); i++)
				{
					lua_newtable(L);
					lua_pushnumber(L, (*m_Points)[i].x);
					lua_rawseti(L, -2, 1);

					lua_pushnumber(L, (*m_Points)[i].y);
					lua_rawseti(L, -2, 2);
					
					lua_rawseti(L, tbl_data, i + 1);
				}

				lua_rawset(L, tbl_points);
			}
		}
		lua_rawset(L, table);

		lua_pushstring(L, "color");
		lua_newtable(L);
		tbl_points = lua_gettop(L);
		{
			lua_pushstring(L, "__type");
			lua_pushstring(L, "colorarray");
			lua_rawset(L, tbl_points);

			lua_pushstring(L, "__data");
			lua_newtable(L);
			int tbl_data = lua_gettop(L);
			{
				lua_pushnumber(L, ColorData->r);
				lua_rawseti(L, tbl_data, 1);

				lua_pushnumber(L, ColorData->g);
				lua_rawseti(L, tbl_data, 2);

				lua_pushnumber(L, ColorData->b);
				lua_rawseti(L, tbl_data, 3);

				lua_pushnumber(L, ColorData->a);
				lua_rawseti(L, tbl_data, 4);
			}
			lua_rawset(L, tbl_points);
		}
		lua_rawset(L, table);
	}

	void ShapeRenderer::Deserialize(lua_State* L)
	{
		lua_pushnil(L);
		while (lua_next(L, -2) != 0)
		{
			std::string key = lua_tostring(L, -2);
			if (key == "visible") *Visible = lua_toboolean(L, -1);
			if (key == "line_mode") *LineMode = lua_toboolean(L, -1);
			if (key == "line_width") *LineWidth = lua_tonumber(L, -1);
			if (key == "layer") *Layer = lua_tonumber(L, -1);

			if (key == "points")
			{
				m_Points->clear();

				lua_pushstring(L, "__data");
				lua_rawget(L, -2);

				lua_pushnil(L);
				while (lua_next(L, -2) != 0)
				{
					lua_pushnumber(L, 1);
					lua_gettable(L, -2);
					float x = lua_tonumber(L, -1);
					lua_pop(L, 1);

					lua_pushnumber(L, 2);
					lua_gettable(L, -2);
					float y = lua_tonumber(L, -1);
					lua_pop(L, 1);

					m_Points->push_back(glm::vec2(x, y));
					
					lua_pop(L, 1);
				}
				lua_pop(L, 1);
			}

			if (key == "color")
			{
				lua_pushstring(L, "__data");
				lua_rawget(L, -2);

				lua_rawgeti(L, -1, 1);
				ColorData->r = lua_tonumber(L, -1);
				lua_pop(L, 1);

				lua_rawgeti(L, -1, 2);
				ColorData->g = lua_tonumber(L, -1);
				lua_pop(L, 1);

				lua_rawgeti(L, -1, 3);
				ColorData->b = lua_tonumber(L, -1);
				lua_pop(L, 1);

				lua_rawgeti(L, -1, 4);
				ColorData->a = lua_tonumber(L, -1);
				lua_pop(L, 1);

				lua_pop(L, 1);
			}

			lua_pop(L, 1);
		}
	}

	#pragma endregion

	#pragma region ScriptComponent

	std::string ScriptComponent::GetClassType() { return "ScriptComponent"; }
	std::string ScriptComponent::GetType() { return "ScriptComponent"; }

	ScriptComponent::ScriptComponent(Object* _parent)
	{
		Parent = _parent;
		ScriptData = new Script(Parent);

		AddParameter<DataType::Filepath>("filepath", ScriptData->Filepath, { true });
		AddParameter<DataType::Bool>("active", ScriptData->Active, { true });
	}

	void ScriptComponent::OnTick(bool _onGame, bool _onEditor)
	{
		if (_onGame && *ScriptData->Active)
			ScriptData->Run();
	}

	void ScriptComponent::Callback()
	{
		StringReplace(ScriptData->Filepath, "\\", "/");

		if (!ScriptData->LoadScript(*ScriptData->Filepath))
		{
			*ScriptData->Filepath = "Invalid filepath.";
		}
	}

	void ScriptComponent::Serialize(lua_State* L)
	{
		int table = lua_gettop(L);

		lua_pushstring(L, "__type");
		lua_pushstring(L, "ScriptComponent");
		lua_rawset(L, table);

		lua_pushstring(L, "filepath");
		lua_pushstring(L, ScriptData->Filepath->c_str());
		lua_rawset(L, table);

		lua_pushstring(L, "active");
		lua_pushboolean(L, *ScriptData->Active);
		lua_rawset(L, table);
	}

	void ScriptComponent::Deserialize(lua_State* L)
	{
		lua_pushnil(L);
		while (lua_next(L, -2) != 0)
		{
			std::string key = lua_tostring(L, -2);
			if (key == "active") *ScriptData->Active = lua_toboolean(L, -1);

			if (key == "filepath")
			{
				*ScriptData->Filepath = lua_tostring(L, -1);
				Callback();
			}

			lua_pop(L, 1);
		}
	}

	#pragma endregion

	#pragma region TextRenderer

	std::string TextRenderer::GetClassType() { return "TextRenderer"; }
	std::string TextRenderer::GetType() { return "TextRenderer"; }

	TextRenderer::TextRenderer(Object* _parent)
	{
		Parent = _parent;

		Visible = new bool(true);
		Text = new std::string("text");
		Offset = new glm::vec2(0, 0);
		ColorData = new Color(1, 1, 1, 1);

		AddParameter<DataType::Bool>("visible", Visible, { true });
		AddParameter<DataType::String>("text", Text, { true });
		AddParameter<DataType::Vec2>("offset", Offset, { true });
		AddParameter<DataType::Color>("color", ColorData, { true });
	}

	void TextRenderer::OnTick(bool _onGame, bool _onEditor)
	{
		Transform* transform = Parent->GetComponent<Transform>();

		Camera* chosen_cam = nullptr;
		if (_onGame) chosen_cam = Render::MainCamera;
		if (_onEditor) chosen_cam = Render::EditorCamera;

		if (chosen_cam != nullptr && *Visible)
		{
			glm::vec2 position = (transform != nullptr) ? *transform->Position : glm::vec2(0, 0);
			glm::vec2 scale = (transform != nullptr) ? *transform->Scale : glm::vec2(1, 1);

			Render::DrawString(*chosen_cam, *Text, Render::TextureShader, position + *Offset, scale, *ColorData);
		}
	}

	void TextRenderer::Serialize(lua_State* L)
	{
		int table = lua_gettop(L);

		lua_pushstring(L, "__type");
		lua_pushstring(L, "TextRenderer");
		lua_rawset(L, table);


		lua_pushstring(L, "visible");
		lua_pushboolean(L, *Visible);
		lua_rawset(L, table);


		lua_pushstring(L, "text");
		lua_pushstring(L, Text->c_str());
		lua_rawset(L, table);


		lua_pushstring(L, "offset_x");
		lua_pushnumber(L, Offset->x);
		lua_rawset(L, table);

		lua_pushstring(L, "offset_y");
		lua_pushnumber(L, Offset->y);
		lua_rawset(L, table);


		lua_pushstring(L, "color");
		lua_newtable(L);
		int tbl_color = lua_gettop(L);
		{
			lua_pushstring(L, "__type");
			lua_pushstring(L, "colorarray");
			lua_rawset(L, tbl_color);

			lua_pushstring(L, "__data");
			lua_newtable(L);
			int tbl_data = lua_gettop(L);
			{
				lua_pushnumber(L, ColorData->r);
				lua_rawseti(L, tbl_data, 1);

				lua_pushnumber(L, ColorData->g);
				lua_rawseti(L, tbl_data, 2);

				lua_pushnumber(L, ColorData->b);
				lua_rawseti(L, tbl_data, 3);

				lua_pushnumber(L, ColorData->a);
				lua_rawseti(L, tbl_data, 4);
			}
			lua_rawset(L, tbl_color);
		}
		lua_rawset(L, table);
	}

	void TextRenderer::Deserialize(lua_State* L)
	{
		lua_pushnil(L);
		while (lua_next(L, -2) != 0)
		{
			std::string key = lua_tostring(L, -2);
			if (key == "visible") *Visible = lua_toboolean(L, -1);
			if (key == "text") *Text = lua_tostring(L, -1);
			if (key == "offset_x") Offset->x = lua_tonumber(L, -1);
			if (key == "offset_y") Offset->y = lua_tonumber(L, -1);
			if (key == "color")
			{
				lua_pushstring(L, "__data");
				lua_rawget(L, -2);

				lua_rawgeti(L, -1, 1);
				ColorData->r = lua_tonumber(L, -1);
				lua_pop(L, 1);

				lua_rawgeti(L, -1, 2);
				ColorData->g = lua_tonumber(L, -1);
				lua_pop(L, 1);

				lua_rawgeti(L, -1, 3);
				ColorData->b = lua_tonumber(L, -1);
				lua_pop(L, 1);

				lua_rawgeti(L, -1, 4);
				ColorData->a = lua_tonumber(L, -1);
				lua_pop(L, 1);

				lua_pop(L, 1);
			}

			lua_pop(L, 1);
		}
	}

	#pragma endregion

}
