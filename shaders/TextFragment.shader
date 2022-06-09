R"(

#version 330 core

layout(location = 0) out vec4 color;

uniform vec4 u_Color;
in vec2 TexCoord;

uniform sampler2D tex;

void main()
{
	vec4 texColor = texture(tex, TexCoord);
	color = texColor * u_Color;
};

)"