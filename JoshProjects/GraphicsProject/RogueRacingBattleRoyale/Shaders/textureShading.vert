// textureShading.vert
#version 130
in vec2 vertexPosition;
in vec4 vertexColor;
in vec2 vertexUV;

out vec2 fragmentPosition;
out vec4 fragmentColor;
out vec2 fragmentUV;

uniform mat4 P;

void main() {
    gl_Position = P * vec4(vertexPosition, 0.0, 1.0);
    fragmentPosition = vertexPosition;
    fragmentColor = vertexColor;
    fragmentUV = vertexUV;
}
