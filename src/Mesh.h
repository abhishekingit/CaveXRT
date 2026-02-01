#pragma once

#include "Shader.h"
#include <glm/glm.hpp>
#include <vector>



struct Vertex {
	glm::vec3 position;
	glm::vec3 normals;
	//more attributes will be added later
};

class Mesh {
public:
	std::vector<Vertex> meshVertices;
	std::vector<uint32_t> meshIndices;
	//some more member variables will be added later

	Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

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


