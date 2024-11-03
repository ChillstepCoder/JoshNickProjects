// road.frag

#version 130

in vec2 fragmentUV;
in float fragDistanceAlong;

out vec4 FragColor;

void main() {
    // Base road color (grey)
    vec3 roadColor = vec3(0.4, 0.4, 0.4);
    
    // Edge darkening: darker at edges, lighter in center
    float edgeFactor = smoothstep(0.0, 0.4, abs(fragmentUV.x - 0.5));
    roadColor *= mix(1.0, 0.7, edgeFactor); // Darken edges by 30%
    
    // Center line
    float centerLine = step(0.48, fragmentUV.x) * step(fragmentUV.x, 0.52);
    
    // Dashed line pattern
    float dashPattern = mod(fragDistanceAlong * 0.05, 1.0);
    centerLine *= step(0.5, dashPattern);
    
    // Mix in the white color for the center line, making sure it's bright
    vec3 finalColor = mix(roadColor, vec3(1.0), centerLine);

    FragColor = vec4(finalColor, 1.0);
}