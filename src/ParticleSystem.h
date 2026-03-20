#pragma once

#include "CaveCompute.h"
#include "Shader.h"
#include <vector>

struct Particle {
	float positionX;
	float positionY;
	float positionZ;
	float velocityX;
	float velocityY;
	float velocityZ;
};

class ParticleSystem {
public:
	ParticleSystem(size_t maxParticles, float simRadius, const glm::vec3& gridMin, const glm::vec3& gridMax, const char* computeShaderPath, const char* vertexShaderPath, const char* fragmentShaderPath);
	~ParticleSystem();
	void Update(float deltaTime, float wallDamping);
	void Render(const glm::mat4& mvp, const glm::vec3& particleColor, float pointSize, const glm::vec2& viewportSize, const glm::mat4& projection, const glm::mat4& view, const glm::vec3& lightWorldPos);

private:
	size_t maxParticles;
	std::vector<Particle> particles;
	float PARTICLE_SIM_RADIUS = 0.02f;
	float GRID_CELL_SIZE = PARTICLE_SIM_RADIUS * 4.0f;
	glm::vec3 GRID_MIN;
	glm::vec3 GRID_MAX;
	glm::vec3 GRID_SIZE;
	glm::ivec3 GRID_RES;
	uint32_t GRID_VOXEL_COUNT;

	uint32_t ssboPos = 0;
	uint32_t ssboVel = 0;
	uint32_t vao = 0;


	uint32_t ssboCellCount = 0;
	uint32_t ssboCellOffset = 0;
	uint32_t ssboCellWrite = 0;
	uint32_t ssboParticleCell = 0;
	uint32_t ssboSortedIndex = 0;

	const uint32_t workGroupSize = 256;


	Shader* renderShader = nullptr;
	CaveCompute* computeProgram = nullptr;
	CaveCompute* gridClearProgram = nullptr;
	CaveCompute* gridParticleCountProgram = nullptr;
	CaveCompute* gridParticleReorderProgram = nullptr;


	void InitializeGrid();
	void BuildUniformGrid();
	void InitializeParticles();
};