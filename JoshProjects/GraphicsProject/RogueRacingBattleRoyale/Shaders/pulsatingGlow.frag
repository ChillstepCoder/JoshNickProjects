// pulsatingGlow.frag

#version 130

in vec2 fragmentPosition;
in vec4 fragmentColor;
in vec2 fragmentUV;
in float distanceFromCenter;

out vec4 color;

uniform sampler2D mySampler;
uniform float time;

void main() {
    vec4 textureColor = texture(mySampler, fragmentUV);
    
    // Create a pulsating effect
    float pulseIntensity = (sin(time * 3.0) + 1.0) * 0.5;
    
    // Create a color gradient based on distance from center
    vec3 gradientColor = mix(vec3(1.0, 0.0, 0.0), vec3(0.0, 0.0, 1.0), distanceFromCenter);
    
    // Combine pulsating effect with gradient color
    vec3 glowColor = mix(gradientColor, vec3(1.0), pulseIntensity * 0.5);
    
    // Combine with original color and texture
    color = vec4(glowColor, 1.0) * fragmentColor * textureColor;
    
    // Add extra glow at the edges
    float edgeGlow = 1.0 - smoothstep(0.5, 1.0, length(fragmentUV - vec2(0.5)));
    color.rgb += edgeGlow * pulseIntensity * 0.5;
}