#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out vec3 WorldPos;
out vec3 WorldNormal;

uniform mat4 mvp;
uniform mat4 modelView;
uniform mat4 model;

void main() {
	FragPos = vec3(modelView * vec4(aPos, 1.0));
	Normal = mat3(transpose(inverse(modelView))) * aNormal;
	TexCoords = aTexCoords;

	WorldPos = vec3(model * vec4(aPos, 1.0));
	WorldNormal = mat3(transpose(inverse(model))) * aNormal;

	gl_Position = (mvp * vec4(aPos, 1.0));
}