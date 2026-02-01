#version 330 core

in vec3 FragPos;
in vec3 Normal;
	
out vec4 outputColor;


struct Material {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float ambientIntensity;
	float specularIntensity;
	float glossiness;
};

struct Light {
	vec3 position;
	vec3 color;
};

uniform Material material;
uniform Light light;
uniform vec3 viewPos;

void main() {
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(light.position - FragPos);
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 halfVec = normalize(lightDir + viewDir);

	float cosTheta = max(dot(norm, lightDir), 0.0);
	float cosAlpha = max(dot(norm, halfVec), 0.0);

	vec3 ambient = material.ambient * material.ambientIntensity * light.color;
	vec3 diffuse = material.diffuse * cosTheta * light.color;
	vec3 specular = material.specular * pow(cosAlpha, material.glossiness) * material.specularIntensity * light.color;

	float rim = 1.0 - max(dot(viewDir, norm), 0.0);
	rim = pow(rim, 2.0);
	vec3 rimColor = rim * vec3(0.1, 0.4, 0.5) * 0.5;

	outputColor = vec4(ambient + diffuse + specular + rimColor, 1.0);
}