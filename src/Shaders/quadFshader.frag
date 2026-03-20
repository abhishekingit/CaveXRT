#version 330 core

in vec2 vUV;	
in vec3 WorldPos;
in vec3 WorldNormal;
in vec4 ReflectionClip;

out vec4 outputColor;

uniform vec3 cameraPosWorld;
uniform bool skyboxEnabled;

uniform float width;
uniform float height;

uniform sampler2D renderTexture;
uniform samplerCube cubemaptexture;

//checker texture procedural

uniform float checkerScale;
uniform float checkerColorStrength;
uniform float envBlend;
uniform float desaturationVal;

vec3 desaturate(vec3 color, float amount) {
	float l = dot(color, vec3(0.2126, 0.7152, 0.0722));
	return mix(color, vec3(l), amount);
}

void main() {
	vec2 uv = vec2(gl_FragCoord.x / width, gl_FragCoord.y / height);
    vec3 reflectionColor = texture(renderTexture, uv).rgb;
	vec3 envColor = vec3(0.0);
	if(skyboxEnabled) {
		vec3 viewDir = normalize(cameraPosWorld - WorldPos);
		vec3 reflectDir = reflect(-viewDir, normalize(WorldNormal));
		envColor = texture(cubemaptexture, reflectDir).rgb;
	}

	vec3 pink   = vec3(0.82, 0.67, 0.76);
    vec3 blue   = vec3(0.62, 0.72, 0.86);
    vec3 yellow = vec3(0.86, 0.82, 0.60);
    vec3 green  = vec3(0.64, 0.82, 0.66);
	
	vec3 quadrantColor;
	if(vUV.x < 0.5 && vUV.y < 0.5) quadrantColor = pink;
	else if(vUV.x >= 0.5 && vUV.y < 0.5) quadrantColor = blue;
	else if(vUV.x <0.5 && vUV.y >= 0.5) quadrantColor = yellow;
	else quadrantColor = green;

	vec2 checkerUV = vUV * checkerScale;
	vec2 cell = floor(checkerUV);
	float checker = mod(cell.x + cell.y, 2.0);
	float shade = mix(1.0 - checkerColorStrength, 1.0, checker);

	vec3 planeColor = quadrantColor * shade;

	planeColor = desaturate(planeColor, desaturationVal);

	vec3 finalColor = planeColor;

	//will later maybe add envBlend

	//vec3 finalColor = mix(reflectionColor, envColor, 0.5);
	outputColor = vec4(finalColor, 1.0);
}