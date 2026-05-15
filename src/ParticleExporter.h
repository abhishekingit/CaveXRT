#pragma once

#include <vector>
#include <filesystem>
#include <cstdint>
#include <glm/glm.hpp>

struct ParticleFrameHeader {
	uint32_t magic;
	uint32_t version;
	uint32_t frameIndex;
	float timeSeconds;
	uint32_t particleCount;
};


class ParticleExporter {
public:
	bool BeginSession(const char* outputPath, int fps, float radius);
	bool WriteFrame(int frameIndex, float timeSeconds, const std::vector<glm::vec4>& positions, const std::vector<glm::vec4>& velocities);
	void EndSession();
	bool IsExporting() const { return m_isExporting; }

private:
	bool WriteManifest() const;
	std::filesystem::path BuildFramePath(int frameIndex) const;

	std::filesystem::path m_outputDir;
	int m_fps = 60;
	float m_particleRadius = 0.0f;
	int m_frameCount = 0;
	bool m_isExporting = false;

};