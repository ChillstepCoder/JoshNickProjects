// grass.vert

#version 130

in vec2 vertexPosition;
in vec2 vertexUV;

uniform mat4 P;

out vec2 worldPosition;
out vec2 fragmentUV;

void main() {
    worldPosition = vertexPosition;
    fragmentUV = vertexUV;
    gl_Position = P * vec4(vertexPosition, 0.0, 1.0);
}