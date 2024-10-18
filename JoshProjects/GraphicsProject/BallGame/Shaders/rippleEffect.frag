//rippleEffect.frag

#version 130

in vec2 fragmentPosition;
in vec4 fragmentColor;
in vec2 fragmentUV;
in vec2 centerCoord;

out vec4 color;

uniform sampler2D mySampler;
uniform float time;
uniform vec2 screenDimensions;

void main() {
    vec2 uv = fragmentUV;
    
    // Create more dramatic ripple effect
    float distance = length(centerCoord);
    float ripple = sin(distance * 30.0 - time * 8.0) * 0.5 + 0.5;
    ripple = pow(ripple, 3.0); // Sharpen the ripple effect
    
    // Amplify the distortion
    vec2 distortedUV = uv + (centerCoord * ripple * 0.2);
    
    // Sample texture with distorted UVs
    vec4 texColor = texture(mySampler, distortedUV);
    
    // Create a more vibrant color gradient based on position and time
    vec3 gradientColor = vec3(
        sin(centerCoord.x * 5.0 + time * 1.5) * 0.5 + 0.5,
        sin(centerCoord.y * 5.0 + time * 1.7) * 0.5 + 0.5,
        sin(distance * 8.0 - time * 3.0) * 0.5 + 0.5
    );
    gradientColor = pow(gradientColor, vec3(1.5)); // Increase color contrast
    
    // Combine ripple effect, texture color, and gradient with higher contrast
    vec3 finalColor = mix(texColor.rgb, gradientColor, ripple * 0.7);
    
    // Add a more pronounced glow effect
    float glow = smoothstep(0.3, 0.7, ripple);
    finalColor += glow * vec3(0.7, 0.9, 1.0) * 0.8;
    
    // Apply a vignette effect for more drama
    float vignette = 1.0 - smoothstep(0.5, 1.5, distance);
    finalColor *= vignette;
    
    // Increase overall contrast
    finalColor = pow(finalColor, vec3(1.2));
    
    color = vec4(finalColor, texColor.a) * fragmentColor;
}