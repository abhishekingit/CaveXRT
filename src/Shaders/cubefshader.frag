#version 410 core

out vec4 outputColor;

struct Material {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float ambientIntensity;
	float specularIntensity;
	float glossiness;

	bool hasDiffuseMap;
	bool hasSpecularMap;
	bool hasBumpMap;

	sampler2D map_Ka;
	sampler2D map_Kd;
	sampler2D map_Ks;
};

uniform Material material;

uniform vec3 lightColor;

void main() {
	vec3 diffuse = material.diffuse;

	outputColor = vec4(diffuse, 1.0);
}