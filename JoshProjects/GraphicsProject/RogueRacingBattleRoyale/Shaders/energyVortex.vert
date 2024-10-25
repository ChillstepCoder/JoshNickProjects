// energyVortex.vert

#version 130

in vec2 vertexPosition;
in vec4 vertexColor;
in vec2 vertexUV;

out vec2 fragmentPosition;
out vec4 fragmentColor;
out vec2 fragmentUV;
out vec2 ballCenter;

uniform mat4 P;
uniform vec2 screenDimensions;

void main() {
    gl_Position.xy = (P * vec4(vertexPosition, 0.0, 1.0)).xy;
    gl_Position.z = 0.0;
    gl_Position.w = 1.0;
    
    fragmentPosition = vertexPosition;
    fragmentColor = vertexColor;
    fragmentUV = vec2(vertexUV.x, 1.0 - vertexUV.y);
    
    // Calculate ball center in normalized device coordinates
    ballCenter = (vertexPosition / screenDimensions) * 2.0 - 1.0;
}