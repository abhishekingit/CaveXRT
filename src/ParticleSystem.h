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
	ParticleSystem(size_t maxParticles, const char* computeShaderPath, const char* vertexShaderPath, const char* fragmentShaderPath);
	~ParticleSystem();
	void Update(float deltaTime, const glm::vec3& bboxMin, const glm::vec3& bboxMax, float pointSize, float wallDamping);
	void Render(const glm::mat4& mvp, const glm::vec3& particleColor, float pointSize, const glm::vec2& viewportSize, const glm::mat4& projection, const glm::mat4& view, const glm::vec3& lightWorldPos);

private:
	size_t maxParticles;
	std::vector<Particle> particles;
	uint32_t ssboPos = 0;
	uint32_t ssboVel = 0;
	uint32_t vao = 0;

	const uint32_t workGroupSize = 256;


	Shader* renderShader = nullptr;
	CaveCompute* computeProgram = nullptr;


	void InitializeParticles();
};