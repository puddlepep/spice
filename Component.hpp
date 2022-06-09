#pragma once
#include <vector>
#include <unordered_map>

#include "Render.hpp"
#include "Vector.hpp"
#include "LuaWrapper.hpp"
#include "Utilities.hpp"
#include <map>
#include <limits>

namespace Spice
{
	class Object;

	enum class DataType
	{
		None,
		Bool,
		Float,
		Vec2,
		Vec2Array,
		Color,
		String,
		Filepath,

		Separator,         // no data required
		Vec2ArrayRenderer , // vector of vec2s required

	};

	static std::map<DataType, std::string> DataTypeNames =
	{
		{DataType::None, "none"},
		{DataType::Bool, "bool"},
		{DataType::Float, "float"},
		{DataType::Vec2, "vec2"},
		{DataType::Vec2Array, "vec2array"},
		{DataType::Color, "color"},
		{DataType::String, "string"},
		{DataType::Filepath, "filepath"}
	};

	struct DataSettings
	{
		float DragSpeed;
		float MinimumLimit;
		float MaximumLimit;

		bool KillData;

		DataSettings(bool _killData, float _dragSpeed = 0.01f, float _minLimit = -std::numeric_limits<float>::max(), float _maxLimit = std::numeric_limits<float>::max()) :
			DragSpeed(_dragSpeed), MinimumLimit(_minLimit), MaximumLimit(_minLimit),
			KillData(_killData) {}

	};

	struct Parameter
	{
		std::string Name;
		DataType Type;
		DataSettings Settings;
		void* Data;

		Parameter() :
			Name(""), Type(DataType::None), Settings(DataSettings(true)), Data(nullptr) {}

		Parameter(std::string _name, DataType _type, void* _data, DataSettings _dataSettings, bool _killData = true) :
			Name(_name), Type(_type), Data(_data), Settings(_dataSettings) {}

		static Parameter Deserialize(lua_State* L);
		void Serialize(lua_State* L);
		void lua_PushData(lua_State* L, bool _makeUsable = false);
		void lua_TrySetData(lua_State* L);
		static Parameter lua_TryCreateParameter(lua_State* L);
	};

	class Component
	{

	public:

		std::vector<Parameter> Parameters;
		Object* Parent = nullptr;

		virtual std::string GetType();
		static std::string GetClassType();
		~Component();

		virtual void OnTick(bool _onGame, bool _onEditor) { };
		virtual void Callback() { };

		template<DataType T>
		void AddParameter(std::string _name, void* _data, DataSettings _settings, bool _killData = true) 
		{ 
			Parameters.push_back(Parameter(_name, T, _data, _settings, _killData));
		}

		virtual void Serialize(lua_State* L) { };
		virtual void Deserialize(lua_State* L) { };

	};

	class LuaComponent
	{
	public:

		LuaComponent(Component* _component);
		~LuaComponent();

		Component* Comp = nullptr;

		static int lua_Index(lua_State* L);
		static int lua_NewIndex(lua_State* L);
		static int lua_ToString(lua_State* L);

		static int lua_New(lua_State* L, Component* _component);
		static void lua_RegisterComponent(lua_State* L);

		static int lua_NewVec2Array(lua_State* L, std::vector<glm::vec2>* _points);
		static int lua_Vec2ArrayIndex(lua_State* L);
		static int lua_Vec2ArrayNewIndex(lua_State* L);
		static int lua_Vec2ArrayLen(lua_State* L);

	};

	class Transform : public Component
	{
		
	public:

		glm::vec2* Position;
		glm::vec2* Scale;
		float* Rotation;

		Transform(Object* _parent);

		static std::string GetClassType();
		std::string GetType() override;

		void Serialize(lua_State* L) override;
		void Deserialize(lua_State* L) override;

	};

	class ShapeRenderer : public Component
	{

	private:

		std::vector<glm::vec2>* m_Points;

	public:

		Shape* ShapeData;
		bool* Visible;
		bool* LineMode;
		float* LineWidth;
		Color* ColorData;
		float* Layer;

		ShapeRenderer(Object* _parent);

		static std::string GetClassType();
		std::string GetType() override;

		void OnTick(bool _onGame, bool _onEditor) override;

		void Serialize(lua_State* L) override;
		void Deserialize(lua_State* L) override;
	};

	class ScriptComponent : public Component
	{
	public:

		Script* ScriptData;

		ScriptComponent(Object* _parent);

		static std::string GetClassType();
		std::string GetType() override;

		void OnTick(bool _onGame, bool _onEditor) override;
		void Callback() override;

		void Serialize(lua_State* L) override;
		void Deserialize(lua_State* L) override;

	};

	class TextRenderer : public Component
	{
	public:

		bool* Visible;
		std::string* Text;
		glm::vec2* Offset;
		Color* ColorData;

		TextRenderer(Object* _parent);

		static std::string GetClassType();
		std::string GetType() override;

		void OnTick(bool _onGame, bool _onEditor) override;

		void Serialize(lua_State* L) override;
		void Deserialize(lua_State* L) override;
	};

}
