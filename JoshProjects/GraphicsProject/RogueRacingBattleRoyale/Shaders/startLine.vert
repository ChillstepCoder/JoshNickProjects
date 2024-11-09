// startLine.vert
#version 130
in vec2 vertexPosition;
in vec2 vertexUV;
in float distanceAlong;
in float depth;

uniform mat4 P;

void main() {
    vec4 pos = P * vec4(vertexPosition, 0.0, 1.0);
    gl_Position = pos;
}

