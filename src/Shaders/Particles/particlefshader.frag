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
	vec3 c0 = vec3(0.20, 0.06, 0.45);
	vec3 c1 = vec3(0.18, 0.34, 0.75);
	vec3 c2 = vec3(0.16, 0.62, 0.70);
	vec3 c3 = vec3(0.58, 0.82, 0.38);
	vec3 c4 = vec3(0.98, 0.83, 0.22);
	vec3 c5 = vec3(0.97, 0.39, 0.20);

	float s0 = smoothstep(0.00, 0.20, t);
	float s1 = smoothstep(0.20, 0.40, t);
	float s2 = smoothstep(0.40, 0.60, t);
	float s3 = smoothstep(0.60, 0.80, t);
	float s4 = smoothstep(0.80, 1.00, t);

	vec3 col = mix(c0, c1, s0);
	col = mix(col, c2, s1);
	col = mix(col, c3, s2);
	col = mix(col, c4, s3);
	col = mix(col, c5, s4);
	return col;

}

vec3 safeNormalize(vec3 v) {
	float m2 = dot(v, v);
	return (m2 > 1e-8) ? v * inversesqrt(m2) : vec3(0.0, 0.0, 1.0);
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
		baseColor = densityRamp(smoothstep(0.0, 1.0, t));

	}
	else if(debugCellColor) {
		 baseColor = colorParticles(CellID, gridRes);
	}

	vec3 lightDir = normalize(lightPosView - fragViewPos);
	vec3 viewDir = normalize(-fragViewPos);
	vec3 halfVec = safeNormalize(lightDir + viewDir);

	float ndl = dot(normalViewSpace, lightDir);
	float diff = clamp((ndl + 0.30) / 1.30, 0.0, 1.0);
	float spec = pow(max(0.0, dot(normalViewSpace, halfVec)), 16.0);
	float rim = pow(1.0 - max(0.0, dot(normalViewSpace, viewDir)), 2.0);

	vec3 ambient = baseColor * 0.28;
	vec3 diffuse = baseColor * (0.72 * diff);
	vec3 specular = vec3(1.0) * (0.18 * spec);
	vec3 rimLight = baseColor * (0.12 * rim);
	vec3 color = ambient + diffuse + specular + rimLight;

	FragColor = vec4(color, 1.0);
}