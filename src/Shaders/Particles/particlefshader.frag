#version 430 core

in vec3 ViewPos;
in float RadiusView;

out vec4 FragColor;

uniform vec3 particleColor;
uniform mat4 projection;
uniform vec3 lightPosView;

void main() {
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


	vec3 lightDir = normalize(lightPosView - fragViewPos);
	vec3 viewDir = normalize(-fragViewPos);
	vec3 halfVec = normalize(lightDir + viewDir);

	float diff = max(0.0, dot(normalViewSpace, lightDir));
	float spec = pow(max(0.0, dot(normalViewSpace, halfVec)), 32.0);
	vec3 color = particleColor * (0.1 + 0.9 * diff) + vec3(1.0) * spec;

	FragColor = vec4(color, 1.0);
}