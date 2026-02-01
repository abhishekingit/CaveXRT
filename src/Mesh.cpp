#pragma once

#include "Mesh.h"
#include <glad/glad.h>
#include <iostream>


Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices): meshVertices(vertices), meshIndices(indices) {
	ComputeBoundingBox(vertices);
	setupMesh();
}

void Mesh::Draw(Shader& shader) const {
	
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, meshIndices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

glm::vec3 Mesh::getBoxMin() const { return boxMin; }
glm::vec3 Mesh::getBoxMax() const { return boxMax; }
glm::vec3 Mesh::getCenter() const { return (boxMin + boxMax) * 0.5f; }

void Mesh::setupMesh() {
	std::cout << "Loading the mesh data ..." << std::endl;

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, meshVertices.size() * sizeof(Vertex), &meshVertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshIndices.size() * sizeof(uint32_t), &meshIndices[0], GL_STATIC_DRAW);

	//vertices 
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

	//normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, normals)));

	glBindVertexArray(0);

}

void Mesh::ComputeBoundingBox(const std::vector<Vertex>& vertices) {
	if (vertices.empty()) {
		boxMin = glm::vec3(1.0f);
		boxMax = glm::vec3(0.0f);
		return;
	}

	boxMin = vertices[0].position;
	boxMax = vertices[0].position;

	for (uint32_t i = 0; i < vertices.size(); i++) {
		boxMin = glm::min(boxMin, vertices[i].position);
		boxMax = glm::max(boxMax, vertices[i].position);
	}
}