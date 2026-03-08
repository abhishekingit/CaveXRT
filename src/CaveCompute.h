#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

class CaveCompute {
public:
	CaveCompute(const char* computeShaderPath);
	~CaveCompute() = default;

	void use() const;
	void dispatch(uint32_t numGroupsX, uint32_t numGroupsY = 1, uint32_t numGroupsZ = 1) const;

	void setUint(const char* uniformName, uint32_t value) const;
	void setFloat(const char* uniformName, float value) const;
	void setVec3(const char* uniformName, const glm::vec3& vec) const;

private:
	uint32_t computeShaderProgramID;
	const char* computeShaderPath;

	void compileComputeShader();
};