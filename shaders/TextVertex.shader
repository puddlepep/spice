R"(

#version 330 core

layout(location = 0) in vec4 Position;
layout(location = 1) in vec2 aTexCoord;

uniform mat4 u_MVP;
out vec2 TexCoord;

void main()
{
	gl_Position = u_MVP * Position;
	TexCoord = aTexCoord;
};

)"