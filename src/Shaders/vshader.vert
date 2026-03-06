#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

out VS_OUT {
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
	vec4 FragPosLightSpace;
	vec3 WorldPos;
	vec3 WorldNormal;
} vs_out;

uniform mat4 mvp;
uniform mat4 modelView;
uniform mat4 model;
uniform mat4 lightSpaceMatrix;

void main() {
	vs_out.FragPos = vec3(modelView * vec4(aPos, 1.0));
	vs_out.Normal = mat3(transpose(inverse(modelView))) * aNormal;
	vs_out.TexCoords = aTexCoords;

	vs_out.WorldPos = vec3(model * vec4(aPos, 1.0));
	vs_out.WorldNormal = mat3(transpose(inverse(model))) * aNormal;

	vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.WorldPos, 1.0);

	gl_Position = (mvp * vec4(aPos, 1.0));
}