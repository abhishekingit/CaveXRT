#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

class Shader {
public:
	Shader(const char* vertexShaderPath, const char* fragmentShaderPath);
	void use() const;
	void reloadShaders();

	void setBool(const char* uniformName, bool value) const;
	void setInt(const char* uniformName, int value) const;
	void setFloat(const char* uniformName, float value) const;
	void setMat4(const char* uniformName, const glm::mat4& mat) const;

private:
	uint32_t ShaderID;
	const char* vertexShaderPath;
	const char* fragmentShaderPath;

	void compileAndLinkShaders();
	


};