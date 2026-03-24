#version 410 core

layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

uniform float lineDepthBiasNdc;

void emitBiasedVertex(vec4 clipPos)
{
    vec4 p = clipPos;
    p.z -= lineDepthBiasNdc * p.w;
    gl_Position = p;
    EmitVertex();   

}

void emitEdge(int a, int b)
{
    emitBiasedVertex(gl_in[a].gl_Position);
    emitBiasedVertex(gl_in[b].gl_Position);
    EndPrimitive();
}

void main()
{
    emitEdge(0, 1);
    emitEdge(1, 2);
    emitEdge(2, 0);
}