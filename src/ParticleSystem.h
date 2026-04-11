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
	enum class SpawnMode {
		Random = 0,
		SingleDam = 1,
		DoubleDam = 2,
		SingleSheet = 3
	};

	ParticleSystem(size_t maxParticles, float simRadius, const glm::vec3& gridMin, const glm::vec3& gridMax, const char* computeShaderPath, const char* vertexShaderPath, const char* fragmentShaderPath);
	~ParticleSystem();
	void Update(float deltaTime, float wallDamping, bool enableSPH);
	void Render(const glm::mat4& mvp, const glm::vec3& particleColor, float pointSize, const glm::vec2& viewportSize, const glm::mat4& projection, const glm::mat4& view, const glm::vec3& lightWorldPos);
	void RenderBoundary(const glm::mat4& mvp, const glm::vec3& color, float pointSize);
	
	//fluid surface rendering
	void RenderFluidDepth(const glm::mat4& mvp, float pointSize, const glm::vec2& viewportSize, const glm::mat4& projection, const glm::mat4& view);
	void RenderFluidThickness(const glm::mat4& mvp, float pointSize, const glm::vec2& viewportSize, const glm::mat4& projection, const glm::mat4& view);

	void ResetParticles();
	void SetSpawnMode(SpawnMode mode, bool reinitialize = true);
	SpawnMode GetSpawnMode() const { return spawnMode; }

	void SetBounds(const glm::vec3& gridMin, const glm::vec3& gridMax, bool rebuild = true, bool reinitializeParticles = true);

	void SetRestDensity(float restDensity) { this->restDensity = restDensity; }
	void SetViscosityCoeff(float viscosityCoeff) { this->viscosityCoeff = viscosityCoeff; }
	void SetStiffness(float stiffness) { this->stiffness = stiffness; }
	void SetGravity(const glm::vec3& gravity) { this->GRAVITY = gravity; }
	void SetVorticityEpsilon(float epsilon) { this->vorticityEpsilon = epsilon; }
	void SetPBFSolverIterations(uint32_t iterations) { this->pbfSolverIterations = iterations; }
	void SetPBFRelaxation(float relaxation) { this->pbfRelaxation = relaxation; }
	void SetPBFCorrK(float k) { this->pbfScorrK = k; }
	void SetPBFCorrN(float n) { this->pbfScorrN = n; }
	void SetPBFCorrDQ(float dq) { this->pbfScorrDQ = dq; }
	void SetPBFEpsilon(float epsilon) { this->pbfEpsilon = epsilon; }

	float GetRestDensity() const { return restDensity; }
	float GetViscosityCoeff() const { return viscosityCoeff; }
	float GetStiffness() const { return stiffness; }
	glm::vec3 GetGravity() const { return GRAVITY; }
	float GetVorticityEpsilon() const { return vorticityEpsilon; }
	uint32_t GetPBFSolverIterations() const { return pbfSolverIterations;  }
	float GetPBFRelaxation() const { return pbfRelaxation; }
	float GetPBFCorrK() const { return pbfScorrK; }
	float GetPBFCorrN() const{ return pbfScorrN; }
	float GetPBFCorrDQ() const{ return pbfScorrDQ; }
	float GetPBFEpsilon() const{ return pbfEpsilon; }

	glm::vec3 GetGridMin() const { return GRID_MIN; }
	glm::vec3 GetGridMax() const { return GRID_MAX; }



private:
	size_t maxParticles;
	std::vector<Particle> particles;
	float PARTICLE_SIM_RADIUS = 0.02f;
	float GRID_CELL_SIZE = PARTICLE_SIM_RADIUS * 2.0f;
	float PI = 3.14159265359;
	glm::vec3 GRID_MIN;
	glm::vec3 GRID_MAX;
	glm::vec3 GRID_SIZE;
	glm::ivec3 GRID_RES;
	uint32_t GRID_VOXEL_COUNT;

	float boundarySpacing = 0.0f;
	uint32_t boundaryCount = 0;

	glm::vec3 GRAVITY;
	float restDensity;
	float viscosityCoeff;
	float stiffness;

	uint32_t pbfSolverIterations = 4;
	float pbfScorrK = 0.00002f;
	float pbfScorrN = 4.0f;
	float pbfEpsilon = 1e-5f;
	float pbfScorrDQ = 0.2f;
	float pbfRelaxation = 0.2f;
	float vorticityEpsilon = 0.0f;
	SpawnMode spawnMode = SpawnMode::Random;

	uint32_t ssboPos = 0;
	uint32_t ssboVel = 0;
	uint32_t vao = 0;

	uint32_t ssboCellCount = 0;
	uint32_t ssboCellOffset = 0;
	uint32_t ssboCellWrite = 0;
	uint32_t ssboParticleCell = 0;
	uint32_t ssboSortedIndex = 0;
	uint32_t ssboSortedPos = 0;
	uint32_t ssboSortedVel = 0;
	uint32_t ssboGridBlockSums = 0;

	uint32_t ssboBoundaryGhostParticles = 0;
	uint32_t ssboBoundaryCellCount = 0;
	uint32_t ssboBoundaryCellStart = 0;
	uint32_t ssboBoundarySortedIndex = 0;

	//sph ssbos
	uint32_t ssboDensity = 0;
	uint32_t ssboViscosityAccel = 0;
	uint32_t ssboPressure = 0;
	uint32_t ssboPressureAccel = 0;

	//pbf ssbos
	uint32_t ssboPredPos = 0;
	uint32_t ssboLambda = 0;
	uint32_t ssboDeltaPos = 0;
	uint32_t ssboVorticity = 0;

	const uint32_t workGroupSize = 256;

	//render shaders
	Shader* renderShader = nullptr;	
	Shader* boundaryRenderShader = nullptr;
	Shader* fluidDepthShader = nullptr;
	Shader* fluidThicknessShader = nullptr;

	CaveCompute* gridClearProgram = nullptr;
	CaveCompute* gridParticleCountProgram = nullptr;
	CaveCompute* gridParticleReorderProgram = nullptr;
	CaveCompute* gridPrefixScanProgram = nullptr;
	CaveCompute* gridPrefixBlockSumProgram = nullptr;
	CaveCompute* gridPrefixAddProgram = nullptr;
	CaveCompute* gridCopyOffsetWriteProgram = nullptr;

	//integrator
	//CaveCompute* computeProgram = nullptr;
	//density compute
	//CaveCompute* densityComputeProgram = nullptr;
	CaveCompute* sphDensityPressureComputeProgram = nullptr;
	//viscosity compute
	//CaveCompute* viscosityComputeProgram = nullptr;
	//pressure compute
	//CaveCompute* pressureComputeProgram = nullptr;
	CaveCompute* sphForceIntegrateComputeProgram = nullptr;
	//CaveCompute* sphVorticityComputeProgram = nullptr;
	//CaveCompute* sphVorticityApplyComputeProgram = nullptr;


	//pbf compute programs
	CaveCompute* pbfPredictPosComputeProgram = nullptr;
	CaveCompute* pbfLambdaComputeProgram = nullptr;
	CaveCompute* pbfDeltaPosComputeProgram = nullptr;
	CaveCompute* pbfApplyCorrComputeProgram = nullptr;
	CaveCompute* pbfIntegrateComputeProgram = nullptr;
	CaveCompute* pbfXSPHComputeProgram = nullptr;
	CaveCompute* pbfXSPHApplyComputeProgram = nullptr;
	CaveCompute* pbfVorticityComputeProgram = nullptr;
	CaveCompute* pbfVorticityApplyComputeProgram = nullptr;



	//cached initial state for reset
	std::vector<glm::vec4> initialPositions;
	std::vector<glm::vec4> initialVelocities;

	//boundary
	std::vector<glm::vec4> boundaryGhostParticles;

	void InitializeGrid();
	void InitializeBoundaryGhostParticles();
	void BuildUniformGrid(bool usePredictedPositions);
	void InitializeParticles();
	//float ComputeCFLTimeStep(float dtMin, float dtMax);

};