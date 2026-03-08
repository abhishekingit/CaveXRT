#pragma once

#include "CaveCompute.h"

CaveCompute::CaveCompute(const char* computeShaderPath) : computeShaderPath(computeShaderPath) {
	compileComputeShader();
}

void CaveCompute::compileComputeShader() {
	std::string computeShaderCode;
	std::ifstream computeShaderFile;

	computeShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try {
		computeShaderFile.open(this->computeShaderPath);
		std::stringstream computeShaderStream;

		computeShaderStream << computeShaderFile.rdbuf();

		computeShaderFile.close();

		computeShaderCode = computeShaderStream.str();
	}
	catch (std::ifstream::failure e) {
		std::cout << "ERROR::COMPUTE::FILE_NOT_SUCCESSFULLY_READ" << std::endl;

	}

	const char* computeCode = computeShaderCode.c_str();

	uint32_t computeShaderID;
	int compileSuccess, linkSuccess;
	char infoLog[512];

	computeShaderID = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(computeShaderID, 1, &computeCode, nullptr);
	glCompileShader(computeShaderID);

	glGetShaderiv(computeShaderID, GL_COMPILE_STATUS, &compileSuccess);
	if (!compileSuccess) {
		glGetShaderInfoLog(computeShaderID, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::COMPUTE::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	computeShaderProgramID = glCreateProgram();
	glAttachShader(computeShaderProgramID, computeShaderID);
	glLinkProgram(computeShaderProgramID);

	glGetProgramiv(computeShaderProgramID, GL_LINK_STATUS, &linkSuccess);
	if (!linkSuccess) {
		glGetProgramInfoLog(computeShaderProgramID, 512, NULL, infoLog);
		std::cout << "ERROR::COMPUTE::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}

	glDeleteShader(computeShaderID);


}

void CaveCompute::use() const {
	if(computeShaderProgramID) glUseProgram(computeShaderProgramID);
}

void CaveCompute::dispatch(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ) const {
	if (!computeShaderProgramID) return;
	glDispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
}

void CaveCompute::setUint(const char* uniformName, uint32_t value) const {
	glUniform1ui(glGetUniformLocation(computeShaderProgramID, uniformName), value);
}

void CaveCompute::setFloat(const char* uniformName, float value) const {
	glUniform1f(glGetUniformLocation(computeShaderProgramID, uniformName), value);
}

void CaveCompute::setVec3(const char* uniformName, const glm::vec3& vec) const {
	glUniform3fv(glGetUniformLocation(computeShaderProgramID, uniformName), 1, &vec[0]);
}