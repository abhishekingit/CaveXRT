#pragma once

#include "Mesh.h"
#include "Shader.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/Logger.hpp"
#include "assimp/DefaultLogger.hpp"
#include "lodepng.h"

uint32_t TextureFromFile(const char* path, const std::string& directory, bool gamma = false);


class ModelLoader {
public:
	std::vector<Mesh> meshes;
	std::string modelDirectory;
	std::vector<Texture> loadedTextures;

	ModelLoader(const std::string& path);

	void Draw(Shader& shader) const;
	//void ComputeModelBoundingBox() const;
private:
	void loadModel(const std::string path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, TextureType texType);
	//some more functions related to materials and textures will be added later

};