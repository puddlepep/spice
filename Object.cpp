#include "Object.hpp"

namespace Spice
{

	Object::Object(std::string _name)
	{
		Name = _name;
		Active = true;
	}

	LuaObject::LuaObject(Object* _ptr)
	{
		Obj = _ptr;
	}

	LuaObject::~LuaObject()
	{
		delete Obj;
	}

	void Object::PushStorageItem(Parameter _parameter)
	{
		if (_parameter.Data == nullptr)
		{
			switch (_parameter.Type)
			{
			case DataType::Bool:
				_parameter.Data = new bool(false);
				break;
			case DataType::Float:
				_parameter.Data = new float(0.0f);
				break;
			case DataType::Vec2:
				_parameter.Data = new glm::vec2(0, 0);
				break;
			case DataType::Vec2Array:
				_parameter.Data = new std::vector<glm::vec2>();
				break;
			case DataType::Color:
				_parameter.Data = new Color(0, 0, 0, 1);
				break;
			case DataType::String:
				_parameter.Data = new std::string("");
				break;
			case DataType::Filepath:
				_parameter.Data = new std::string("");
				break;
			} 
		}

		Storage.push_back(_parameter);
	}

	Parameter Object::GetStorageItem(std::string _paramName)
	{
		for (Parameter param : Storage)
		{
			if (param.Name == _paramName)
				return param;
		}

		return Parameter("", DataType::None, nullptr, { false });
	}

	Parameter Object::GetStorageItem(int _index)
	{
		if (_index >= 0 && _index < Storage.size())
		{
			return Storage[_index];
		}

		return Parameter("", DataType::None, nullptr, { false });
	}

	void Object::RemoveStorageItem(std::string _paramName)
	{
		for (int i = 0; i < Storage.size(); i++)
		{
			Parameter& param = Storage[i];
			if (param.Name == _paramName)
			{
				delete param.Data;
				Storage.erase(Storage.begin() + i);
			}
		}
	}

	void Object::RemoveStorageItem(int _paramIndex)
	{
		if (_paramIndex < Storage.size() && _paramIndex >= 0)
		{
			delete Storage[_paramIndex].Data;
			Storage.erase(Storage.begin() + _paramIndex);
		}
	}

	void Object::RemoveComponent(int _index)
	{
		if (_index < Components.size() && _index >= 0)
		{
			delete Components[_index];
			Components.erase(Components.begin() + _index);
		}
	}

	Object::~Object()
	{

		for (Component* c : Components)
			delete c;

		for (Parameter p : Storage)
			delete p.Data;

		Components.clear();
	}

	// LUA //

	int LuaObject::lua_GetComponent(lua_State* L)
	{
		if (lua_gettop(L) > 1)
		{
			if (lua_isuserdata(L, 1))
			{
				if (lua_isstring(L, 2))
				{
					LuaObject* lobj = static_cast<LuaObject*>(lua_touserdata(L, 1));
					Object* obj = lobj->Obj;
						
					Component* comp = obj->GetComponent(lua_tostring(L, 2));
					if (comp == nullptr) return 0;

					LuaComponent::lua_New(L, comp);
					return 1;
				}
				else
				{
					LuaThrowSyntaxError(L, "Object.GetComponent(object, component)'s second parameter must be a string!");
					return 0;
				}
			}
			else
			{
				LuaThrowSyntaxError(L, "Object.GetComponent(object, component)'s first parameter must be an Object!");
				return 0;
			}
		}
		else
		{
			LuaThrowSyntaxError(L, "Object.GetComponent(object, component) requires 2 parameters!");
			return 0;
		}
	}

	int LuaObject::lua_AddComponent(lua_State* L)
	{
		lua_pushstring(L, "NO Component");
		return 1;
	}

	int LuaObject::lua_AddStorageItem(lua_State* L)
	{
		if (lua_gettop(L) == 4)
		{
			if (lua_isuserdata(L, 1))
			{
				if (lua_isstring(L, 2) && lua_isstring(L, 3))
				{
					Object* obj = static_cast<LuaObject*>(lua_touserdata(L, 1))->Obj;
					Parameter param = Parameter::lua_TryCreateParameter(L);
					obj->PushStorageItem(param);
					return 0;
				}
				else
				{
					LuaThrowSyntaxError(L, "Object.AddStorageItem(object, name, type, value)'s second and third parameters must be strings!");
					return 0;
				}
			}
			else
			{
				LuaThrowSyntaxError(L, "Object.AddStorageItem(object, name, type, value)'s first parameter must be an object!");
				return 0;
			}
		}
		else
		{
			LuaThrowSyntaxError(L, "Object.AddStorageItem(object, name, type, value) requires 4 parameters!");
			return 0;
		}
	}

	int LuaObject::lua_Clone(lua_State* L)
	{
		if (lua_gettop(L) == 1)
		{
			if (lua_type(L, 1) == LUA_TUSERDATA)
			{
				Object* object = static_cast<LuaObject*>(lua_touserdata(L, 1))->Obj;
				
				Object* new_object = new Object(object->Name);
				new_object->Active = object->Active;
				
				for (Component* comp : object->Components)
				{
					lua_newtable(L);
					comp->Serialize(L);

					Component* new_comp = new_object->AddComponent(comp->GetType());
					new_comp->Deserialize(L);
					lua_pop(L, 1);
				}

				for (Parameter param : object->Storage)
				{
					lua_newtable(L);
					param.Serialize(L);

					Parameter new_param = Parameter::Deserialize(L);
					new_object->PushStorageItem(new_param);
					lua_pop(L, 1);
				}

				EventHandler::PushEvent(EventHandler::EventHandleType::EngineHandled, EventHandler::EventType::AddObject, new_object);
				lua_New(L, new_object);
				return 1;
			}
			else
			{
				LuaThrowSyntaxError(L, "Object.Clone(object)'s first parameter must be an object!");
				return 0;
			}
		}
		else
		{
			LuaThrowSyntaxError(L, "Object.Clone(object) requires 1 parameter!");
			return 0;
		}
	}

	int LuaObject::lua_Destroy(lua_State* L)
	{
		if (lua_gettop(L) == 1)
		{
			if (lua_type(L, 1) == LUA_TUSERDATA)
			{
				LuaObject* object = static_cast<LuaObject*>(lua_touserdata(L, 1));
				lua_pushnil(L);
				lua_setmetatable(L, 1);

				EventHandler::PushEvent(EventHandler::EventHandleType::EngineHandled, EventHandler::EventType::DestroyObject, object->Obj);
				return 1;
			}
			else
			{
				LuaThrowSyntaxError(L, "Object.Destroy(object)'s first parameter must be an object!");
				return 0;
			}
		}
		else
		{
			LuaThrowSyntaxError(L, "Object.Destroy(object) requires 1 parameter!");
			return 0;
		}
	}

	int LuaObject::lua_Index(lua_State* L)
	{
		LuaObject* lobj = (LuaObject*)lua_touserdata(L, 1);
		Object* obj = lobj->Obj;

		std::string index = lua_tostring(L, 2);

		if (index == "name")
		{
			lua_pushstring(L, obj->Name.c_str());
			return 1;
		}
		else if (index == "active")
		{
			lua_pushboolean(L, obj->Active);
			return 1;
		}
		else if (index == "storage")
		{
			lua_newtable(L);

			lua_pushstring(L, "__object");
			lua_pushlightuserdata(L, lobj);
			lua_rawset(L, -3);

			luaL_getmetatable(L, "ObjectStorageMTT");
			lua_setmetatable(L, -2);
			return 1;
		}
		else
		{
			lua_getglobal(L, "Object");
			lua_pushstring(L, index.c_str());
			lua_rawget(L, -2);

			return 1;
		}

	}

	int LuaObject::lua_NewIndex(lua_State* L)
	{
	    LuaObject* lobj = (LuaObject*)lua_touserdata(L, 1);
		Object* obj = lobj->Obj;

		std::string index = lua_tostring(L, 2);

		if (lua_isstring(L, 3))
		{
			std::string value = lua_tostring(L, 3);
			if (index == "name")
				obj->Name = value;
		}
		else if (lua_isboolean(L, 3))
		{
			bool value = lua_toboolean(L, 3);
			if (index == "active")
				obj->Active = value;
		}

		return 0;
	}
	
	int LuaObject::lua_ToString(lua_State* L)
	{
		LuaObject* lobj = (LuaObject*)lua_touserdata(L, 1);
		Object* obj = lobj->Obj;

		std::string str = obj->Name;

		lua_pushstring(L, str.c_str());
		return 1;
	}

	int LuaObject::lua_StorageIndex(lua_State* L)
	{
		lua_pushstring(L, "__object");
		lua_rawget(L, 1);

		std::string index = lua_tostring(L, 2);
		Object* obj = static_cast<LuaObject*>(lua_touserdata(L, -1))->Obj;
		Parameter param = obj->GetStorageItem(index);
		param.lua_PushData(L, true);

		return 1;
	}

	int LuaObject::lua_StorageNewIndex(lua_State* L)
	{
		lua_pushstring(L, "__object");
		lua_rawget(L, 1);

		std::string index = lua_tostring(L, 2);
		Object* obj = static_cast<LuaObject*>(lua_touserdata(L, -1))->Obj;
		lua_pop(L, 1);

		if (lua_isnil(L, -1))
			obj->RemoveStorageItem(index);
		else
			obj->GetStorageItem(index).lua_TrySetData(L);

		return 0;
	}

	int LuaObject::lua_StorageLen(lua_State* L)
	{
		lua_pushstring(L, "__object");
		lua_rawget(L, 1);

		Object* obj = static_cast<LuaObject*>(lua_touserdata(L, -1))->Obj;
		lua_pushnumber(L, obj->Storage.size());
		return 1;
	}

	int LuaObject::lua_New(lua_State* L, Object* _object)
	{
		LuaObject* lobj = static_cast<LuaObject*>(lua_newuserdata(L, sizeof(LuaObject)));
		new (lobj) LuaObject(_object);

		luaL_getmetatable(L, "ObjectMTT");
		lua_setmetatable(L, -2);

		return 1;
	}

	void LuaObject::lua_RegisterObject(lua_State* L)
	{
		static const luaL_Reg ObjectLib[] =
		{
			{"GetComponent", LuaObject::lua_GetComponent},
			{"AddComponent", LuaObject::lua_AddComponent},
			{"AddStorageItem", LuaObject::lua_AddStorageItem},
			{"Clone", LuaObject::lua_Clone},
			{"Destroy", LuaObject::lua_Destroy},
			{NULL, NULL}
		};

		luaL_newlib(L, ObjectLib);
		lua_setglobal(L, "Object");

		luaL_newmetatable(L, "ObjectStorageMTT");

		lua_pushstring(L, "__index");
		lua_pushcfunction(L, LuaObject::lua_StorageIndex);
		lua_settable(L, -3);

		lua_pushstring(L, "__newindex");
		lua_pushcfunction(L, LuaObject::lua_StorageNewIndex);
		lua_settable(L, -3);

		lua_pushstring(L, "__len");
		lua_pushcfunction(L, LuaObject::lua_StorageLen);
		lua_settable(L, -3);

		luaL_newmetatable(L, "ObjectMTT");

		lua_pushstring(L, "__index");
		lua_pushcfunction(L, LuaObject::lua_Index);
		lua_settable(L, -3);

		lua_pushstring(L, "__newindex");
		lua_pushcfunction(L, LuaObject::lua_NewIndex);
		lua_settable(L, -3);

		lua_pushstring(L, "__tostring");
		lua_pushcfunction(L, LuaObject::lua_ToString);
		lua_settable(L, -3);
	}

}
