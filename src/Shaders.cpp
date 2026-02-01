#pragma once

#include "Shader.h"

Shader::Shader(const char* vertexShaderPath, const char* fragmentShaderPath): vertexShaderPath(vertexShaderPath), fragmentShaderPath(fragmentShaderPath) {
	compileAndLinkShaders();
}

void Shader::compileAndLinkShaders() {
	std::string vertexShaderCode;
	std::string fragmentShaderCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;

	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try {
		vShaderFile.open(this->vertexShaderPath);
		fShaderFile.open(this->fragmentShaderPath);

		std::stringstream vertexShaderStream, fragmentShaderStream;
		vertexShaderStream << vShaderFile.rdbuf();
		fragmentShaderStream << fShaderFile.rdbuf();

		vShaderFile.close();
		fShaderFile.close();

		vertexShaderCode = vertexShaderStream.str();
		fragmentShaderCode = fragmentShaderStream.str();

	}
	catch (std::ifstream::failure e) {
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
	}

	const char* vShaderCode = vertexShaderCode.c_str();
	const char* fShaderCode = fragmentShaderCode.c_str();

	uint32_t vertexShaderID, fragmentShaderID;
	int compileSuccess, linkSuccess;
	char infoLog[512];


	vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderID, 1, &vShaderCode, NULL);
	glCompileShader(vertexShaderID);

	glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &compileSuccess);
	if (!compileSuccess) {
		glGetShaderInfoLog(vertexShaderID, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderID, 1, &fShaderCode, NULL);
	glCompileShader(fragmentShaderID);

	glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &compileSuccess);
	if (!compileSuccess) {
		glGetShaderInfoLog(fragmentShaderID, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	ShaderID = glCreateProgram();
	glAttachShader(ShaderID, vertexShaderID);
	glAttachShader(ShaderID, fragmentShaderID);
	glLinkProgram(ShaderID);

	glGetProgramiv(ShaderID, GL_LINK_STATUS, &linkSuccess);
	if (!linkSuccess) {
		glGetProgramInfoLog(ShaderID, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);

}

void Shader::reloadShaders() {
	glDeleteProgram(this->ShaderID);
	std::cout << "Reloading Shaders .." << std::endl;
	this->compileAndLinkShaders();
}

void Shader::use() const {
	glUseProgram(ShaderID);
}

void Shader::setBool(const char* uniformName, bool value) const{
	glUniform1i(glGetUniformLocation(ShaderID, uniformName), (int)value);
}

void Shader::setInt(const char* uniformName, int value) const {
	glUniform1i(glGetUniformLocation(ShaderID, uniformName), value);
}

void Shader::setFloat(const char* uniformName, float value) const {
	glUniform1f(glGetUniformLocation(ShaderID, uniformName), value);
}

void Shader::setVec3(const char* uniformName, const glm::vec3& vec) const {
	glUniform3fv(glGetUniformLocation(ShaderID, uniformName), 1, &vec[0]);
}

void Shader::setVec3(const char* uniformName, float x, float y, float z) const {
	glUniform3f(glGetUniformLocation(ShaderID, uniformName), x, y, z);
}

void Shader::setMat4(const char* uniformName, const glm::mat4& mat) const {
	glUniformMatrix4fv(glGetUniformLocation(ShaderID, uniformName), 1, GL_FALSE, &mat[0][0]);
}