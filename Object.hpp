#pragma once

#include "Component.hpp"
#include "EventHandler.hpp"

namespace Spice
{
	class LuaObject
	{

	public:

		~LuaObject();
		LuaObject(Object* _ptr);
		Object* Obj = nullptr;

		static int lua_GetComponent(lua_State* L);
		static int lua_AddComponent(lua_State* L);

		static int lua_AddStorageItem(lua_State* L);

		static int lua_Clone(lua_State* L);
		static int lua_Destroy(lua_State* L);

		static int lua_Index(lua_State* L);
		static int lua_NewIndex(lua_State* L);
		static int lua_ToString(lua_State* L);

		static int lua_StorageIndex(lua_State* L);
		static int lua_StorageNewIndex(lua_State* L);
		static int lua_StorageLen(lua_State* L);
		

		static int lua_New(lua_State* L, Object* _object);
		static void lua_RegisterObject(lua_State* L);

	};

	class Object
	{

	public:
		
		std::string Name;
		bool Active;
		std::vector<Component*> Components;
		std::vector<Parameter> Storage;

		Object(std::string _name);

		//

		void PushStorageItem(Parameter _parameter);
		Parameter GetStorageItem(std::string _paramName);
		Parameter GetStorageItem(int _index);
		void RemoveStorageItem(std::string _paramName);
		void RemoveStorageItem(int _paramIndex);

		//

		void RemoveComponent(int _index);

		Component* AddComponent(std::string _type)
		{
			Component* comp = nullptr;

			if (_type == "Transform")
				comp = AddComponent<Transform>();

			else if (_type == "ShapeRenderer")
				comp = AddComponent<ShapeRenderer>();

			else if (_type == "ScriptComponent")
				comp = AddComponent<ScriptComponent>();

			else if (_type == "TextRenderer")
				comp = AddComponent<TextRenderer>();

			return comp;
		}

		template<typename T>
		Component* AddComponent()
		{
			if (GetComponent<T>() != nullptr) return nullptr;

			T* comp = new T(this);
			Components.push_back(comp);

			return comp;
		}

		Component* GetComponent(std::string _type)
		{
			for (Component* comp : Components)
			{
				if (comp->GetType() == _type)
				{
					return comp;
				}
			}

			return nullptr;
		}

		template<typename T>
		T* GetComponent()
		{
			std::string type = T::GetClassType();

			for (Component* comp : Components)
			{
				if (comp->GetType() == type)
				{
					return (T*) comp;
				}
			}

			return nullptr;
		}

		~Object();

	};

}
