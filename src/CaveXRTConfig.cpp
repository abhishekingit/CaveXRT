#pragma once

#include "CaveXRTConfig.h"
#include <iostream>

using nlohmann::json;

static glm::vec3 convVector(const json& j, const char* key, glm::vec3 fallback)
{
	if (!j.contains(key) || !j[key].is_array() || j[key].size() < 3) {
		return fallback;
	}

	glm::vec3 result;
	for (int i = 0; i < 3; ++i) {
		result[i] = j[key][i].get<float>();
	}
	return result;
}


CaveXRTConfig loadConfig(const std::string& jsonFilePath) {
	CaveXRTConfig caveXRTConfig;
	json jsonConfig;
	
	try {

		std::ifstream configFile(jsonFilePath);
		if (!configFile) {
			std::cerr << "Could not open config json file: " << jsonFilePath << std::endl;
			return caveXRTConfig;
		}
		jsonConfig = nlohmann::json::parse(configFile);

		caveXRTConfig.skyboxConfig.enabled = jsonConfig.value("skyboxEnabled", caveXRTConfig.skyboxConfig.enabled);
		caveXRTConfig.skyboxConfig.root = jsonConfig.value("skyboxRootDirectory", caveXRTConfig.skyboxConfig.root);

		if (jsonConfig.contains("skyboxFaces")) {
			for (size_t i = 0; i < caveXRTConfig.skyboxConfig.faces.size(); i++) {
				caveXRTConfig.skyboxConfig.faces[i] = jsonConfig["skyboxFaces"][i].get<std::string>();
			}
		}
		caveXRTConfig.skyboxConfig.exposure = jsonConfig.value("skyboxExposure", caveXRTConfig.skyboxConfig.exposure);

		caveXRTConfig.backgroundColor = convVector(jsonConfig, "backgroundColor", caveXRTConfig.backgroundColor);
		caveXRTConfig.lightColor = convVector(jsonConfig, "lightColor", caveXRTConfig.lightColor);
		
		caveXRTConfig.lightYaw = jsonConfig.value("lightYaw", caveXRTConfig.lightYaw);
		caveXRTConfig.lightPitch = jsonConfig.value("lightPitch", caveXRTConfig.lightPitch);
		caveXRTConfig.lightRadius = jsonConfig.value("lightRadius", caveXRTConfig.lightRadius);

		caveXRTConfig.yaw = jsonConfig.value("yaw", caveXRTConfig.yaw);
		caveXRTConfig.pitch = jsonConfig.value("pitch", caveXRTConfig.pitch);
		caveXRTConfig.distance = jsonConfig.value("distance", caveXRTConfig.distance);
		caveXRTConfig.rotationSpeed = jsonConfig.value("rotationSpeed", caveXRTConfig.rotationSpeed);
		caveXRTConfig.zoomSpeed = jsonConfig.value("zoomSpeed", caveXRTConfig.zoomSpeed);
		caveXRTConfig.cameraTarget = convVector(jsonConfig, "cameraTarget", caveXRTConfig.cameraTarget);
		caveXRTConfig.cameraUp = convVector(jsonConfig, "cameraUp", caveXRTConfig.cameraUp);

		caveXRTConfig.Ka = convVector(jsonConfig, "Ka", caveXRTConfig.Ka);
		caveXRTConfig.Kd = convVector(jsonConfig, "Kd", caveXRTConfig.Kd);
		caveXRTConfig.Ks = convVector(jsonConfig, "Ks", caveXRTConfig.Ks);

		caveXRTConfig.ambientIntensity = jsonConfig.value("ambientIntensity", caveXRTConfig.ambientIntensity);
		caveXRTConfig.specularIntensity = jsonConfig.value("specularIntensity", caveXRTConfig.specularIntensity);
		caveXRTConfig.glossiness = jsonConfig.value("glossiness", caveXRTConfig.glossiness);

		caveXRTConfig.width = jsonConfig.value("width", caveXRTConfig.width);
		caveXRTConfig.height = jsonConfig.value("height", caveXRTConfig.height);

		
	}
	catch (const nlohmann::json::exception& e) {
		std::cerr << "Config JSON error: " << e.what() << std::endl;
	}
	return caveXRTConfig;
}


void CaveXRTConfig::apply(Shader& mainShader, Shader& lightShader) const {

	//mainShader
	mainShader.setVec3("light.color", lightColor);

	mainShader.setFloat("material.ambientIntensity", ambientIntensity);
	mainShader.setFloat("material.specularIntensity", specularIntensity);


	//lightShader
	lightShader.setVec3("lightcolor", lightColor);
}