#pragma once

#include <random>
#include <glm/glm.hpp>
#include "ParticleSystem.h"


ParticleSystem::ParticleSystem(size_t maxParticles, const char* computeShaderPath, const char* vertexShaderPath, const char* fragmentShaderPath) : maxParticles(maxParticles) {
	particles.resize(maxParticles);
	computeProgram = new CaveCompute(computeShaderPath);
	renderShader = new Shader(vertexShaderPath, fragmentShaderPath);

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

	for (uint32_t i = 0; i < maxParticles; i++) {
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
	}

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

void ParticleSystem::Update(float deltaTime, const glm::vec3 &bboxMin, const glm::vec3 &bboxMax, float pointSize, float wallDamping) {
	if (!computeProgram) return;
	computeProgram->use();
	computeProgram->setFloat("deltaTime", deltaTime);
	computeProgram->setUint("particleCount", maxParticles);
	computeProgram->setVec3("boxMin", bboxMin);
	computeProgram->setVec3("boxMax", bboxMax);
	computeProgram->setFloat("particleRadius", pointSize);
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
	renderShader->setVec3("particleColor", particleColor);
	//need to decide for simple/lean blinn phong shading uniforms

	glDrawArrays(GL_POINTS, 0, maxParticles);
	glBindVertexArray(0);

}