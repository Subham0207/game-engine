#vertex shader
#version 330 core
layout(location = 0) in vec3 apos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(apos, 1.0);
}

#fragment shader
#version 330 core

out vec4 fragColor;

void main()
{
	fragColor = vec4(1.0);
}