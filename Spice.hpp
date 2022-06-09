
#include <iostream>
#include <stdio.h>
#include <random>
#include <filesystem>
#include <locale>
#include <codecvt>
#include <chrono>
#include <ctime>

#include "dep/lua544/include/lua.hpp"

#include "dep/imgui/imgui.h"
#include "dep/imgui/imgui_impl_glfw.h"
#include "dep/imgui/imgui_impl_opengl3.h"
#include "dep/imgui/imgui_stdlib.h"

#include "GLWrapper.hpp"
#include "Shape.hpp"
#include "Color.hpp"
#include "Vector.hpp"
#include "Object.hpp"
#include "Render.hpp"
#include "Utilities.hpp"
#include "EventHandler.hpp"
#include <functional>

namespace fs = std::filesystem;
constexpr float PI = 3.1415926f;
using namespace EventHandler;

static double xScroll = 0.0;
static double yScroll = 0.0;

fs::path DisplayPath(fs::path, bool _asMenu = false);
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

namespace Spice
{

	// all the different "special" keys needed for input. (that I could think of. or care enough to add)
	static const unsigned int
		ARROW_LEFT(GLFW_KEY_LEFT), ARROW_RIGHT(GLFW_KEY_RIGHT), ARROW_UP(GLFW_KEY_UP), ARROW_DOWN(GLFW_KEY_DOWN),
		SPACE(GLFW_KEY_SPACE), MOUSE_LEFT(GLFW_MOUSE_BUTTON_LEFT), MOUSE_MIDDLE(GLFW_MOUSE_BUTTON_RIGHT),
		MOUSE_RIGHT(GLFW_MOUSE_BUTTON_MIDDLE), LEFT_CONTROL(GLFW_KEY_LEFT_CONTROL), RIGHT_CONTROL(GLFW_KEY_RIGHT_CONTROL);

	struct InputState
	{
		bool Held = false;		// On at all times if KEY is pressed.
		bool Pressed = false;   // On only if key was pressed this frame.
		bool Released = false;  // On only if key was released this frame.
	};

	enum class EaseType
	{
		LINEAR,

		EASE_IN_SINE,
		EASE_OUT_SINE,
		EASE_IN_OUT_SINE,

		EASE_IN_QUAD,
		EASE_OUT_QUAD,
		EASE_IN_OUT_QUAD,

		EASE_IN_CUBIC,
		EASE_OUT_CUBIC,
		EASE_IN_OUT_CUBIC,

		EASE_IN_QUART,
		EASE_OUT_QUART,
		EASE_IN_OUT_QUART,

		EASE_IN_EXPO,
		EASE_OUT_EXPO,
		EASE_IN_OUT_EXPO,

		EASE_IN_BACK,
		EASE_OUT_BACK,
		EASE_IN_OUT_BACK,

		EASE_IN_ELASTIC,
		EASE_OUT_ELASTIC,
		EASE_IN_OUT_ELASTIC
	};

	void RegisterScript(lua_State* L, Object* _parent);

	class Engine
	{

	private:
		lua_State* m_SaveL;

		GLFWwindow* m_Window;
		std::string m_WindowTitle;
		Color m_BackgroundColor;

		std::unordered_map<unsigned int, Spice::InputState> m_InputMap;
		
		int m_WindowWidth;
		int m_WindowHeight;

		bool m_ShowGameView = true;
		bool m_ShowFileBrowser = true;
		bool m_ShowImGuiDemo = false;
		bool m_ShowObjectView = true;
		bool m_ShowProjectSettings = false;
		bool m_ShippingModeToggle = false;
		bool m_ResizeableWindow = true;

		bool m_ShippingMode = false;

		glm::vec2 m_GameViewPos = glm::vec2(0, 0);
		glm::vec2 m_GameViewContentMin = glm::vec2(0, 0);
		glm::vec2 m_GameViewContentMax = glm::vec2(0, 0);

		unsigned int m_GameFBO;
		unsigned int m_GameTXO;

		bool m_GameMode = false;

		float m_Delta = 0.0f;
		float m_TimeElapsed = 0.0f;
		float m_LastTime = 0.0f;
		float m_CurrentTime = 0.0f;

	public:

		Texture* pep;
		std::vector<Object*> Objects;

	private:

		void ShowMenuBar();
		void ShowFileBrowser();
		void ShowGameView();
		void ShowObjectView();
		void ShowProjectSettings();
		bool ShowParameter(Parameter& _parameter, bool editable, int index);

		static void ShowHelpMarker(std::string _desc);

		InputState GetKey(unsigned int key);
		glm::vec2 GetCursorPos();

		void RegisterMouseButton(char b);
		void RegisterKey(unsigned int k);
		void RegisterInput();

		bool Init();
		void Loop();

		void SaveEngine();
		void ReloadObjects();
		void ReloadProjectSettings(bool _init = false);
		void ResetViewport();

	public:

		static void lua_RegisterKeys(lua_State* L);

		static int lua_GetKey(lua_State* L);
		static int lua_GetCursorPos(lua_State* L);

		static void lua_RegisterInput(lua_State* L);

		float inline GetWindowWidth() { return m_WindowWidth; };
		float inline GetWindowHeight() { return m_WindowHeight; };
		glm::vec2 inline GetWindowCenter() { return glm::vec2(m_WindowWidth / 2.0f, m_WindowHeight / 2.0f); }

		float inline GetDelta() { return m_Delta; }
		float inline GetTimeElapsed() { return m_TimeElapsed; }

		static float Tween(float _v1, float _v2, float _progress, EaseType _type);

		static int lua_GetWindowWidth(lua_State* L);
		static int lua_GetWindowHeight(lua_State* L);
		static int lua_GetWindowCenter(lua_State* L);
		static int lua_GetDelta(lua_State* L);
		static int lua_GetTimeElapsed(lua_State* L);
		static int lua_Tween(lua_State* L);
		static int lua_SetBackgroundColor(lua_State* L);

		static int lua_RandomNew(lua_State* L);
		static int lua_RandomIndex(lua_State* L);
		static int lua_RandomInteger(lua_State* L);
		static int lua_RandomNumber(lua_State* L);
		static void lua_RegisterRandom(lua_State* L);

		static int lua_FindObject(lua_State* L);
		static int lua_FindObjects(lua_State* L);
		
		static void lua_RegisterEaseTypes(lua_State* L);
		static void lua_RegisterHelpers(lua_State* L);

		void Run(int w, int h, std::string title = "Spice");
	};

}
