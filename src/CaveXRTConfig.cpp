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

		caveXRTConfig.backgroundColor = convVector(jsonConfig, "backgroundColor", caveXRTConfig.backgroundColor);
		caveXRTConfig.lightColor = convVector(jsonConfig, "lightColor", caveXRTConfig.lightColor);
		
	}
	catch (const nlohmann::json::exception& e) {
		std::cerr << "Config JSON error: " << e.what() << std::endl;
	}
	return caveXRTConfig;
}


void CaveXRTConfig::apply(Shader& mainShader, Shader& lightShader) const {

	//mainShader
	mainShader.setVec3("light.color", lightColor);


	//lightShader
	lightShader.setVec3("lightcolor", lightColor);
}