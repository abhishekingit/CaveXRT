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
	std::vector<Texture> textures;
	Material mtl;

	for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
		Vertex vert;
		vert.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

		if (mesh->HasNormals()) {
			vert.normals = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
		}

		if (mesh->HasTextureCoords(0)) {
			vert.texCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
		}

		
		vertices.push_back(vert);
	}

	for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (uint32_t j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}
	//materials and textures would come here 
	if (mesh->mMaterialIndex >= 0 && scene->HasMaterials()) {
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		aiColor3D color(0.0f, 0.0f, 0.0f);
		float shininess = 0.0f;

		if (material->Get(AI_MATKEY_COLOR_AMBIENT, color) != AI_SUCCESS) {
			color = aiColor3D(0.2f, 0.2f, 0.2f);
		}
		mtl.Ka = glm::vec3(color.r, color.g, color.b);

		if (material->Get(AI_MATKEY_COLOR_DIFFUSE, color) != AI_SUCCESS) {
			color = aiColor3D(0.5f, 0.5f, 0.5f);
		}
		mtl.Kd = glm::vec3(color.r, color.g, color.b);

		if (material->Get(AI_MATKEY_COLOR_SPECULAR, color) != AI_SUCCESS) {
			color = aiColor3D(0.5f, 0.5f, 0.5f);
		}
		mtl.Ks = glm::vec3(color.r, color.g, color.b);

		if (material->Get(AI_MATKEY_SHININESS, shininess) != AI_SUCCESS) {
			shininess = 32.0f;
		}
		mtl.Ns = shininess;


	}

	if (mesh->mMaterialIndex >= 0) {
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		std::vector<Texture> ambientMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, TextureType::AMBIENT);
		if (!ambientMaps.empty()) mtl.map_Ka = ambientMaps[0];
		std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, TextureType::DIFFUSE);
		if(!diffuseMaps.empty()) mtl.map_Kd = diffuseMaps[0];			
		std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, TextureType::SPECULAR);
		if(!specularMaps.empty()) mtl.map_Ks = specularMaps[0];
		std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, TextureType::HEIGHT);
		if (!heightMaps.empty()) mtl.map_bump = heightMaps[0];
		

		//will add more support
		/*std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, TextureType::NORMAL);
		
		std::vector<Texture> metallicMaps = loadMaterialTextures(material, aiTextureType_REFLECTION, TextureType::METALLIC);
		textures.insert(textures.end(), metallicMaps.begin(), metallicMaps.end());*/
		
		//maybe a bool flag to use for shaders
	}


	return Mesh(vertices, indices, textures, mtl);
	
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
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);
		
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cout << "ERROR::ASSIMP" << importer.GetErrorString() << std::endl;

	}
	
	Assimp::DefaultLogger::kill();
	modelDirectory = path.substr(0, path.find_last_of('/'));
	processNode(scene->mRootNode, scene);

}


std::vector<Texture> ModelLoader::loadMaterialTextures(aiMaterial* mat, aiTextureType type, TextureType texType) {
	std::vector<Texture> textures;
	for (uint32_t i = 0; i < mat->GetTextureCount(type); i++) {
		aiString str;
		mat->GetTexture(type, i, &str);
		bool skip = false;
		for (uint32_t j = 0; j < loadedTextures.size(); j++) {
			if (std::strcmp(loadedTextures[j].path.data(), str.C_Str()) == 0) {
				textures.push_back(loadedTextures[j]);
				skip = true;
				break;
			}
		}

		if (!skip) {
			Texture texture;
			texture.id = TextureFromFile(str.C_Str(), modelDirectory);
			texture.type = texType;
			texture.path = str.C_Str();
			textures.push_back(texture);
			loadedTextures.push_back(texture);
		}
	}
	return textures;
}

void flipImageVertically(std::vector<unsigned char>& image, uint32_t width, uint32_t height) {
	const size_t stride = width * 4; 
	std::vector<unsigned char> temp(stride);

	for (uint32_t y = 0; y < height / 2; y++) {
		unsigned char* row = &image[y * stride];
		unsigned char* oppositeRow = &image[(height - 1 - y) * stride];

		memcpy(temp.data(), row, stride);
		memcpy(row, oppositeRow, stride);
		memcpy(oppositeRow, temp.data(), stride);
	}
}

uint32_t TextureFromFile(const char* path, const std::string& directory, bool gamma) {
	std::string filename = directory + '/' + std::string(path);

	std::vector<unsigned char> image;
	uint32_t width, height;

	uint32_t error = lodepng::decode(image, width, height, filename);
	if (error) {
		std::cout << "Texture failed to load at path: " << filename << "\n" << "Lodepng error: " << error << ":" << lodepng_error_text(error) << std::endl;
		return 0;
	}

	uint32_t textureID;
	glGenTextures(1, &textureID);

	glBindTexture(GL_TEXTURE_2D, textureID);

	//flipImageVertically(image, width, height);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data());

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return textureID;
}
