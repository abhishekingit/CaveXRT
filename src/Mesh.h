#pragma once

#include "Shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

struct Vertex {
	glm::vec3 position;
	glm::vec3 normals;
	glm::vec2 texCoords;
	//more attributes will be added later
};

enum TextureType {
	AMBIENT,
	SPECULAR,
	DIFFUSE,
	NORMAL,
	HEIGHT,
	BASE_COLOR,
	EMISSIVE,
	METALLIC,
	ROUGHNESS
};

struct Texture {
	uint32_t id;
	TextureType type;
	std::string path;
};

struct Material {
	glm::vec3 Ka;
	glm::vec3 Kd;
	glm::vec3 Ks;
	float Ns;
	Texture map_Ka;
	Texture map_Kd;
	Texture map_Ks;
	Texture map_bump;
};

class Mesh {
public:
	std::vector<Vertex> meshVertices;
	std::vector<uint32_t> meshIndices;
	std::vector<Texture> meshTextures;
	Material mtl;
	//some more member variables will be added later

	Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const std::vector<Texture>& textures, const Material& material);

	void Draw(Shader& shader) const;

	glm::vec3 getBoxMin() const;
	glm::vec3 getBoxMax() const;
	glm::vec3 getCenter() const;

private:
	uint32_t VAO, VBO, EBO;
	uint32_t indexCount;

	glm::vec3 boxMin, boxMax;

	void setupMesh();
	void ComputeBoundingBox(const std::vector<Vertex>& vertices);

};


