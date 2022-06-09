#pragma once

#include "LuaWrapper.hpp"

namespace Spice
{
	enum class ColorType
	{
		RGB,
		HSV
	};

	class Color
	{
	public:

		float r, g, b, a;

		Color();
		Color(float _v1, float _v2, float _v3, float _a = 1.0f, ColorType type = ColorType::RGB);

		bool operator!=(const Color& c);
		bool operator==(const Color& c);

		static Color Lerp(Color color, Color target, float t);
		static Color FromHSV(float _h, float _s, float _v, float _a = 1.0f);
		static Color FromRGB(float _r, float _g, float _b, float _a = 1.0f);
	};

	class ColorManager
	{
	private:
		int m_Instances;
		bool m_KillColor;

	public:

		Color* ColorPtr;

		ColorManager(float _r, float _g, float _b, float _a = 1.0f);
		ColorManager(Color* _col, bool _killColor);

		void IncInstance();
		void DecInstance();

	};

	class LuaColor
	{
	public:

		ColorManager* Manager;
		Color* ColorPtr;

		LuaColor(ColorManager* _manager);
		LuaColor(Color* _color, bool _killColor);
		LuaColor(float _r, float _g, float _b, float _a = 1.0f);
		~LuaColor();

		static int lua_FromColor(lua_State* L, Color* _color, bool _killColor);
		static int lua_FromManager(lua_State* L, ColorManager* _manager);

		static int lua_FromRGB(lua_State* L);
		static int lua_FromHSV(lua_State* L);
		static int lua_Clone(lua_State* L);

		static int lua_Index(lua_State* L);
		static int lua_NewIndex(lua_State* L);
		static int lua_ToString(lua_State* L);
		static int lua_Mul(lua_State* L);
		static int lua_GC(lua_State* L);

		static void lua_RegisterColor(lua_State* L);
	};

	// A bunch of color constants. You can either use these, or construct
	// your own colors with Spice::Color(r, g, b, a) (0.0f-1.0f range, or 0i-255i range)
	static const Color
		GREY(0.75f, 0.75f, 0.75f), DARK_GREY(0.5f, 0.5f, 0.5f), VERY_DARK_GREY(0.25f, 0.25f, 0.25f),
		RED(1.0f, 0.0f, 0.0f), DARK_RED(0.5f, 0.0f, 0.0f), VERY_DARK_RED(0.25f, 0.0f, 0.0f),
		YELLOW(1.0f, 1.0f, 0.0f), DARK_YELLOW(0.5f, 0.5f, 0.0f), VERY_DARK_YELLOW(0.25f, 0.25f, 0.0f),
		GREEN(0.0f, 1.0f, 0.0f), DARK_GREEN(0.0f, 0.5f, 0.0f), VERY_DARK_GREEN(0.0f, 0.25f, 0.0f),
		CYAN(0.0f, 1.0f, 1.0f), DARK_CYAN(0.0f, 0.5f, 0.5f), VERY_DARK_CYAN(0.0f, 0.25f, 0.25f),
		BLUE(0.0f, 0.0f, 1.0f), DARK_BLUE(0.0f, 0.0f, 0.5f), VERY_DARK_BLUE(0.0f, 0.0f, 0.25f),
		MAGENTA(1.0f, 0.0f, 1.0f), DARK_MAGENTA(0.5f, 0.0f, 0.5f), VERY_DARK_MAGENTA(0.25f, 0.0f, 0.25f),
		WHITE(1.0f, 1.0f, 1.0f), BLACK(0.0f, 0.0f, 0.0f), BLANK(0.0f, 0.0f, 0.0f, 0.0f);
}
