// grass.frag

#version 130

in vec2 worldPosition;
in vec2 fragmentUV;

uniform vec3 grassColor;
uniform float noiseScale;    // Add back noiseScale uniform
uniform float noiseIntensity; // Add back noiseIntensity uniform

out vec4 FragColor;

vec2 hash2(vec2 p) {
    p = vec2(dot(p,vec2(127.1,311.7)), dot(p,vec2(269.5,183.3)));
    return -1.0 + 2.0 * fract(sin(p)*43758.5453123);
}

float noise(vec2 p) {
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
    float total = 0.0;
    float amplitude = 0.5;
    float frequency = 1.0;
    
    // Use noiseScale uniform for base scaling
    p *= 0.001 * noiseScale;
    
    for(int i = 0; i < 4; i++) {
        total += amplitude * noise(p * frequency);
        frequency *= 2.0;
        amplitude *= 0.5;
    }
    return total;
}

void main() {
    // Use world-space coordinates for noise
    float baseNoise = fbm(worldPosition);
    float detailNoise = fbm(worldPosition * 2.0) * 0.5;
    float microDetail = fbm(worldPosition * 4.0) * 0.25;
    
    // Use noiseIntensity uniform for controlling strength
    float finalNoise = (baseNoise + detailNoise + microDetail) * noiseIntensity;
    
    vec3 finalColor = grassColor + vec3(finalNoise);
    finalColor = clamp(finalColor, grassColor * 0.7, grassColor * 1.3);
    
    FragColor = vec4(finalColor, 1.0);
}