#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aUV;

out vec2 vUV;
out vec3 WorldPos;
out vec3 WorldNormal;
out vec4 ReflectionClip;

uniform mat4 mvp;
uniform mat4 model;
uniform mat4 reflectionVP;


void main() {
	vUV = aUV;

	WorldPos = vec3(model * vec4(aPos, 1.0));
	WorldNormal = mat3(transpose(inverse(model))) * vec3(0.0, 0.0, 1.0);
	ReflectionClip = reflectionVP * vec4(WorldPos, 1.0);
	gl_Position = mvp * vec4(aPos, 1.0);
}