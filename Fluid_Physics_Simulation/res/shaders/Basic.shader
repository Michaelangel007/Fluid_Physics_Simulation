#shader vertex
#version 330 core

layout (location = 0) in vec4 position;
uniform vec4 u_pos;

void main ()
{
	vec4 pos = position;
	pos += u_pos;
    gl_Position = pos;
};

#shader fragment
#version 330 core

layout (location = 0) out vec4 color;

uniform vec3 u_Color;

void main ()
{
	color = vec4(u_Color, 1.0f);
};