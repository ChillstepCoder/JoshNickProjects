// offroad.frag
#version 130

in vec2 fragmentPosition;
in vec2 fragmentUV;
in float fragDistanceAlong;

uniform vec3 offroadColor = vec3(0.45, 0.32, 0.15);
uniform float noiseIntensity = 0.1;
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

float fbm(vec2 p) {
    float sum = 0.0;
    float amp = 1.0;
    float freq = 1.0;
    for(int i = 0; i < 3; i++) {
        sum += amp * noise2D(p * freq);
        freq *= 2.0;
        amp *= 0.5;
    }
    return sum;
}

void main() {
    // Use world position for consistent noise
    vec2 worldPos = fragmentPosition;
    
    // Generate two layers of noise for more detail
    float baseNoise = fbm(worldPos * 0.02);
    float detailNoise = fbm(worldPos * 0.08) * 0.5;
    
    float combinedNoise = (baseNoise + detailNoise) * noiseIntensity;
    
    // Base dirt/brown color with noise variation
    vec3 finalColor = offroadColor + vec3(combinedNoise);
    
    // Fade to edges
    float edgeFactor = smoothstep(0.0, 0.3, fragmentUV.x) * smoothstep(1.0, 0.7, fragmentUV.x);
    finalColor *= mix(0.7, 1.0, edgeFactor);
    
    FragColor = vec4(finalColor, 1.0);
}