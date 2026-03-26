#version 430 core

in vec3 ViewPos;
in float RadiusView;
in float ParticleDensity;
flat in uint CellID;


out vec4 FragColor;

uniform vec3 particleColor;
uniform mat4 projection;
uniform vec3 lightPosView;
uniform bool debugDensityColor;
uniform bool debugCellColor;
uniform vec3 gridRes;
uniform vec2 densityMinMax;

vec3 colorParticles(uint id, vec3 res) {
	float rx = max(1.0, res.x - 1.0);
    float ry = max(1.0, res.y - 1.0);
    float rz = max(1.0, res.z - 1.0);

    float x = float(id % uint(res.x));
    float y = float((id / uint(res.x)) % uint(res.y));
    float z = float(id / uint(res.x * res.y));

    return vec3(x / rx, y / ry, z / rz);

}

vec3 densityRamp(float t) {
	vec3 c0 = vec3(0.10, 0.30, 1.00);
    vec3 c1 = vec3(0.10, 0.90, 1.00);
    vec3 c2 = vec3(0.95, 0.90, 0.20);
    vec3 c3 = vec3(1.00, 0.25, 0.20);

    if (t < 0.33) return mix(c0, c1, t / 0.33);
    if (t < 0.66) return mix(c1, c2, (t - 0.33) / 0.33);
    return mix(c2, c3, (t - 0.66) / 0.34);

}

void main() {
	//centered coords and sphere like normals for billboard particles
	vec2 coord = (gl_PointCoord - vec2(0.5)) * 2.0;
	float r2 = dot(coord, coord);
	if(r2 > 1.0) discard;

	float z = sqrt(1.0 - r2);

	vec3 normalViewSpace = normalize(vec3(coord.x, coord.y, z));

	vec3 fragViewPos = ViewPos + normalViewSpace * RadiusView;

	vec4 clip = projection * vec4(fragViewPos, 1.0);
	float ndcDepth = clip.z / clip.w;
	float windowDepth = ndcDepth * 0.5 + 0.5;
	gl_FragDepth = windowDepth;

	vec3 baseColor = particleColor;

	if(debugDensityColor) {
		float dMin = densityMinMax.x;
		float dMax = max(densityMinMax.y, dMin + 1e-6);
		float t = clamp((ParticleDensity - dMin) / (dMax - dMin), 0.0, 1.0);
		baseColor = densityRamp(t);

	}
	else if(debugCellColor) {
		 baseColor = colorParticles(CellID, gridRes);
	}

	vec3 lightDir = normalize(lightPosView - fragViewPos);
	vec3 viewDir = normalize(-fragViewPos);
	vec3 halfVec = normalize(lightDir + viewDir);

	float diff = max(0.0, dot(normalViewSpace, lightDir));
	float spec = pow(max(0.0, dot(normalViewSpace, halfVec)), 32.0);
	vec3 color = baseColor * (0.1 + 0.9 * diff) + vec3(1.0) * spec;

	FragColor = vec4(color, 1.0);
}