#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "lodepng.h"
#include <vector>
#include <string>

struct SkyboxConfig {
	bool enabled = true;
	std::string root = "../../../assets/skybox/sceneCubemap";
	std::vector<std::string> faces = {
		"cubemap_posx.png",
		"cubemap_negx.png",
		"cubemap_posy.png",
		"cubemap_negy.png",
		"cubemap_posz.png",
		"cubemap_negz.png"
	};
	float exposure = 1.0f;
};


inline uint32_t loadCubemap(const SkyboxConfig& config) {
	uint32_t textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	for (int i = 0; i < config.faces.size(); i++) {
		std::string filePath = config.root + "/" + config.faces[i];

		std::vector<unsigned char> image;
		uint32_t width, height;

		uint32_t error = lodepng::decode(image, width, height, filePath);

		if (error) {
			std::cout << "Texture failed to load at path: " << filePath << "\n" << "Lodepng error: " << error << ":" << lodepng_error_text(error) << std::endl;
			glDeleteTextures(1, &textureID);
			return 0;
		}

		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data());

		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	}
	return textureID;

}