#version 410 core

out vec4 outputColor;

uniform vec3 lineColor;

void main() 
{
	outputColor = vec4(lineColor, 1.0);
}