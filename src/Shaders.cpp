#pragma once

#include "Shader.h"

Shader::Shader(const char* vertexShaderPath, const char* fragmentShaderPath): vertexShaderPath(vertexShaderPath), fragmentShaderPath(fragmentShaderPath), geometryShaderPath(nullptr), tessControlShaderPath(nullptr), tessEvalShaderPath(nullptr) {
	compileAndLinkShaders();
}

Shader::Shader(const char* vertexShaderPath, const char* fragmentShaderPath, const char* geometryShaderPath) : vertexShaderPath(vertexShaderPath), fragmentShaderPath(fragmentShaderPath), geometryShaderPath(geometryShaderPath), tessControlShaderPath(nullptr), tessEvalShaderPath(nullptr) {
	compileAndLinkShaders();
}

Shader::Shader(const char* vertexShaderPath, const char* fragmentShaderPath, const char* geometryShaderPath, const char* tessControlShaderPath, const char* tessEvalShaderPath) : vertexShaderPath(vertexShaderPath), fragmentShaderPath(fragmentShaderPath), geometryShaderPath(geometryShaderPath), tessControlShaderPath(tessControlShaderPath), tessEvalShaderPath(tessEvalShaderPath) {
	compileAndLinkShaders();
}

void Shader::compileAndLinkShaders() {
	std::string vertexShaderCode;
	std::string fragmentShaderCode;
	std::string geometryShaderCode;
	std::string tessControlShaderCode;
	std::string tessEvalShaderCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;
	std::ifstream gShaderFile;
	std::ifstream tControlShaderFile;
	std::ifstream tEvalShaderFile;

	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	tControlShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	tEvalShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try {
		vShaderFile.open(this->vertexShaderPath);
		fShaderFile.open(this->fragmentShaderPath);
		if(geometryShaderPath != nullptr) gShaderFile.open(this->geometryShaderPath);
		//need better checks for tessellation shader paths, but this will do for now
		if(tessControlShaderPath != nullptr) tControlShaderFile.open(this->tessControlShaderPath);
		if(tessEvalShaderPath != nullptr) tEvalShaderFile.open(this->tessEvalShaderPath);


		std::stringstream vertexShaderStream, fragmentShaderStream, geometryShaderStream, tControlShaderStream, tEvalShaderStream;
		vertexShaderStream << vShaderFile.rdbuf();
		fragmentShaderStream << fShaderFile.rdbuf();
		if (geometryShaderPath != nullptr) geometryShaderStream << gShaderFile.rdbuf();
		if (tessControlShaderPath != nullptr) tControlShaderStream << tControlShaderFile.rdbuf();
		if (tessEvalShaderPath != nullptr) tEvalShaderStream << tEvalShaderFile.rdbuf();

		vShaderFile.close();
		fShaderFile.close();
		if (geometryShaderPath != nullptr) gShaderFile.close();
		if (tessControlShaderPath != nullptr) tControlShaderFile.close();
		if (tessEvalShaderPath != nullptr) tEvalShaderFile.close();

		vertexShaderCode = vertexShaderStream.str();
		fragmentShaderCode = fragmentShaderStream.str();
		if (geometryShaderPath != nullptr) geometryShaderCode = geometryShaderStream.str();
		if (tessControlShaderPath != nullptr) tessControlShaderCode = tControlShaderStream.str();
		if (tessEvalShaderPath != nullptr) tessEvalShaderCode = tEvalShaderStream.str();
	}
	catch (std::ifstream::failure e) {
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
	}

	const char* vShaderCode = vertexShaderCode.c_str();
	const char* fShaderCode = fragmentShaderCode.c_str();

	uint32_t vertexShaderID, fragmentShaderID;
	uint32_t geometryShaderID = 0;
	uint32_t tessControlShaderID = 0;
	uint32_t tessEvalShaderID = 0;
	int compileSuccess, linkSuccess;
	char infoLog[512];


	if (geometryShaderPath != nullptr) {
		const char* gShaderCode = geometryShaderCode.c_str();
		geometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometryShaderID, 1, &gShaderCode, NULL);
		glCompileShader(geometryShaderID);

		glGetShaderiv(geometryShaderID, GL_COMPILE_STATUS, &compileSuccess);
		if (!compileSuccess) {
			glGetShaderInfoLog(geometryShaderID, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::GEOMETRY::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
	}

	if (tessControlShaderPath != nullptr && tessEvalShaderPath != nullptr) {
		const char* tControlShaderCode = tessControlShaderCode.c_str();
		const char* tEvalShaderCode = tessEvalShaderCode.c_str();
		tessControlShaderID = glCreateShader(GL_TESS_CONTROL_SHADER);
		glShaderSource(tessControlShaderID, 1, &tControlShaderCode, NULL);
		glCompileShader(tessControlShaderID);
		glGetShaderiv(tessControlShaderID, GL_COMPILE_STATUS, &compileSuccess);
		if (!compileSuccess) {
			glGetShaderInfoLog(tessControlShaderID, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::TESS_CONTROL::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
		tessEvalShaderID = glCreateShader(GL_TESS_EVALUATION_SHADER);
		glShaderSource(tessEvalShaderID, 1, &tEvalShaderCode, NULL);
		glCompileShader(tessEvalShaderID);
		glGetShaderiv(tessEvalShaderID, GL_COMPILE_STATUS, &compileSuccess);
		if (!compileSuccess) {
			glGetShaderInfoLog(tessEvalShaderID, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::TESS_EVAL::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
	}


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

	if (geometryShaderID != 0) glAttachShader(ShaderID, geometryShaderID);
	if (tessControlShaderID != 0) glAttachShader(ShaderID, tessControlShaderID);
	if (tessEvalShaderID != 0) glAttachShader(ShaderID, tessEvalShaderID);

	glLinkProgram(ShaderID);

	glGetProgramiv(ShaderID, GL_LINK_STATUS, &linkSuccess);
	if (!linkSuccess) {
		glGetProgramInfoLog(ShaderID, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);
	if (geometryShaderID != 0) glDeleteShader(geometryShaderID);
	if (tessControlShaderID != 0) glDeleteShader(tessControlShaderID);
	if (tessEvalShaderID != 0) glDeleteShader(tessEvalShaderID);

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

void Shader::setVec2(const char* uniformName, const glm::vec2& vec) const {
	glUniform2fv(glGetUniformLocation(ShaderID, uniformName), 1, &vec[0]);
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

void Shader::setMat3(const char* uniformName, const glm::mat3& mat) const {
	glUniformMatrix3fv(glGetUniformLocation(ShaderID, uniformName), 1, GL_FALSE, &mat[0][0]);
}