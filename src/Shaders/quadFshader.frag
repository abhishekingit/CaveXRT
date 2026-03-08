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

void main() {
	vec2 uv = vec2(gl_FragCoord.x / width, gl_FragCoord.y / height);
    vec3 reflectionColor = texture(renderTexture, uv).rgb;
	vec3 envColor = vec3(0.0);
	if(skyboxEnabled) {
		vec3 viewDir = normalize(cameraPosWorld - WorldPos);
		vec3 reflectDir = reflect(-viewDir, normalize(WorldNormal));
		envColor = texture(cubemaptexture, reflectDir).rgb;
	}
	vec3 finalColor = vec3(0.4, 0.4, 0.4);

	//vec3 finalColor = mix(reflectionColor, envColor, 0.5);
	outputColor = vec4(finalColor, 1.0);
}