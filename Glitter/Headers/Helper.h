#pragma once
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Helper
{
public:
	struct ShaderProgramSource
	{
		std::string VertexSource;
		std::string FragmentSource;
	};
	static unsigned int compileShader(unsigned int type, const std::string& source);

	static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader);

	static ShaderProgramSource ParseShader(const std::string& filepath);

	static void frame_buffer_size_callback(GLFWwindow* m_Window, int width, int height);

	static void switchOnWireFrame(bool isWireFrame);

	static void maxVertexAttributesAvailableOnMyGPU();

	static void unbindAllOpenGLObjects();
};

