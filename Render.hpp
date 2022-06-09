#pragma once

#include "Shape.hpp"
#include "dep/fonts/oldschool_font.h"

namespace Spice
{

	class Camera
	{
	private:
		int m_ViewportWidth;
		int m_ViewportHeight;


	public:

		glm::vec2 ZoomOrigin;
		glm::vec3 Position;
		float Rotation;
		float Zoom;

		Camera(int _w, int _h, glm::vec3 _pos = { 0, 0, 0 }, float _rot = 0.0f) :
			Position(_pos), Rotation(_rot), m_ViewportWidth(_w), m_ViewportHeight(_h), Zoom(1.0f), ZoomOrigin({ m_ViewportWidth/2, m_ViewportHeight/2 }) {}

		glm::mat4 inline GetProjMatrix() { return glm::ortho(0.0f, (float)m_ViewportWidth, 0.0f, (float)m_ViewportHeight); }
		glm::mat4 inline GetViewMatrix() { return glm::translate(glm::translate(glm::scale(glm::rotate(glm::translate(glm::mat4(1), glm::vec3(ZoomOrigin.x, ZoomOrigin.y, 0)), Rotation, glm::vec3(0, 0, 1)), { Zoom, Zoom, 1 }), Position), glm::vec3(-ZoomOrigin.x, -ZoomOrigin.y, 0)); }

		void ResetViewport(int _w, int _h) { m_ViewportWidth = _w; m_ViewportHeight = _h; }
		glm::vec2 inline GetViewport() { return glm::vec2(m_ViewportWidth, m_ViewportHeight); }
	};

	class Render
	{

	public:

		static Camera* MainCamera;
		static Camera* EditorCamera;

		static Shader* DefaultShader;
		static Shader* TextureShader;

		static void DrawShape(Camera _camera, Shape& _shape, Shader* _shader, glm::vec2 _pos, float _rotation = 0, glm::vec2 _scale = { 1, 1 }, bool _filled = true, Spice::Color _color = Spice::WHITE, float _lineWidth = 1.0f);
		static void DrawSprite(Camera _camera, Texture& _texture, Shader* _shader, glm::vec2 _pos, glm::vec2 _scale = { 1, 1 }, Spice::Color _modulation = Spice::WHITE);
		static void DrawString(Camera _camera, std::string _str, Shader* _shader, glm::vec2 _pos, glm::vec2 _scale = { 1, 1 }, Spice::Color _modulation = Spice::WHITE);
		static void DrawLine(Camera _camera, glm::vec2 _p1, glm::vec2 _p2, Shader* _shader, Spice::Color _modulation = Spice::WHITE, float _lw = 1.0f);

		static void Setup(int _w, int _h);
		static void ResetViewport(int _w, int _h);

	};

}
