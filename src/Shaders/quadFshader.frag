#version 330 core

in vec2 vUV;	

out vec4 outputColor;

uniform sampler2D renderTexture;

void main() {
	outputColor = texture(renderTexture, vUV);
}