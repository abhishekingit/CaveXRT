#pragma once

#include <random>
#include <glm/glm.hpp>
#include "ParticleSystem.h"



ParticleSystem::ParticleSystem(size_t maxParticles, float simRadius, const glm::vec3& gridMin, const glm::vec3& gridMax, const char* computeShaderPath, const char* vertexShaderPath, const char* fragmentShaderPath) : maxParticles(maxParticles), PARTICLE_SIM_RADIUS(simRadius), GRID_MIN(gridMin), GRID_MAX(gridMax) {
	particles.resize(maxParticles);
	//computeProgram = new CaveCompute(computeShaderPath);
	renderShader = new Shader(vertexShaderPath, fragmentShaderPath);
	boundaryRenderShader = new Shader("../../../src/Shaders/Particles/boundaryvshader.vert", "../../../src/Shaders/Particles/boundaryfshader.frag");
	fluidDepthShader = new Shader("../../../src/Shaders/Particles/fluidRender/particlevshader.vert", "../../../src/Shaders/Particles/fluidRender/fluidDepthpass.frag");
	fluidThicknessShader = new Shader("../../../src/Shaders/Particles/fluidRender/particlevshader.vert", "../../../src/Shaders/Particles/fluidRender/fluidThicknesspass.frag");

	gridClearProgram = new CaveCompute("../../../src/Shaders/Particles/gridclear.comp");
	gridParticleCountProgram = new CaveCompute("../../../src/Shaders/Particles/gridparticlecount.comp");
	gridParticleReorderProgram = new CaveCompute("../../../src/Shaders/Particles/gridparticlesort.comp");
	gridPrefixScanProgram = new CaveCompute("../../../src/Shaders/Particles/gridprefixscan.comp");
	gridPrefixBlockSumProgram = new CaveCompute("../../../src/Shaders/Particles/gridprefixblocksum.comp");
	gridPrefixAddProgram = new CaveCompute("../../../src/Shaders/Particles/gridprefixadd.comp");
	gridCopyOffsetWriteProgram = new CaveCompute("../../../src/Shaders/Particles/gridoffsetcopy.comp");

	/*densityComputeProgram = new CaveCompute("../../../src/Shaders/Particles/densitysimcshader.comp");*/
	sphDensityPressureComputeProgram = new CaveCompute("../../../src/Shaders/Particles/sphdensitypressure.comp");
	/*viscosityComputeProgram = new CaveCompute("../../../src/Shaders/Particles/viscositysimcshader.comp");
	pressureComputeProgram = new CaveCompute("../../../src/Shaders/Particles/pressuresimcshader.comp");*/
	sphForceIntegrateComputeProgram = new CaveCompute("../../../src/Shaders/Particles/sphforceintegrate.comp");
	/*sphVorticityComputeProgram = new CaveCompute("../../../src/Shaders/Particles/sphvorticity.comp");
	sphVorticityApplyComputeProgram = new CaveCompute("../../../src/Shaders/Particles/sphvorticityapply.comp");*/

	//pbf compute programs
	pbfPredictPosComputeProgram = new CaveCompute("../../../src/Shaders/Particles/pbf/pbfpospredict.comp");
	pbfLambdaComputeProgram = new CaveCompute("../../../src/Shaders/Particles/pbf/pbflambda.comp");
	pbfDeltaPosComputeProgram = new CaveCompute("../../../src/Shaders/Particles/pbf/pbfdeltapos.comp");
	pbfApplyCorrComputeProgram = new CaveCompute("../../../src/Shaders/Particles/pbf/pbfapplycorr.comp");
	pbfIntegrateComputeProgram = new CaveCompute("../../../src/Shaders/Particles/pbf/pbfintegrate.comp");
	pbfXSPHComputeProgram = new CaveCompute("../../../src/Shaders/Particles/pbf/pbfxsph.comp");
	pbfXSPHApplyComputeProgram = new CaveCompute("../../../src/Shaders/Particles/pbf/pbfxsphapply.comp");
	pbfVorticityComputeProgram = new CaveCompute("../../../src/Shaders/Particles/pbf/pbfvorticity.comp");
	pbfVorticityApplyComputeProgram = new CaveCompute("../../../src/Shaders/Particles/pbf/pbfvorticityapply.comp");

	this->GRAVITY = glm::vec3(0.0f, -9.81f, 0.0f);
	this->restDensity = 1000.0f;
	this->viscosityCoeff = 0.01f;
	this->stiffness = 40.0f;
	this->boundarySpacing = this->PARTICLE_SIM_RADIUS * 2.0f;

	InitializeGrid();
	InitializeBoundaryGhostParticles();
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

	if (ssboPredPos) {
		glDeleteBuffers(1, &ssboPredPos);
		ssboPredPos = 0;
	}

	if (ssboLambda) {
		glDeleteBuffers(1, &ssboLambda);
		ssboLambda = 0;
	}

	if (ssboDeltaPos) {
		glDeleteBuffers(1, &ssboDeltaPos);
		ssboDeltaPos = 0;
	}

	if (ssboVorticity) {
		glDeleteBuffers(1, &ssboVorticity);
		ssboVorticity = 0;
	}

	if (ssboBoundaryGhostParticles) {
		glDeleteBuffers(1, &ssboBoundaryGhostParticles);
		ssboBoundaryGhostParticles = 0;
	}

	if (ssboBoundaryCellCount) {
		glDeleteBuffers(1, &ssboBoundaryCellCount);
		ssboBoundaryCellCount = 0;
	}

	if (ssboBoundaryCellStart) {
		glDeleteBuffers(1, &ssboBoundaryCellStart);
		ssboBoundaryCellStart = 0;
	}

	if (ssboBoundarySortedIndex) {
		glDeleteBuffers(1, &ssboBoundarySortedIndex);
		ssboBoundarySortedIndex = 0;
	}

	if (ssboSortedPos) {
		glDeleteBuffers(1, &ssboSortedPos);
		ssboSortedPos = 0;
	}

	if (ssboSortedVel) {
		glDeleteBuffers(1, &ssboSortedVel);
		ssboSortedVel = 0;
	}

	if (vao) {
		glDeleteVertexArrays(1, &vao);
		vao = 0;
	}

	/*if (computeProgram) {
		delete computeProgram;
		computeProgram = nullptr;
	}

	if (densityComputeProgram) {
		delete densityComputeProgram;
		densityComputeProgram = nullptr;
	}*/

	if (sphDensityPressureComputeProgram) {
		delete sphDensityPressureComputeProgram;
		sphDensityPressureComputeProgram = nullptr;
	}

	/*if (viscosityComputeProgram) {
		delete viscosityComputeProgram;
		viscosityComputeProgram = nullptr;
	}

	if (pressureComputeProgram) {
		delete pressureComputeProgram;
		pressureComputeProgram = nullptr;
	}*/

	if (sphForceIntegrateComputeProgram) {
		delete sphForceIntegrateComputeProgram;
		sphForceIntegrateComputeProgram = nullptr;
	}

	/*if (sphVorticityComputeProgram) {
		delete sphVorticityComputeProgram;
		sphVorticityComputeProgram = nullptr;
	}

	if (sphVorticityApplyComputeProgram) {
		delete sphVorticityApplyComputeProgram;
		sphVorticityApplyComputeProgram = nullptr;
	}*/

	if(pbfPredictPosComputeProgram) {
		delete pbfPredictPosComputeProgram;
		pbfPredictPosComputeProgram = nullptr;
	
	}

	if (pbfLambdaComputeProgram) {
		delete pbfLambdaComputeProgram;
		pbfLambdaComputeProgram = nullptr;
	}

	if (pbfDeltaPosComputeProgram) {
		delete pbfDeltaPosComputeProgram;
		pbfDeltaPosComputeProgram = nullptr;
	}

	if (pbfApplyCorrComputeProgram) {
		delete pbfApplyCorrComputeProgram;
		pbfApplyCorrComputeProgram = nullptr;
	}

	if (pbfIntegrateComputeProgram) {
		delete pbfIntegrateComputeProgram;
		pbfIntegrateComputeProgram = nullptr;
	}

	if (pbfXSPHComputeProgram) {
		delete pbfXSPHComputeProgram;
		pbfXSPHComputeProgram = nullptr;
	}

	if (pbfXSPHApplyComputeProgram) {
		delete pbfXSPHApplyComputeProgram;
		pbfXSPHApplyComputeProgram = nullptr;
	}

	if (pbfVorticityComputeProgram) {
		delete pbfVorticityComputeProgram;
		pbfVorticityComputeProgram = nullptr;
	}

	if (pbfVorticityApplyComputeProgram) {
		delete pbfVorticityApplyComputeProgram;
		pbfVorticityApplyComputeProgram = nullptr;
	}

	if (renderShader) {
		delete renderShader;
		renderShader = nullptr;
	}

	if (boundaryRenderShader) {
		delete boundaryRenderShader;
		boundaryRenderShader = nullptr;
	}

	if (fluidDepthShader) {
		delete fluidDepthShader;
		fluidDepthShader = nullptr;
	}

	if (fluidThicknessShader) {
		delete fluidThicknessShader;
		fluidThicknessShader = nullptr;
	}
}

void ParticleSystem::InitializeBoundaryGhostParticles() {
	boundaryGhostParticles.clear();

	const float spacing = boundarySpacing;
	const float inset = PARTICLE_SIM_RADIUS * 0.10f;
	const glm::vec3 minB = GRID_MIN;
	const glm::vec3 maxB = GRID_MAX;
	const float eps = 1e-6f;
	const int layers = 4;

	auto push = [&](float x, float y, float z, float weight) {
		boundaryGhostParticles.emplace_back(x, y, z, weight);
	};

	const float weight0 = restDensity * spacing * spacing * spacing * 0.8f;

	for (int l = 1; l <= layers; l++) {
		float offset = l * spacing;
		//xmin/max
		for (float y = minB.y; y <= maxB.y + eps; y += spacing) {
			for (float z = minB.z; z <= maxB.z + eps; z += spacing) {
				push(minB.x - offset, y, z, weight0);
				push(maxB.x + offset, y, z, weight0);
			}
		}

		//ymin/max
		for (float x = minB.x + spacing; x <= maxB.x - spacing + eps; x += spacing) {
			for (float z = minB.z; z <= maxB.z + eps; z += spacing) {
				push(x, minB.y - offset, z, weight0);
				push(x, maxB.y + offset, z, weight0);
			}
		}

		//zmin/zmax
		for (float x = minB.x + spacing; x <= maxB.x - spacing + 1e-6f; x += spacing) {
			for (float y = minB.y + spacing; y <= maxB.y - spacing + 1e-6f; y += spacing) {
				push(x, y, minB.z - offset, weight0);
				push(x, y, maxB.z + offset, weight0);
			}
		}
	}

	

	boundaryCount = static_cast<uint32_t>(boundaryGhostParticles.size());
	//boundary neighbor search
	auto clampCell = [&](const glm::vec3& p) -> glm::ivec3 {
		glm::vec3 rel = (p - GRID_MIN) / GRID_CELL_SIZE;
		glm::ivec3 c = glm::ivec3(glm::floor(rel));
		return glm::clamp(c, glm::ivec3(0), GRID_RES - glm::ivec3(1));
	};

	auto flatten = [&](const glm::ivec3& c) -> uint32_t {
		return static_cast<uint32_t>(c.x + GRID_RES.x * (c.y + GRID_RES.y * c.z));
	};

	std::vector<uint32_t> boundaryCellCount(GRID_VOXEL_COUNT, 0u);
	for (const glm::vec4& b : boundaryGhostParticles) {
		uint32_t cell = flatten(clampCell(glm::vec3(b)));
		boundaryCellCount[cell]++;
	}

	std::vector<uint32_t> boundaryCellStart(GRID_VOXEL_COUNT, 0u);
	uint32_t sum = 0u;
	for (uint32_t i = 0; i < GRID_VOXEL_COUNT; ++i) {
		boundaryCellStart[i] = sum;
		sum += boundaryCellCount[i];
	}

	std::vector<uint32_t> boundarySortedIndex(boundaryCount, 0u);
	std::vector<uint32_t> writeHead = boundaryCellStart;
	for (uint32_t i = 0u; i < boundaryCount; ++i) {
		uint32_t cell = flatten(clampCell(glm::vec3(boundaryGhostParticles[i])));
		boundarySortedIndex[writeHead[cell]++] = i;
	}
	//initial start

	if (ssboBoundaryGhostParticles == 0) glGenBuffers(1, &ssboBoundaryGhostParticles);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboBoundaryGhostParticles);
	glBufferData(GL_SHADER_STORAGE_BUFFER, boundaryGhostParticles.size() * sizeof(glm::vec4), boundaryGhostParticles.data(), GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 15, ssboBoundaryGhostParticles);

	if (ssboBoundaryCellCount == 0) glGenBuffers(1, &ssboBoundaryCellCount);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboBoundaryCellCount);
	glBufferData(GL_SHADER_STORAGE_BUFFER, boundaryCellCount.size() * sizeof(uint32_t), boundaryCellCount.data(), GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, ssboBoundaryCellCount);

	if (ssboBoundaryCellStart == 0) glGenBuffers(1, &ssboBoundaryCellStart);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboBoundaryCellStart);
	glBufferData(GL_SHADER_STORAGE_BUFFER, boundaryCellStart.size() * sizeof(uint32_t), boundaryCellStart.data(), GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, ssboBoundaryCellStart);

	if (ssboBoundarySortedIndex == 0) glGenBuffers(1, &ssboBoundarySortedIndex);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboBoundarySortedIndex);
	glBufferData(GL_SHADER_STORAGE_BUFFER, boundarySortedIndex.size() * sizeof(uint32_t), boundarySortedIndex.data(), GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 13, ssboBoundarySortedIndex);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

}

void ParticleSystem::InitializeGrid() {
	GRID_CELL_SIZE = PARTICLE_SIM_RADIUS * 4.0f;
	GRID_SIZE = GRID_MAX - GRID_MIN;
	GRID_RES = glm::ivec3(glm::max(glm::vec3(1.0f), glm::ceil(GRID_SIZE / GRID_CELL_SIZE)));
	GRID_VOXEL_COUNT = static_cast<uint32_t>(GRID_RES.x * GRID_RES.y * GRID_RES.z);

	auto divUp = [](uint32_t a, uint32_t b) { return (a + b - 1u) / b; };
	const uint32_t scanElemsPerGroup = 512u;
	const uint32_t scanGroupCount = divUp(GRID_VOXEL_COUNT, scanElemsPerGroup);

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

	if (ssboGridBlockSums == 0) glGenBuffers(1, &ssboGridBlockSums);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboGridBlockSums);
	glBufferData(GL_SHADER_STORAGE_BUFFER, scanGroupCount * sizeof(uint32_t), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 15, ssboGridBlockSums);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

}

void ParticleSystem::BuildUniformGrid(bool usePredictedPositions) {
	auto divUp = [](uint32_t a, uint32_t b) { return (a + b - 1u) / b; };
	const uint32_t scanElemsPerGroup = 512u;
	const uint32_t scanGroupCount = divUp(GRID_VOXEL_COUNT, scanElemsPerGroup);

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

	//GPU Prefix scan
	/*glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 15, ssboGridBlockSums);

	gridPrefixScanProgram->use();
	gridPrefixScanProgram->setUint("elementCount", GRID_VOXEL_COUNT);
	gridPrefixScanProgram->dispatch(scanGroupCount, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	gridPrefixBlockSumProgram->use();
	gridPrefixBlockSumProgram->setUint("blockCount", scanGroupCount);
	gridPrefixBlockSumProgram->dispatch(1, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	gridPrefixAddProgram->use();
	gridPrefixAddProgram->setUint("elementCount", GRID_VOXEL_COUNT);
	gridPrefixAddProgram->dispatch(scanGroupCount, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	gridCopyOffsetWriteProgram->use();
	gridCopyOffsetWriteProgram->setUint("elementCount", GRID_VOXEL_COUNT);
	gridCopyOffsetWriteProgram->dispatch(divUp(GRID_VOXEL_COUNT, 256u), 1, 1);
	glMemoryBarrier(
		GL_SHADER_STORAGE_BARRIER_BIT |
		GL_BUFFER_UPDATE_BARRIER_BIT |
		GL_ATOMIC_COUNTER_BARRIER_BIT
	);*/

	/*glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCellCount);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, counts.size() * sizeof(uint32_t), counts.data());

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCellOffset);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, offsets.size() * sizeof(uint32_t), offsets.data());

	bool ok = true;

	if (GRID_VOXEL_COUNT > 0 && offsets[0] != 0u) {
		std::cout << "[ScanCheck] offsets[0] != 0, got " << offsets[0] << "\n";
		ok = false;
	}

	for (uint32_t i = 0; i + 1 < GRID_VOXEL_COUNT; ++i) {
		uint32_t diff = offsets[i + 1] - offsets[i];
		if (diff != counts[i]) {
			std::cout << "[ScanCheck] mismatch at i=" << i
				<< " diff=" << diff
				<< " count=" << counts[i]
				<< " off[i]=" << offsets[i]
				<< " off[i+1]=" << offsets[i + 1] << "\n";
			ok = false;
			break;
		}
	}

	uint32_t total = 0u;
	for (uint32_t c : counts) total += c;

	if (GRID_VOXEL_COUNT > 0) {
		uint32_t endVal = offsets[GRID_VOXEL_COUNT - 1] + counts[GRID_VOXEL_COUNT - 1];
		if (endVal != total) {
			std::cout << "[ScanCheck] end mismatch endVal=" << endVal << " total=" << total << "\n";
			ok = false;
		}
	}

	if (!ok) {
		std::cout << "[ScanCheck] FAILED\n";
	}*/

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

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, usePredictedPositions ? ssboPredPos : ssboPos);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboVel);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, ssboSortedIndex);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, ssboSortedPos);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, ssboSortedVel);

	gridParticleReorderProgram->use();
	gridParticleReorderProgram->setUint("particleCount", static_cast<uint32_t>(maxParticles));
	gridParticleReorderProgram->dispatch(particleGroups, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

}

void ParticleSystem::SetSpawnMode(SpawnMode mode, bool reinitialize) {
	spawnMode = mode;
	if (reinitialize) {
		InitializeParticles();
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

	uint32_t idx = 0;

	auto writeParticle = [&](uint32_t i, const glm::vec3& p, const glm::vec3& v) {
		posData[i] = glm::vec4(p, 1.0f);
		velData[i] = glm::vec4(v, 0.0f);
		particles[i].positionX = p.x;
		particles[i].positionY = p.y;
		particles[i].positionZ = p.z;
		particles[i].velocityX = v.x;
		particles[i].velocityY = v.y;
		particles[i].velocityZ = v.z;
	};

	auto fillRandom = [&]() {
		for (; idx < maxParticles; idx++) {
			float pX = ux(rng) * 0.5f;
			float pY = uy(rng) * 0.8f + 0.9f;
			float pZ = uz(rng) * 0.5f;

			float vX = vv(rng);
			float vY = vv(rng);
			float vZ = vv(rng);

			writeParticle(idx, glm::vec3(pX, pY, pZ), glm::vec3(vX, vY, vZ));
		}
	};

	auto emitBlock = [&](const glm::vec3& bMin, const glm::vec3& bMax, const glm::vec3& initVel, float spacing) {
		for (float z = bMin.z; z <= bMax.z && idx < maxParticles; z += spacing) {
			for (float y = bMin.y; y <= bMax.y && idx < maxParticles; y += spacing) {
				for (float x = bMin.x; x <= bMax.x && idx < maxParticles; x += spacing) {
					glm::vec3 p = glm::clamp(glm::vec3(x, y, z), GRID_MIN + glm::vec3(PARTICLE_SIM_RADIUS), GRID_MAX - glm::vec3(PARTICLE_SIM_RADIUS));
					writeParticle(idx, p, initVel);
					idx++;
				}
			}
		}
	};

	if (spawnMode == SpawnMode::Random) {
		fillRandom();
	}
	else {
		const float spacing = PARTICLE_SIM_RADIUS * 2.05f;
		const float singleDamSpacing = PARTICLE_SIM_RADIUS * 2.15f;
		const glm::vec3 margin = glm::vec3(2.0f * spacing);
		const glm::vec3 minB = GRID_MIN + margin;
		const glm::vec3 maxB = GRID_MAX - margin;
		const glm::vec3 size = glm::max(maxB - minB, glm::vec3(spacing));

		const float damHeight = size.y * 0.75f;
		const float damDepth = size.z * 0.45f;

		auto emitFallingStack = [&](float xMin, float xMax, float zMin, float zMax, uint32_t targetCount, float s) {
			if(targetCount == 0u) return;
			float xSpan = glm::max(0.0f, xMax - xMin);
			float zSpan = glm::max(0.0f, zMax - zMin);
			uint32_t xCount = static_cast<uint32_t>(xSpan / s) + 1u;
			uint32_t zCount = static_cast<uint32_t>(zSpan / s) + 1u;
			if(xCount == 0u) xCount = 1u;
			if(zCount == 0u) zCount = 1u;
			uint32_t perLayer = xCount * zCount;
			if(perLayer == 0u) perLayer = 1u;
			uint32_t layerCount = (targetCount + perLayer - 1u) / perLayer;

			float topY = maxB.y - PARTICLE_SIM_RADIUS;
			float bottomY = topY - (static_cast<float>(layerCount - 1u) * s);
			float minBottomY = minB.y + PARTICLE_SIM_RADIUS;
			if(bottomY < minBottomY) bottomY = minBottomY;

			uint32_t emitted = 0u;
			for(float y = bottomY; y <= topY + 1e-6f && idx < maxParticles && emitted < targetCount; y += s) {
				for(float z = zMin; z <= zMax + 1e-6f && idx < maxParticles && emitted < targetCount; z += s) {
					for(float x = xMin; x <= xMax + 1e-6f && idx < maxParticles && emitted < targetCount; x += s) {
						glm::vec3 p = glm::clamp(glm::vec3(x, y, z), GRID_MIN + glm::vec3(PARTICLE_SIM_RADIUS), GRID_MAX - glm::vec3(PARTICLE_SIM_RADIUS));
						writeParticle(idx, p, glm::vec3(0.0f));
						idx++;
						emitted++;
					}
				}
			}
		};

		if (spawnMode == SpawnMode::SingleDam) {
			const float damWidth = size.x * 0.36f;
			const float zMin = minB.z + (size.z - damDepth) * 0.5f;
			const float zMax = minB.z + (size.z + damDepth) * 0.5f;
			emitFallingStack(minB.x, minB.x + damWidth, zMin, zMax, static_cast<uint32_t>(maxParticles), singleDamSpacing);
		}
		else if (spawnMode == SpawnMode::DoubleDam) {
			const float damWidth = size.x * 0.22f;
			const float zMin = minB.z + (size.z - damDepth) * 0.5f;
			const float zMax = minB.z + (size.z + damDepth) * 0.5f;
			uint32_t leftTarget = static_cast<uint32_t>(maxParticles / 2u);
			uint32_t rightTarget = static_cast<uint32_t>(maxParticles) - leftTarget;
			emitFallingStack(minB.x, minB.x + damWidth, zMin, zMax, leftTarget, spacing);
			emitFallingStack(maxB.x - damWidth, maxB.x, zMin, zMax, rightTarget, spacing);
		}
		else {
			const float sheetSpacing = PARTICLE_SIM_RADIUS * 2.0f;
			const float xMargin = size.x * 0.08f;
			const float zMargin = size.z * 0.08f;

			const float xMin = minB.x + xMargin;
			const float xMax = maxB.x - xMargin;
			const float zMin = minB.z + zMargin;
			const float zMax = maxB.z - zMargin;

			const float xSpan = glm::max(0.0f, xMax - xMin);
			const float zSpan = glm::max(0.0f, zMax - zMin);

			uint32_t xCount = static_cast<uint32_t>(xSpan / sheetSpacing) + 1u;
			uint32_t zCount = static_cast<uint32_t>(zSpan / sheetSpacing) + 1u;
			if (xCount == 0u) xCount = 1u;
			if (zCount == 0u) zCount = 1u;

			const uint32_t perLayer = xCount * zCount;
			const uint32_t layerCount = (static_cast<uint32_t>(maxParticles) + perLayer - 1u) / perLayer;

			float sheetBottomY = minB.y + size.y * 0.55f;
			float sheetTopY = sheetBottomY + (static_cast<float>(layerCount - 1u) * sheetSpacing);

			const float topLimit = maxB.y - PARTICLE_SIM_RADIUS;
			if (sheetTopY > topLimit) {
				sheetBottomY = topLimit - (static_cast<float>(layerCount - 1u) * sheetSpacing);
				sheetBottomY = glm::max(sheetBottomY, minB.y + PARTICLE_SIM_RADIUS);
				sheetTopY = sheetBottomY + (static_cast<float>(layerCount - 1u) * sheetSpacing);
			}

			glm::vec3 sMin(xMin, sheetBottomY, zMin);
			glm::vec3 sMax(xMax, sheetTopY, zMax);

			emitBlock(sMin, sMax, glm::vec3(0.0f, 0.0f, 0.0f), sheetSpacing);

		}

		if (idx == 0) {
			fillRandom();
		}
		else if (idx < maxParticles) {
			glm::vec3 fallbackMin(minB.x + spacing, minB.y + PARTICLE_SIM_RADIUS, minB.z + spacing);
			glm::vec3 fallbackMax(maxB.x - spacing, maxB.y - PARTICLE_SIM_RADIUS, maxB.z - spacing);
			emitBlock(fallbackMin, fallbackMax, glm::vec3(0.0f), spacing);
			if (idx < maxParticles) {
				fillRandom();
			}
		}
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

	if (ssboPredPos == 0) glGenBuffers(1, &ssboPredPos);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboPredPos);
	glBufferData(GL_SHADER_STORAGE_BUFFER, posData.size() * sizeof(glm::vec4), posData.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, ssboPredPos);

	if (ssboLambda == 0) glGenBuffers(1, &ssboLambda);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboLambda);
	std::vector<float> lambdaData(maxParticles, 0.0f);
	glBufferData(GL_SHADER_STORAGE_BUFFER, lambdaData.size() * sizeof(float), lambdaData.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, ssboLambda);

	if (ssboDeltaPos == 0) glGenBuffers(1, &ssboDeltaPos);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboDeltaPos);
	std::vector<glm::vec4> deltaPosData(maxParticles, glm::vec4(0.0f));
	glBufferData(GL_SHADER_STORAGE_BUFFER, deltaPosData.size() * sizeof(glm::vec4), deltaPosData.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 13, ssboDeltaPos);

	if (ssboVorticity == 0) glGenBuffers(1, &ssboVorticity);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboVorticity);
	std::vector<glm::vec4> vorticityData(maxParticles, glm::vec4(0.0f));
	glBufferData(GL_SHADER_STORAGE_BUFFER, vorticityData.size() * sizeof(glm::vec4), vorticityData.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 14, ssboVorticity);

	//not yet binded
	if (ssboSortedPos == 0) glGenBuffers(1, &ssboSortedPos);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboSortedPos);
	std::vector<glm::vec4> sortedPosData(maxParticles, glm::vec4(0.0f));
	glBufferData(GL_SHADER_STORAGE_BUFFER, sortedPosData.size() * sizeof(glm::vec4), sortedPosData.data(), GL_DYNAMIC_DRAW);

	if (ssboSortedVel == 0) glGenBuffers(1, &ssboSortedVel);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboSortedVel);
	std::vector < glm::vec4> sortedVelData(maxParticles, glm::vec4(0.0f));
	glBufferData(GL_SHADER_STORAGE_BUFFER, sortedVelData.size() * sizeof(glm::vec4), sortedVelData.data(), GL_DYNAMIC_DRAW);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	if (vao == 0) glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindVertexArray(0);

	std::cout << "Particle System: Initialized with " << maxParticles << " particles" << std::endl;

}

void ParticleSystem::Update(float deltaTime, float wallDamping, bool enableSPH) {
	uint32_t groups = (maxParticles + workGroupSize - 1) / workGroupSize;
	const float dt = std::min(deltaTime, 1.0f / 120.0f);
	
	//smoothing kernel radius
	const float h = GRID_CELL_SIZE;

	//particle spacing(particle radius * 4.0f) for more neighbours
	const float r = GRID_CELL_SIZE;
	const float dx = PARTICLE_SIM_RADIUS * 2.0f;
	const float mass = this->restDensity * dx * dx * dx;
	const int substeps = 2;
	bool usePredictedPositionsFlag = !enableSPH;
	//const float mass = 0.8 * pow(dx, 3) * this->restDensity;

	
	//density pass SPH

	if (enableSPH) {
		//for toggling
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboPos);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboVel);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, ssboDensity);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, ssboPressure);

		BuildUniformGrid(usePredictedPositionsFlag);
		//for boundary using same binding
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, ssboSortedPos);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, ssboSortedVel);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 15, ssboBoundaryGhostParticles);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, ssboBoundaryCellCount);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, ssboBoundaryCellStart);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 13, ssboBoundarySortedIndex);

		if (!sphDensityPressureComputeProgram || !sphForceIntegrateComputeProgram) return;

		sphDensityPressureComputeProgram->use();
		sphDensityPressureComputeProgram->setUint("particleCount", maxParticles);
		sphDensityPressureComputeProgram->setUint("gridResX", static_cast<uint32_t>(GRID_RES.x));
		sphDensityPressureComputeProgram->setUint("gridResY", static_cast<uint32_t>(GRID_RES.y));
		sphDensityPressureComputeProgram->setUint("gridResZ", static_cast<uint32_t>(GRID_RES.z));
		sphDensityPressureComputeProgram->setUint("boundaryCount", boundaryCount);
		sphDensityPressureComputeProgram->setFloat("h", h);
		sphDensityPressureComputeProgram->setFloat("mass", mass);
		sphDensityPressureComputeProgram->setFloat("PI", this->PI);
		sphDensityPressureComputeProgram->setFloat("restDensity", this->restDensity);
		sphDensityPressureComputeProgram->setFloat("stiffness", this->stiffness);
		sphDensityPressureComputeProgram->dispatch(groups, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		sphForceIntegrateComputeProgram->use();
		sphForceIntegrateComputeProgram->setUint("particleCount", maxParticles);
		sphForceIntegrateComputeProgram->setUint("gridResX", static_cast<uint32_t>(GRID_RES.x));
		sphForceIntegrateComputeProgram->setUint("gridResY", static_cast<uint32_t>(GRID_RES.y));
		sphForceIntegrateComputeProgram->setUint("gridResZ", static_cast<uint32_t>(GRID_RES.z));
		sphForceIntegrateComputeProgram->setFloat("h", h);
		sphForceIntegrateComputeProgram->setFloat("mass", mass);
		sphForceIntegrateComputeProgram->setFloat("PI", this->PI);
		sphForceIntegrateComputeProgram->setFloat("viscosityCoeff", this->viscosityCoeff);
		sphForceIntegrateComputeProgram->setFloat("deltaTime", dt);
		sphForceIntegrateComputeProgram->setVec3("boxMin", this->GRID_MIN);
		sphForceIntegrateComputeProgram->setVec3("boxMax", this->GRID_MAX);
		sphForceIntegrateComputeProgram->setFloat("particleRadius", this->PARTICLE_SIM_RADIUS);
		sphForceIntegrateComputeProgram->setVec3("gravity", this->GRAVITY);
		sphForceIntegrateComputeProgram->setFloat("wallDamping", wallDamping);
		sphForceIntegrateComputeProgram->dispatch(groups, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
	}
	else {
		//pbf 
		for (int s = 0; s < substeps; s++) {
			const float dtSub = dt / float(substeps);

			//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, ssboViscosityAccel);
			//for toggling
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboPos);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboVel);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, ssboDensity);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, ssboViscosityAccel); 
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, ssboPredPos);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, ssboLambda);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 13, ssboDeltaPos);

			uint32_t pbfIter = 0;
			if (!pbfPredictPosComputeProgram) return;
			pbfPredictPosComputeProgram->use();
			pbfPredictPosComputeProgram->setUint("particleCount", maxParticles);
			pbfPredictPosComputeProgram->setFloat("deltaTime", dtSub);
			pbfPredictPosComputeProgram->setVec3("boxMin", this->GRID_MIN);
			pbfPredictPosComputeProgram->setVec3("boxMax", this->GRID_MAX);
			pbfPredictPosComputeProgram->setFloat("particleRadius", this->PARTICLE_SIM_RADIUS);
			pbfPredictPosComputeProgram->setVec3("gravity", this->GRAVITY);
			pbfPredictPosComputeProgram->setFloat("wallDamping", wallDamping);
			pbfPredictPosComputeProgram->dispatch(groups, 1, 1);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboPredPos);
			BuildUniformGrid(usePredictedPositionsFlag);



			while (pbfIter < pbfSolverIterations) {
				//Grid here
				//bottleneck
				
				

				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, ssboBoundaryCellCount);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, ssboBoundaryCellStart);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, ssboBoundarySortedIndex);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 15, ssboBoundaryGhostParticles);

				if (!pbfLambdaComputeProgram) return;
				pbfLambdaComputeProgram->use();
				pbfLambdaComputeProgram->setUint("particleCount", maxParticles);
				pbfLambdaComputeProgram->setUint("gridResX", static_cast<uint32_t>(GRID_RES.x));
				pbfLambdaComputeProgram->setUint("gridResY", static_cast<uint32_t>(GRID_RES.y));
				pbfLambdaComputeProgram->setUint("gridResZ", static_cast<uint32_t>(GRID_RES.z));
				pbfLambdaComputeProgram->setFloat("h", h);
				pbfLambdaComputeProgram->setFloat("mass", mass);
				pbfLambdaComputeProgram->setFloat("PI", this->PI);
				pbfLambdaComputeProgram->setFloat("restDensity", this->restDensity);
				pbfLambdaComputeProgram->setFloat("relaxationEpsilon", this->pbfEpsilon);
				pbfLambdaComputeProgram->dispatch(groups, 1, 1);
				glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

				if (!pbfDeltaPosComputeProgram) return;
				pbfDeltaPosComputeProgram->use();
				pbfDeltaPosComputeProgram->setUint("particleCount", maxParticles);
				pbfDeltaPosComputeProgram->setUint("gridResX", static_cast<uint32_t>(GRID_RES.x));
				pbfDeltaPosComputeProgram->setUint("gridResY", static_cast<uint32_t>(GRID_RES.y));
				pbfDeltaPosComputeProgram->setUint("gridResZ", static_cast<uint32_t>(GRID_RES.z));
				pbfDeltaPosComputeProgram->setFloat("h", h);
				pbfDeltaPosComputeProgram->setFloat("mass", mass);
				pbfDeltaPosComputeProgram->setFloat("PI", this->PI);
				pbfDeltaPosComputeProgram->setFloat("restDensity", this->restDensity);
				pbfDeltaPosComputeProgram->setFloat("scorrK", this->pbfScorrK);
				pbfDeltaPosComputeProgram->setFloat("scorrN", this->pbfScorrN);
				pbfDeltaPosComputeProgram->setFloat("scorrDQ", this->pbfScorrDQ);
				pbfDeltaPosComputeProgram->dispatch(groups, 1, 1);
				glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

				if (!pbfApplyCorrComputeProgram) return;
				pbfApplyCorrComputeProgram->use();
				pbfApplyCorrComputeProgram->setUint("particleCount", maxParticles);
				pbfApplyCorrComputeProgram->setVec3("boxMin", this->GRID_MIN);
				pbfApplyCorrComputeProgram->setVec3("boxMax", this->GRID_MAX);
				pbfApplyCorrComputeProgram->setFloat("particleRadius", this->PARTICLE_SIM_RADIUS);
				pbfApplyCorrComputeProgram->setFloat("pbfRelaxation", this->pbfRelaxation);
				pbfApplyCorrComputeProgram->dispatch(groups, 1, 1);
				glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboPredPos);
				BuildUniformGrid(usePredictedPositionsFlag);

				pbfIter++;
			}

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboPos);

			if (!pbfIntegrateComputeProgram) return;
			pbfIntegrateComputeProgram->use();
			pbfIntegrateComputeProgram->setUint("particleCount", maxParticles);
			pbfIntegrateComputeProgram->setVec3("boxMin", this->GRID_MIN);
			pbfIntegrateComputeProgram->setVec3("boxMax", this->GRID_MAX);
			pbfIntegrateComputeProgram->setFloat("particleRadius", this->PARTICLE_SIM_RADIUS);
			pbfIntegrateComputeProgram->setFloat("deltaTime", dtSub);
			pbfIntegrateComputeProgram->setFloat("wallDamping", wallDamping);
			pbfIntegrateComputeProgram->setFloat("velocityDamping", 0.985f);
			pbfIntegrateComputeProgram->setFloat("maxVelocity", (h / glm::max(dtSub, 1e-6f)) * 1.25f);
			pbfIntegrateComputeProgram->dispatch(groups, 1, 1);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

			//rebind
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, ssboViscosityAccel);

			if (!pbfXSPHComputeProgram) return;
			pbfXSPHComputeProgram->use();
			pbfXSPHComputeProgram->setUint("particleCount", maxParticles);
			pbfXSPHComputeProgram->setUint("gridResX", static_cast<uint32_t>(GRID_RES.x));
			pbfXSPHComputeProgram->setUint("gridResY", static_cast<uint32_t>(GRID_RES.y));
			pbfXSPHComputeProgram->setUint("gridResZ", static_cast<uint32_t>(GRID_RES.z));
			pbfXSPHComputeProgram->setFloat("h", h);
			pbfXSPHComputeProgram->setFloat("mass", mass);
			pbfXSPHComputeProgram->setFloat("PI", this->PI);
			pbfXSPHComputeProgram->setFloat("restDensity", this->restDensity);
			pbfXSPHComputeProgram->setFloat("viscosityCoeff", this->viscosityCoeff);
			pbfXSPHComputeProgram->dispatch(groups, 1, 1);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

			if (!pbfXSPHApplyComputeProgram) return;
			pbfXSPHApplyComputeProgram->use();
			pbfXSPHApplyComputeProgram->setUint("particleCount", maxParticles);
			pbfXSPHApplyComputeProgram->dispatch(groups, 1, 1);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

			if (!pbfVorticityComputeProgram) return;
			pbfVorticityComputeProgram->use();
			pbfVorticityComputeProgram->setUint("particleCount", maxParticles);
			pbfVorticityComputeProgram->setUint("gridResX", static_cast<uint32_t>(GRID_RES.x));
			pbfVorticityComputeProgram->setUint("gridResY", static_cast<uint32_t>(GRID_RES.y));
			pbfVorticityComputeProgram->setUint("gridResZ", static_cast<uint32_t>(GRID_RES.z));
			pbfVorticityComputeProgram->setFloat("h", h);
			pbfVorticityComputeProgram->setFloat("mass", mass);
			pbfVorticityComputeProgram->setFloat("PI", this->PI);
			pbfVorticityComputeProgram->setFloat("restDensity", this->restDensity);
			pbfVorticityComputeProgram->dispatch(groups, 1, 1);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

			if (!pbfVorticityApplyComputeProgram) return;
			pbfVorticityApplyComputeProgram->use();
			pbfVorticityApplyComputeProgram->setUint("particleCount", maxParticles);
			pbfVorticityApplyComputeProgram->setUint("gridResX", static_cast<uint32_t>(GRID_RES.x));
			pbfVorticityApplyComputeProgram->setUint("gridResY", static_cast<uint32_t>(GRID_RES.y));
			pbfVorticityApplyComputeProgram->setUint("gridResZ", static_cast<uint32_t>(GRID_RES.z));
			pbfVorticityApplyComputeProgram->setFloat("h", h);
			pbfVorticityApplyComputeProgram->setFloat("mass", mass);
			pbfVorticityApplyComputeProgram->setFloat("PI", this->PI);
			pbfVorticityApplyComputeProgram->setFloat("deltaTime", dtSub);
			pbfVorticityApplyComputeProgram->setFloat("vorticityEpsilon", this->vorticityEpsilon);
			pbfVorticityApplyComputeProgram->dispatch(groups, 1, 1);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);	



		}


		

	}

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

	glDrawArrays(GL_POINTS, 0, maxParticles);
	glBindVertexArray(0);

}

void ParticleSystem::RenderBoundary(const glm::mat4& mvp, const glm::vec3& color, float pointSize) {
	if (!boundaryRenderShader || boundaryCount == 0 || ssboBoundaryGhostParticles == 0) return;
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 15, ssboBoundaryGhostParticles);
	boundaryRenderShader->use();
	boundaryRenderShader->setMat4("mvp", mvp);
	boundaryRenderShader->setVec3("color", color);
	boundaryRenderShader->setFloat("pointSize", pointSize);

	glBindVertexArray(vao);
	glDrawArrays(GL_POINTS, 0, boundaryCount);
	glBindVertexArray(0);
}

void ParticleSystem::RenderFluidDepth(const glm::mat4& mvp, float pointSize, const glm::vec2& viewportSize, const glm::mat4& projection, const glm::mat4& view) {
	if (!fluidDepthShader) return;
	
	glBindVertexArray(vao);
	fluidDepthShader->use();
	fluidDepthShader->setMat4("mvp", mvp);
	fluidDepthShader->setMat4("projection", projection);
	fluidDepthShader->setMat4("view", view);
	fluidDepthShader->setVec2("viewportSize", viewportSize);
	fluidDepthShader->setFloat("pointSize", pointSize);
	glDrawArrays(GL_POINTS, 0, maxParticles);
	glBindVertexArray(0);
}

void ParticleSystem::RenderFluidThickness(const glm::mat4& mvp, float pointSize, const glm::vec2& viewportSize, const glm::mat4& projection, const glm::mat4& view) {
	if (!fluidThicknessShader) return;

	glBindVertexArray(vao);
	fluidThicknessShader->use();
	fluidThicknessShader->setMat4("mvp", mvp);
	fluidThicknessShader->setMat4("projection", projection);
	fluidThicknessShader->setMat4("view", view);
	fluidThicknessShader->setVec2("viewportSize", viewportSize);
	fluidThicknessShader->setFloat("pointSize", pointSize);
	glDrawArrays(GL_POINTS, 0, maxParticles);
	glBindVertexArray(0);

}

void ParticleSystem::SetBounds(const glm::vec3& gridMin, const glm::vec3& gridMax, bool rebuild, bool reintializeParticles) {
	const glm::vec3 eps(0.001f);

	glm::vec3 newMin = glm::min(gridMin, gridMax - eps);
	glm::vec3 newMax = glm::max(gridMax, newMin + eps);


	GRID_MIN = newMin;
	GRID_MAX = newMax;

	if (rebuild) {
		InitializeGrid();
		InitializeBoundaryGhostParticles();
	}

	if (reintializeParticles) {
		InitializeParticles();
	}
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