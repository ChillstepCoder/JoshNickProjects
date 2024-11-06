// barrier.frag

#version 130

in vec2 fragmentPosition;
in vec2 fragmentUV;
in float fragDistanceAlong;

// Declare all uniforms without default values
uniform vec3 primaryColor;
uniform vec3 secondaryColor;
uniform float patternScale;

out vec4 FragColor;

void main() {
    // Force use of patternScale to prevent optimization
    float scaledDistance = fragDistanceAlong / max(patternScale, 1.0);
    float segmentIndex = floor(scaledDistance);
    float checker = mod(segmentIndex, 2.0);
    
    vec3 finalColor = mix(primaryColor, secondaryColor, checker);
    FragColor = vec4(finalColor, 1.0);
}