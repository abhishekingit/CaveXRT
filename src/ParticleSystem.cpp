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

	densityComputeProgram = new CaveCompute("../../../src/Shaders/Particles/densitysimcshader.comp");
	viscosityComputeProgram = new CaveCompute("../../../src/Shaders/Particles/viscositysimcshader.comp");
	pressureComputeProgram = new CaveCompute("../../../src/Shaders/Particles/pressuresimcshader.comp");

	this->GRAVITY = glm::vec3(0.0f, -9.81f, 0.0f);
	this->restDensity = 1000.0f;
	this->viscosityCoeff = 0.03f;
	this->stiffness = 60.0f;

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

	if(ssboDensity) {
		glDeleteBuffers(1, &ssboDensity);
		ssboDensity = 0;
	}

	if(ssboViscosityAccel) {
		glDeleteBuffers(1, &ssboViscosityAccel);
		ssboViscosityAccel = 0;
	}

	if (ssboPressure) {
		glDeleteBuffers(1, &ssboPressure);
		ssboPressure = 0;
	}

	if (ssboPressureAccel) {
		glDeleteBuffers(1, &ssboPressureAccel);
		ssboPressureAccel = 0;
	}

	if (vao) {
		glDeleteVertexArrays(1, &vao);
		vao = 0;
	}

	if (computeProgram) {
		delete computeProgram;
		computeProgram = nullptr;
	}

	if (densityComputeProgram) {
		delete densityComputeProgram;
		densityComputeProgram = nullptr;
	}

	if (viscosityComputeProgram) {
		delete viscosityComputeProgram;
		viscosityComputeProgram = nullptr;
	}

	if (pressureComputeProgram) {
		delete pressureComputeProgram;
		pressureComputeProgram = nullptr;
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

	/*for (int z = 0; z < GRID_RES.z && idx < maxParticles; z++) {
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
	}*/

	for (uint32_t i = 0; i < maxParticles; i++) {
		float pX = ux(rng) * 0.5f;
		float pY = uy(rng) * 0.8f + 0.9f;
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
	}

	initialPositions = posData;
	initialVelocities = velData;

	

	if (ssboPos == 0) glGenBuffers(1, &ssboPos);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboPos);
	glBufferData(GL_SHADER_STORAGE_BUFFER, posData.size() * sizeof(glm::vec4), posData.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboPos);

	if (ssboVel == 0) glGenBuffers(1, &ssboVel);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboVel);
	glBufferData(GL_SHADER_STORAGE_BUFFER, velData.size() * sizeof(glm::vec4), velData.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboVel);

	if (ssboDensity == 0) glGenBuffers(1, &ssboDensity);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboDensity);
	std::vector<float> densityData(maxParticles, this->restDensity);
	glBufferData(GL_SHADER_STORAGE_BUFFER, densityData.size() * sizeof(float), densityData.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, ssboDensity);

	if (ssboViscosityAccel == 0) glGenBuffers(1, &ssboViscosityAccel);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboViscosityAccel);
	std::vector<glm::vec4> viscosityData(maxParticles, glm::vec4(0.0f));
	glBufferData(GL_SHADER_STORAGE_BUFFER, viscosityData.size() * sizeof(glm::vec4), viscosityData.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, ssboViscosityAccel);

	if (ssboPressure == 0) glGenBuffers(1, &ssboPressure);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboPressure);
	std::vector<float> pressureData(maxParticles, 0.0f);
	glBufferData(GL_SHADER_STORAGE_BUFFER, pressureData.size() * sizeof(float), pressureData.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, ssboPressure);

	if (ssboPressureAccel == 0) glGenBuffers(1, &ssboPressureAccel);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboPressureAccel);
	std::vector<glm::vec4> pressureAccelData(maxParticles, glm::vec4(0.0f));
	glBufferData(GL_SHADER_STORAGE_BUFFER, pressureAccelData.size() * sizeof(glm::vec4), pressureAccelData.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, ssboPressureAccel);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	if (vao == 0) glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindVertexArray(0);

	std::cout << "Particle System: Initialized with " << maxParticles << " particles" << std::endl;

}

void ParticleSystem::Update(float deltaTime, float wallDamping) {
	//build uniform grid
	BuildUniformGrid();
	uint32_t groups = (maxParticles + workGroupSize - 1) / workGroupSize;
	const float dt = std::min(deltaTime, 1.0f / 120.0f);
	
	//smoothing kernel radius
	const float h = GRID_CELL_SIZE;

	//particle spacing(particle radius * 4.0f) for more neighbours
	const float r = GRID_CELL_SIZE;
	const float dx = PARTICLE_SIM_RADIUS * 2.0f;
	const float mass = this->restDensity * dx * dx * dx;
	
	//density pass SPH
	if (!densityComputeProgram) return;
	densityComputeProgram->use();
	densityComputeProgram->setUint("particleCount", maxParticles);
	densityComputeProgram->setUint("gridResX", static_cast<uint32_t>(GRID_RES.x));
	densityComputeProgram->setUint("gridResY", static_cast<uint32_t>(GRID_RES.y));
	densityComputeProgram->setUint("gridResZ", static_cast<uint32_t>(GRID_RES.z));
	densityComputeProgram->setFloat("h", h);
	densityComputeProgram->setFloat("mass", mass);
	densityComputeProgram->setFloat("PI", this->PI);
	densityComputeProgram->setFloat("restDensity", this->restDensity);
	densityComputeProgram->dispatch(groups, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	//viscosity pass SPH
	if (!viscosityComputeProgram) return;
	viscosityComputeProgram->use();
	viscosityComputeProgram->setUint("particleCount", maxParticles);
	viscosityComputeProgram->setUint("gridResX", static_cast<uint32_t>(GRID_RES.x));
	viscosityComputeProgram->setUint("gridResY", static_cast<uint32_t>(GRID_RES.y));
	viscosityComputeProgram->setUint("gridResZ", static_cast<uint32_t>(GRID_RES.z));
	viscosityComputeProgram->setFloat("h", h);
	viscosityComputeProgram->setFloat("mass", mass);
	viscosityComputeProgram->setFloat("PI", this->PI);
	viscosityComputeProgram->setFloat("viscosityCoeff", this->viscosityCoeff);
	viscosityComputeProgram->dispatch(groups, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	if (!pressureComputeProgram) return;
	pressureComputeProgram->use();
	pressureComputeProgram->setUint("particleCount", maxParticles);
	pressureComputeProgram->setUint("gridResX", static_cast<uint32_t>(GRID_RES.x));
	pressureComputeProgram->setUint("gridResY", static_cast<uint32_t>(GRID_RES.y));
	pressureComputeProgram->setUint("gridResZ", static_cast<uint32_t>(GRID_RES.z));
	pressureComputeProgram->setFloat("h", h);
	pressureComputeProgram->setFloat("mass", mass);
	pressureComputeProgram->setFloat("PI", this->PI);
	pressureComputeProgram->setFloat("restDensity", this->restDensity);
	pressureComputeProgram->setFloat("stiffness", this->stiffness);
	pressureComputeProgram->dispatch(groups, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	if (!computeProgram) return;
	computeProgram->use();
	computeProgram->setFloat("deltaTime", dt);
	computeProgram->setUint("particleCount", maxParticles);
	computeProgram->setVec3("boxMin", this->GRID_MIN);
	computeProgram->setVec3("boxMax", this->GRID_MAX);
	computeProgram->setFloat("particleRadius", this->PARTICLE_SIM_RADIUS);
	computeProgram->setVec3("gravity", this->GRAVITY);
	computeProgram->setFloat("wallDamping", wallDamping);

	//part of density debug test
	computeProgram->setFloat("restDensity", this->restDensity);
	computeProgram->setFloat("buoyancyCoeff", 0.05f);
	computeProgram->setFloat("densityDragCoeff", 0.2f);

	
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
	renderShader->setBool("debugCellColor", false);
	renderShader->setBool("debugDensityColor", true);
	renderShader->setVec3("gridRes", glm::vec3(GRID_RES));
	renderShader->setVec3("particleColor", particleColor);

	renderShader->setVec2("densityMinMax", glm::vec2(restDensity * 0.2, restDensity * 1.5f));
	//need to decide for simple/lean blinn phong shading uniforms

	glDrawArrays(GL_POINTS, 0, maxParticles);
	glBindVertexArray(0);

}

void ParticleSystem::ResetParticles() {
	if (initialPositions.empty() || initialVelocities.empty()) return;

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboPos);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, initialPositions.size() * sizeof(glm::vec4), initialPositions.data());

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboVel);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, initialVelocities.size() * sizeof(glm::vec4), initialVelocities.data());

	// optional: reset density buffer to rest density
	if (ssboDensity) {
		std::vector<float> densityData(maxParticles, restDensity);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboDensity);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, densityData.size() * sizeof(float), densityData.data());
	}

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

}