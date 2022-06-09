#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <iomanip>
#include <iostream>

#include "dep/glm/glm.hpp"
#include "dep/glm/gtc/matrix_transform.hpp"
#include "dep/glm/gtx/rotate_vector.hpp"
#include "dep/stb_image/stb_image.h"


// Error Logging //
#define ASSERT(x) if (!(x)) __debugbreak();
#define GLCall(x) GLClearError();\
    x;\
    ASSERT(GLLogCall(#x, __FILE__, __LINE__))    

void GLClearError();
bool GLLogCall(const char* function, const char* file, int line);
// ------------- //

class Shader
{

private:
	unsigned int m_ID;

public:
	Shader(const std::string& vertexSource, const std::string& fragmentSource);

	unsigned int CreateShader(const std::string& vertexSource, const std::string& fragmentSource);
	unsigned int CompileShader(unsigned int type, const std::string& source);
	std::string ParseShader(const std::string& filepath);

	void Bind();
	void Unbind();

	void SetUniform1i(const std::string& name, int value);
	void SetUniform1f(const std::string& name, float value);
	void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3);
	void SetUniformMat4f(const std::string& name, const glm::mat4 matrix);

	int GetUniformLocation(const std::string& name);
};

class Texture
{
private:
	unsigned int m_ID;
	std::string m_Filepath;
	unsigned char* m_LocalBuffer;
	int m_Width, m_Height, m_BPP;

public:

	Texture(std::string filepath);
	Texture(const stbi_uc* bytes, int bytesLen);
	~Texture();

	void Bind(unsigned int slot = 0);
	void Unbind();

	int inline GetWidth() { return m_Width; }
	int inline GetHeight() { return m_Height; }
	int inline GetID() { return m_ID; }

};

class VertexBuffer
{
private:
	unsigned int m_ID;

public:

	VertexBuffer(const void* data, unsigned int size);
	~VertexBuffer();

	void NewData(const void* data, unsigned int size);

	void Bind();
	void Unbind();

};
