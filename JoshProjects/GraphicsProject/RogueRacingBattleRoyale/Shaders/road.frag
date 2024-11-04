// road.frag

#version 130

in vec2 fragmentPosition;
in vec2 fragmentUV;
in float fragDistanceAlong;

uniform mat4 P;

out vec4 FragColor;

// Reuse the same noise functions from grass.frag
vec2 hash2(vec2 p) {
    p = vec2(dot(p,vec2(127.1,311.7)), dot(p,vec2(269.5,183.3)));
    return -1.0 + 2.0 * fract(sin(p)*43758.5453123);
}

float noise2D(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    vec2 u = f * f * (3.0 - 2.0 * f);
    
    float n00 = dot(hash2(i), f);
    float n10 = dot(hash2(i + vec2(1.0, 0.0)), f - vec2(1.0, 0.0));
    float n01 = dot(hash2(i + vec2(0.0, 1.0)), f - vec2(0.0, 1.0));
    float n11 = dot(hash2(i + vec2(1.0, 1.0)), f - vec2(1.0, 1.0));
    
    return mix(mix(n00, n10, u.x), mix(n01, n11, u.x), u.y);
}

void main() {
    // Base road color (grey)
    vec3 roadColor = vec3(0.4, 0.4, 0.4);
    
    // Add subtle noise for road texture
    float roadNoise = noise2D(fragmentPosition * 0.1) * 0.05;
    roadColor += vec3(roadNoise);
    
    // Enhanced edge darkening
    float edgeFactor = smoothstep(0.0, 0.4, abs(fragmentUV.x - 0.5));
    roadColor *= mix(1.0, 0.6, edgeFactor);
    
    // Center line with world-space based dashing
    float centerLine = step(0.47, fragmentUV.x) * step(fragmentUV.x, 0.53);
    float dashPattern = mod(fragDistanceAlong * 0.04, 1.0);
    centerLine *= step(0.5, dashPattern);
    
    // Add slight noise to the center line
    float lineNoise = noise2D(fragmentPosition * 0.2) * 0.02;
    
    vec3 finalColor = mix(roadColor, vec3(1.0 + lineNoise), centerLine);
    FragColor = vec4(finalColor, 1.0);
}