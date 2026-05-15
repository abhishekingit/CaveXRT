#include "ParticleExporter.h"
#include <string>
#include <iostream>
#include <sstream>
#include <filesystem>
#include <fstream>
#include <iomanip>

namespace {
	constexpr uint32_t PARTICLE_FRAME_MAGIC = 0x31505843l; // "CXP1" in ASCII
	constexpr uint32_t PARTICLE_FRAME_VERSION = 1u;

	template<typename T>
	bool WritePod(std::ofstream& out, const T& value) {
		out.write(reinterpret_cast<const char*>(&value), sizeof(T));
		return static_cast<bool>(out);
	}


	bool WriteFloatVec3Array(std::ofstream& out, const std::vector<glm::vec4>& data) {
		for (const auto& v : data) {
			out.write(reinterpret_cast<const char*>(&v.x), sizeof(float));
			out.write(reinterpret_cast<const char*>(&v.y), sizeof(float));
			out.write(reinterpret_cast<const char*>(&v.z), sizeof(float));

			if (!out) {
				return false;
			}
		}
		return true;
	}
}

bool ParticleExporter::BeginSession(const char* outputPath, int fps, float radius) {
	if (outputPath == nullptr || outputPath[0] == '\0') {
		return false;
	}

	if (m_isExporting) {
		EndSession();
	}

	m_outputDir = std::filesystem::path(outputPath);
	m_fps = (fps > 0) ? fps : 60;
	m_particleRadius = radius;
	m_frameCount = 0;

	std::error_code ec;
	std::filesystem::create_directories(m_outputDir, ec);

	if (ec) {
		m_isExporting = false;
		return false;
	}

	m_isExporting = true;
	return true;

}

bool ParticleExporter::WriteFrame(int frameIndex, float timeSeconds, const std::vector<glm::vec4>& positions, const std::vector<glm::vec4>& velocities) {
	if (!m_isExporting) {
		return false;
	}

	if (positions.size() != velocities.size()) {
		return false;
	}

	const std::filesystem::path framePath = BuildFramePath(frameIndex);
	std::ofstream out(framePath, std::ios::binary);
	if (!out) {
		return false;
	}

	ParticleFrameHeader header{};
	header.magic = PARTICLE_FRAME_MAGIC;
	header.version = PARTICLE_FRAME_VERSION;
	header.frameIndex = static_cast<uint32_t>(frameIndex);
	header.timeSeconds = timeSeconds;
	header.particleCount = static_cast<uint32_t>(positions.size());

	if (!WritePod(out, header)) {
		return false;
	}

	if (!WriteFloatVec3Array(out, positions)) {
		return false;
	}

	if (!WriteFloatVec3Array(out, velocities)) {
		return false;
	}

	if (!out) {
		return false;
	}

	m_frameCount++;
	return true;
}

void ParticleExporter::EndSession() {
	if (!m_isExporting) {
		return;
	}

	//Write manifest here
	WriteManifest();

	m_isExporting = false;
	m_frameCount = 0;

}

bool ParticleExporter::WriteManifest() const {
	try {
		const std::filesystem::path manifestPath = m_outputDir / "manifest.json";
		std::ofstream out(manifestPath, std::ios::binary);

		if (!out) {
			return false;
		}

		std::ostringstream ss;
		ss << "{\n";
		ss << " \"format\": " << "\"CXP1\"" << ", \n";
		ss << " \"version\": " << PARTICLE_FRAME_VERSION << ",\n";
		ss << " \"fps\": " << m_fps << ",\n";
		ss << " \"particleRadius\": " << m_particleRadius << ",\n";
		ss << " \"frameCount\": " << m_frameCount << ",\n";
		ss << " \"framePattern\": \"frame_%06d.bin\"\n";
		ss << "}\n";

		const std::string text = ss.str();
		out.write(text.data(), static_cast<std::streamsize>(text.size()));
		return static_cast<bool>(out);

	}
	catch (...) {
		return false;
	}
}

std::filesystem::path ParticleExporter::BuildFramePath(int frameIndex) const {
	std::ostringstream ss;
	ss << "frame_" << std::setw(6) << std::setfill('0') << frameIndex << ".bin";
	return m_outputDir / ss.str();
}