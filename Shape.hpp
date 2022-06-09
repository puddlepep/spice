#pragma once
#include "GLWrapper.hpp"
#include "Color.hpp"
#include "Vector.hpp"

namespace Spice
{

	class Shape
	{

	private:

		VertexBuffer* m_VBuffer;
		std::vector<glm::vec2> m_Points;

		float m_Width;
		float m_Height;

	public:

		Shape(std::vector<glm::vec2> points);
		~Shape();

		void Reform(std::vector<glm::vec2> points);

		inline VertexBuffer* GetBuffer() const { return m_VBuffer; }
		inline std::vector<glm::vec2> GetPoints() const { return m_Points; }
		inline float GetWidth() const { return m_Width; }
		inline float GetHeight() const { return m_Height; }

		static int lua_Shape(lua_State* L);
		static int lua_Reform(lua_State* L);
		static int lua_GetPoints(lua_State* L);

		static int lua_Index(lua_State* L);
		static int lua_ToString(lua_State* L);

		static void lua_RegisterShape(lua_State* L);

	};


}
