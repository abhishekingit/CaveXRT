#version 430 core

out vec4 FragColor;

uniform vec3 particleColor;

void main() {
	float r = length(gl_PointCoord - vec2(0.5));
	if(r > 0.5) discard;
	FragColor = vec4(particleColor, 1.0);
}