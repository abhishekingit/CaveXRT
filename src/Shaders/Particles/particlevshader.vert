#version 430 core

layout(std430, binding = 0) buffer Particles {
	vec4 positions[];
};

uniform mat4 mvp;
uniform float pointSize;

void main() {
	vec3 p = positions[gl_VertexID].xyz;
	gl_Position = mvp *  vec4(p, 1.0);
	gl_PointSize = pointSize;
}