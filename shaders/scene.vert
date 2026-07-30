#version 120

varying vec2 v_uv;

void main() {
    v_uv = gl_Vertex.xy * 0.5 + 0.5;
    gl_Position = gl_Vertex;
}
