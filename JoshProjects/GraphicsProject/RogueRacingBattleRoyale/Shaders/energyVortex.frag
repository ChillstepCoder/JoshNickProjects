//energyVortex.frag

#version 130

in vec2 fragmentPosition;
in vec4 fragmentColor;
in vec2 fragmentUV;
in vec2 ballCenter;

out vec4 color;

uniform sampler2D mySampler;
uniform float time;
uniform vec2 ballVelocity;
uniform float recentCollisionIntensity;
uniform vec2 screenDimensions;

vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
    vec2 uv = fragmentUV;
    float dist = length(uv - vec2(0.5));
    
    // Calculate angle based on UV and adjust with velocity
    float angle = atan(uv.y - 0.5, uv.x - 0.5);
    angle += dot(ballVelocity, vec2(cos(angle), sin(angle))) * 0.1;
    
    // Create swirling vortex effect
    float vortex = sin(angle * 10.0 + dist * 20.0 - time * 5.0) * 0.5 + 0.5;
    vortex = pow(vortex, 3.0); // Sharpen the vortex
    
    // Calculate energy based on velocity magnitude
    float energy = length(ballVelocity) * 0.01;
    
    // Use HSV color space for more control
    float hue = mod(angle / (2.0 * 3.14159) + time * 0.1, 1.0);
    float saturation = mix(0.5, 1.0, energy);
    float value = mix(0.5, 1.0, vortex);
    
    // Add recent collision effect
    value += recentCollisionIntensity * sin(time * 20.0) * 0.2;
    
    vec3 vortexColor = hsv2rgb(vec3(hue, saturation, value));
    
    // Sample base texture
    vec4 texColor = texture(mySampler, uv);
    
    // Combine base texture with vortex effect
    vec3 finalColor = mix(texColor.rgb, vortexColor, 0.8);
    
    // Add glow based on energy
    finalColor += vec3(1.0, 0.7, 0.3) * energy * 0.5;
    
    // Apply a subtle vignette
    float vignette = smoothstep(0.7, 0.3, dist);
    finalColor *= vignette;
    
    color = vec4(finalColor, texColor.a) * fragmentColor;
}