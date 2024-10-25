//textRendering.vert

#version 130
in vec2 vertexPosition;
in vec4 vertexColor;
in vec2 vertexUV;
out vec2 fragmentUV;
out vec4 fragmentColor;
uniform mat4 P;

void main() {
    gl_Position = P * vec4(vertexPosition, 0.0, 1.0);
    fragmentColor = vertexColor;
    fragmentUV = vertexUV;
}