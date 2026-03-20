#pragma once

#include <random>
#include <glm/glm.hpp>
#include "ParticleSystem.h"



ParticleSystem::ParticleSystem(size_t maxParticles, float simRadius, const glm::vec3& gridMin, const glm::vec3& gridMax, const char* computeShaderPath, const char* vertexShaderPath, const char* fragmentShaderPath) : maxParticles(maxParticles), PARTICLE_SIM_RADIUS(simRadius), GRID_MIN(gridMin), GRID_MAX(gridMax) {
	particles.resize(maxParticles);
	computeProgram = new CaveCompute(computeShaderPath);
	renderShader = new Shader(vertexShaderPath, fragmentShaderPath);

	gridClearProgram = new CaveCompute("../../../src/Shaders/Particles/gridclear.comp");
	gridParticleCountProgram = new CaveCompute("../../../src/Shaders/Particles/gridparticlecount.comp");
	gridParticleReorderProgram = new CaveCompute("../../../src/Shaders/Particles/gridparticlesort.comp");

	InitializeGrid();
	InitializeParticles();
}

ParticleSystem::~ParticleSystem() {
	if (ssboPos) {
		glDeleteBuffers(1, &ssboPos);
		ssboPos = 0;
	}

	if (ssboVel) {
		glDeleteBuffers(1, &ssboVel);
		ssboVel = 0;
	}

	if (vao) {
		glDeleteVertexArrays(1, &vao);
		vao = 0;
	}

	if (computeProgram) {
		delete computeProgram;
		computeProgram = nullptr;
	}
	if (renderShader) {
		delete renderShader;
		renderShader = nullptr;
	}
}

void ParticleSystem::InitializeGrid() {
	GRID_CELL_SIZE = PARTICLE_SIM_RADIUS * 4.0f;
	GRID_SIZE = GRID_MAX - GRID_MIN;
	GRID_RES = glm::ivec3(glm::max(glm::vec3(1.0f), glm::ceil(GRID_SIZE / GRID_CELL_SIZE)));
	GRID_VOXEL_COUNT = static_cast<uint32_t>(GRID_RES.x * GRID_RES.y * GRID_RES.z);

	if (ssboCellCount == 0) glGenBuffers(1, &ssboCellCount);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCellCount);
	glBufferData(GL_SHADER_STORAGE_BUFFER, GRID_VOXEL_COUNT * sizeof(uint32_t), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssboCellCount);

	if(ssboCellOffset == 0) glGenBuffers(1, &ssboCellOffset);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCellOffset);
	glBufferData(GL_SHADER_STORAGE_BUFFER, GRID_VOXEL_COUNT * sizeof(uint32_t), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssboCellOffset);

	if (ssboCellWrite == 0) glGenBuffers(1, &ssboCellWrite);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCellWrite);
	glBufferData(GL_SHADER_STORAGE_BUFFER, GRID_VOXEL_COUNT * sizeof(uint32_t), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ssboCellWrite);

	if (ssboParticleCell == 0) glGenBuffers(1, &ssboParticleCell);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboParticleCell);
	glBufferData(GL_SHADER_STORAGE_BUFFER, maxParticles * sizeof(uint32_t), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, ssboParticleCell);

	if (ssboSortedIndex == 0) glGenBuffers(1, &ssboSortedIndex);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboSortedIndex);
	glBufferData(GL_SHADER_STORAGE_BUFFER, maxParticles * sizeof(uint32_t), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, ssboSortedIndex);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

}

void ParticleSystem::BuildUniformGrid() {
	auto divUp = [](uint32_t a, uint32_t b) { return (a + b - 1u) / b; };

	gridClearProgram->use();
	gridClearProgram->setUint("gridResX", static_cast<uint32_t>(GRID_RES.x));
	gridClearProgram->setUint("gridResY", static_cast<uint32_t>(GRID_RES.y));
	gridClearProgram->setUint("gridResZ", static_cast<uint32_t>(GRID_RES.z));
	gridClearProgram->dispatch(divUp(static_cast<uint32_t>(GRID_RES.x), 4u), divUp(static_cast<uint32_t>(GRID_RES.y), 4u), divUp(static_cast<uint32_t>(GRID_RES.z), 4u));
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);



	gridParticleCountProgram->use();
	gridParticleCountProgram->setUint("particleCount", static_cast<uint32_t>(maxParticles));
	gridParticleCountProgram->setVec3("gridMin", GRID_MIN);
	gridParticleCountProgram->setFloat("cellSize", GRID_CELL_SIZE);
	gridParticleCountProgram->setUint("gridResX", static_cast<uint32_t>(GRID_RES.x));
	gridParticleCountProgram->setUint("gridResY", static_cast<uint32_t>(GRID_RES.y));
	gridParticleCountProgram->setUint("gridResZ", static_cast<uint32_t>(GRID_RES.z));


	uint32_t particleGroups = divUp(static_cast<uint32_t>(maxParticles), 64u);

	gridParticleCountProgram->dispatch(particleGroups, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	std::vector<uint32_t> counts(GRID_VOXEL_COUNT);
	std::vector<uint32_t> offsets(GRID_VOXEL_COUNT);
	uint32_t particleProcessed = 0;

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCellCount);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, counts.size() * sizeof(uint32_t), counts.data());

	for (uint32_t i = 0; i < GRID_VOXEL_COUNT; i++) {
		offsets[i] = particleProcessed;
		particleProcessed += counts[i];
	}

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCellOffset);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, offsets.size() * sizeof(uint32_t), offsets.data());

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCellWrite);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, offsets.size() * sizeof(uint32_t), offsets.data());

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	gridParticleReorderProgram->use();
	gridParticleReorderProgram->setUint("particleCount", static_cast<uint32_t>(maxParticles));
	gridParticleReorderProgram->dispatch(particleGroups, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

}

void ParticleSystem::InitializeParticles() {
	std::mt19937 rng(static_cast<uint32_t>(std::random_device{}()));
	std::uniform_real_distribution<float> ux(-1.0f, 1.0f);
	std::uniform_real_distribution<float> uy(0.0f, 1.0f);
	std::uniform_real_distribution<float> uz(-1.0f, 1.0f);
	std::uniform_real_distribution<float> vv(-0.05f, 0.05f);

	std::vector<glm::vec4> posData;
	posData.resize(maxParticles);
	std::vector<glm::vec4> velData;
	velData.resize(maxParticles);

	uint32_t idx = 0;

	for (int z = 0; z < GRID_RES.z && idx < maxParticles; z++) {
		for (int y = 0; y < GRID_RES.y && idx < maxParticles; y++) {
			for (int x = 0; x < GRID_RES.x && idx < maxParticles; x++) {
				glm::vec3 p = GRID_MIN + (glm::vec3((float)x, (float)y, (float)z) + 0.5f) * GRID_CELL_SIZE;
				p = glm::clamp(p, GRID_MIN + glm::vec3(PARTICLE_SIM_RADIUS), GRID_MAX - glm::vec3(PARTICLE_SIM_RADIUS));

				posData[idx] = glm::vec4(p, 1.0f);
				velData[idx] = glm::vec4(0.0f);

				particles[idx].positionX = p.x;
				particles[idx].positionY = p.y;
				particles[idx].positionZ = p.z;
				particles[idx].velocityX = 0.0f;
				particles[idx].velocityY = 0.0f;
				particles[idx].velocityZ = 0.0f;
				idx++;
			}
		}
	}

	for (; idx < maxParticles; idx++) {
		uint32_t wrap = idx % (uint32_t)(GRID_RES.x * GRID_RES.y * GRID_RES.z);
		posData[idx] = posData[wrap];
		velData[idx] = glm::vec4(0.0f);
		particles[idx].positionX = posData[idx].x;
		particles[idx].positionY = posData[idx].y;
		particles[idx].positionZ = posData[idx].z;
		particles[idx].velocityX = 0.0f;
		particles[idx].velocityY = 0.0f;
		particles[idx].velocityZ = 0.0f;
	}

	/*for (uint32_t i = 0; i < maxParticles; i++) {
		float pX = ux(rng) * 0.5f;
		float pY = uy(rng) * 0.8f + 0.2f;
		float pZ = uz(rng) * 0.5f;

		float vX = vv(rng);
		float vY = vv(rng);
		float vZ = vv(rng);

		posData[i] = glm::vec4(pX, pY, pZ, 1.0f);
		velData[i] = glm::vec4(vX, vY, vZ, 0.0f);

		particles[i].positionX = pX;
		particles[i].positionY = pY;
		particles[i].positionZ = pZ;
		particles[i].velocityX = vX;
		particles[i].velocityY = vY;
		particles[i].velocityZ = vZ;
	}*/

	if (ssboPos == 0) glGenBuffers(1, &ssboPos);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboPos);
	glBufferData(GL_SHADER_STORAGE_BUFFER, posData.size() * sizeof(glm::vec4), posData.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboPos);

	if (ssboVel == 0) glGenBuffers(1, &ssboVel);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboVel);
	glBufferData(GL_SHADER_STORAGE_BUFFER, velData.size() * sizeof(glm::vec4), velData.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboVel);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	if (vao == 0) glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindVertexArray(0);

	std::cout << "Particle System: Initialized with " << maxParticles << " particles" << std::endl;

}

void ParticleSystem::Update(float deltaTime, float wallDamping) {
	//build uniform grid
	BuildUniformGrid();

	if (!computeProgram) return;
	computeProgram->use();
	computeProgram->setFloat("deltaTime", deltaTime);
	computeProgram->setUint("particleCount", maxParticles);
	computeProgram->setVec3("boxMin", this->GRID_MIN);
	computeProgram->setVec3("boxMax", this->GRID_MAX);
	computeProgram->setFloat("particleRadius", this->PARTICLE_SIM_RADIUS);
	computeProgram->setFloat("wallDamping", wallDamping);
	uint32_t groups = (maxParticles + workGroupSize - 1) / workGroupSize;
	computeProgram->dispatch(groups, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

}


void ParticleSystem::Render(const glm::mat4& mvp, const glm::vec3 &particleColor, float pointSize, const glm::vec2 &viewportSize, const glm::mat4& projection, const glm::mat4& view, const glm::vec3 &lightWorldPos) {
	glBindVertexArray(vao);

	renderShader->use();
	renderShader->setMat4("mvp", mvp);
	renderShader->setMat4("projection", projection);
	renderShader->setMat4("view", view);
	renderShader->setVec2("viewportSize", viewportSize);
	renderShader->setVec3("lightPosView", glm::vec3(view * glm::vec4(lightWorldPos, 1.0)));
	renderShader->setFloat("pointSize", pointSize);
	renderShader->setBool("debugCellColor", true);
	renderShader->setVec3("gridRes", glm::vec3(GRID_RES));
	renderShader->setVec3("particleColor", particleColor);
	//need to decide for simple/lean blinn phong shading uniforms

	glDrawArrays(GL_POINTS, 0, maxParticles);
	glBindVertexArray(0);

}