// road.vert

#version 130

in vec2 vertexPosition;
in vec2 vertexUV;
in float distanceAlong;
in float depth;

uniform mat4 P;

out vec2 fragmentUV;
out float fragDistanceAlong;

void main() {
    vec4 position = P * vec4(vertexPosition, depth, 1.0);
    gl_Position = position;
    fragmentUV = vertexUV;
    fragDistanceAlong = distanceAlong;
}