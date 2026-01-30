#pragma once

#include "ModelLoader.h"
#include <iostream>
#include <fstream>

ModelLoader::ModelLoader(const std::string& path) {
	loadModel(path);
}

void ModelLoader::Draw(Shader& shader) const {
	for (uint32_t i = 0; i < meshes.size(); i++) {
		meshes[i].Draw(shader);
	}
}

Mesh ModelLoader::processMesh(aiMesh* mesh, const aiScene* scene) {
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
		Vertex vert{
			.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z)
		};
		vertices.push_back(vert);
	}

	for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (uint32_t j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}
	//materials and textures would come here 

	return Mesh(vertices, indices);
	
}

void ModelLoader::processNode(aiNode* node, const aiScene* scene) {
	for (uint32_t i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(processMesh(mesh, scene));
	}

	for (uint32_t i = 0; i < node->mNumChildren; i++) {
		processNode(node->mChildren[i], scene);
	}
}

void ModelLoader::loadModel(const std::string path) {
	Assimp::Importer importer;
	Assimp::DefaultLogger::create("IMPORT_modelLog.txt", Assimp::Logger::VERBOSE);

	std::ifstream modelFile(path);
	if (!modelFile) {
		std::cerr << "File not found!" << std::endl;
	}

	Assimp::DefaultLogger::get()->info("Starting Import..");
	//will later add more postprocessing operations for Normals and other stuff
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate);
		
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cout << "ERROR::ASSIMP" << importer.GetErrorString() << std::endl;

	}
	
	Assimp::DefaultLogger::kill();
	modelDirectory = path.substr(0, path.find_last_of('/'));
	processNode(scene->mRootNode, scene);

}
