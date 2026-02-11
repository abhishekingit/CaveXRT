#pragma once

#include "Shader.h"
#include <nlohmann/json.hpp>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

struct CaveXRTConfig {
	glm::vec3 backgroundColor = glm::vec3(0.1f, 0.1f, 0.1f);
	glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
	
	//light properties 
	float lightYaw = 0.0f;
	float lightPitch = 0.3f;
	float lightRadius = 2.0f;

	//camera properties
	float yaw = 0.0f;
	float pitch = 0.0f;
	float distance = 5.0f;
	float rotationSpeed = 0.005f;
	float zoomSpeed = 0.01f;
	glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);


	//material properties
	glm::vec3 Ka = glm::vec3(0.9f, 0.5f, 0.5f);
	glm::vec3 Kd = glm::vec3(0.9f, 0.2f, 0.1f);
	glm::vec3 Ks = glm::vec3(1.0f, 1.0f, 1.0f);
	float ambientIntensity = 0.2f;
	float specularIntensity = 1.0;
	float glossiness = 128;


	//window properties 
	int width = 800;
	int height = 600;

	void apply(Shader& mainShader, Shader& lightShader) const;

};

CaveXRTConfig loadConfig(const std::string& jsonFilePath);