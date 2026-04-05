#version 430 core
layout(std430, binding = 15) readonly buffer BoundaryPos { vec4 boundaryGhostPart[]; };

uniform mat4 mvp;
uniform float pointSize;

void main() {
    vec3 p = boundaryGhostPart[gl_VertexID].xyz;
    gl_Position = mvp * vec4(p, 1.0);
    gl_PointSize = pointSize;
}