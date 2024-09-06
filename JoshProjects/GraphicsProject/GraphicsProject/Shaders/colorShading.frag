#version 130
in vec2 fragmentPosition;
in vec4 fragmentColor;
in vec2 fragmentUV;
out vec4 color;
uniform float time;
uniform sampler2D mySampler;

void main() {
    vec4 textureColor = texture(mySampler, fragmentUV);
    
    // Center coordinates
    vec2 center = vec2(0.0, 0.0);
    vec2 toCenter = fragmentPosition - center;
    
    // Calculate distance from center
    float distance = length(toCenter);
    
    // Calculate angle
    float angle = atan(toCenter.y, toCenter.x);
    
    // Create high-density expanding spiral effect
    float spiralDensity = 100.0; // Dramatically increased for many more rotations
    float expansionRate = 0.2; // Adjusted for quicker expansion
    float spiralSpeed = 0.9;
    
    // Calculate spiral with expanding gaps and high density
    float spiral = fract((angle + spiralDensity * pow(distance, expansionRate)) / (2.0 * 3.14159) - time * spiralSpeed);
    
    // Create smooth spiral lines
    float lineWidth = 0.1; // Adjusted for balance between green and dark lines
    float smoothness = 0.4; // Reduced for sharper edges
    spiral = smoothstep(0.5 - lineWidth - smoothness, 0.5 - lineWidth, spiral) - 
             smoothstep(0.5 + lineWidth, 0.5 + lineWidth + smoothness, spiral);
    
    // Create color scheme similar to the image
    vec3 spiralColor = vec3(0.0, 0.8, 0.0); // Bright green
    vec3 bgColor = vec3(0.1, 0.05, 0.0); // Dark background
    
    // Mix colors based on the spiral
    vec3 finalColor = mix(bgColor, spiralColor, spiral);
    
    // Apply distance-based darkening for depth effect
    float depthFactor = 1.0 / (1.0 + distance * 0.3);
    finalColor *= depthFactor;
    
    // Apply fragment color and create final output
    color = vec4(finalColor * fragmentColor.rgb, fragmentColor.a) * textureColor;
}