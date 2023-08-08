#pragma once
#include <glad/glad.h>
#include <string>

class Shader {

public:
	void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType);

	void CompileShaders();

	bool ReadFile(const char* pFileName, std::string& outFile);

	GLuint Model;
	GLuint View;
	GLuint Projection;
	GLuint SamplerLocation;
};