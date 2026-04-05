#version 430 core
out vec4 FragColor;
uniform vec3 color;

void main() {
    vec2 c = gl_PointCoord * 2.0 - 1.0;
    if (dot(c, c) > 1.0) discard;
    FragColor = vec4(color, 1.0);
}