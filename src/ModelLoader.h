#pragma once

#include "Mesh.h"
#include "Shader.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/Logger.hpp"
#include "assimp/DefaultLogger.hpp"



class ModelLoader {
public:
	std::vector<Mesh> meshes;
	std::string modelDirectory;

	ModelLoader(const std::string& path);

	void Draw(Shader& shader) const;
	//void ComputeModelBoundingBox() const;
private:
	void loadModel(const std::string path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	//some more functions related to materials and textures will be added later

};