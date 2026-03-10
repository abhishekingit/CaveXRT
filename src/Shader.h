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
	~Shader() = default;
	void use() const;
	void reloadShaders();

	void setBool(const char* uniformName, bool value) const;
	void setInt(const char* uniformName, int value) const;
	void setFloat(const char* uniformName, float value) const;
	void setVec2(const char* uniformName, const glm::vec2& vec) const;
	void setVec3(const char* uniformName, const glm::vec3& vec) const;
	void setVec3(const char* uniformName, float x, float y, float z) const;
	void setVec4(const char* uniformName, const glm::vec4& vec) const;
	void setMat4(const char* uniformName, const glm::mat4& mat) const;
	void setMat3(const char* uniformName, const glm::mat3& mat) const;

private:
	uint32_t ShaderID;
	const char* vertexShaderPath;
	const char* fragmentShaderPath;

	void compileAndLinkShaders();
	


};