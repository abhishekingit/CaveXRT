#version 430 core

layout(std430, binding = 0) buffer Particles {
	vec4 positions[];
};

//from the uniform grid
layout(std430, binding = 5) buffer ParticleCell {
    uint particleCell[];
};

layout(std430, binding = 7) buffer ParticleDensities { 
    float densityBuf[]; 
};

uniform mat4 mvp;
uniform float pointSize;
uniform mat4 view;
uniform mat4 projection;
uniform vec2 viewportSize;


out vec3 ViewPos;
out float RadiusView;
flat out uint CellID;
out float ParticleDensity;

void main() {
	vec3 p = positions[gl_VertexID].xyz;
	vec4 viewPos4 = view * vec4(p, 1.0);
	ViewPos = viewPos4.xyz;

	
    float dist = max(0.0001, -ViewPos.z);

   
    float proj11 = projection[1][1];

    float particleRadius = pointSize;

    
    float pixelDiameter = particleRadius * proj11 * viewportSize.y / dist;
    RadiusView = particleRadius;

    CellID = particleCell[gl_VertexID];
    ParticleDensity = densityBuf[gl_VertexID];

    
    gl_PointSize = clamp(pixelDiameter, 1.0, 256.0);

    gl_Position = projection * viewPos4;
}