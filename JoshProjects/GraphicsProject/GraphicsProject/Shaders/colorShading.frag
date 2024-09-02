#version 130

in vec2 fragmentPosition;
in vec4 fragmentColor;
out vec4 color;
uniform float time;

void main() {
    )
    vec2 center = vec2(0.0, 0.0);
	
    vec2 toCenter = fragmentPosition - center;
    
    // Calculate distance from center
    float distance = length(toCenter);
    
    float angle = atan(toCenter.y, toCenter.x);
    
    float spiral = sin(distance * 15.0 - angle * 4.0 + time * 2.0);
    
    float pulse = sin(distance * 5.0 - time * 10.0) * 1 ;
    
    // Combine effects
    float effect = spiral * pulse;
    
    // Create colors
    vec3 color1 = vec3(sin(time) * 0.5 + 0.5, cos(time * 1.1) * 0.5 + 0.5, sin(time * 0.7) * 0.5 + 0.5);
    vec3 color2 = vec3(cos(time * 0.8) * 0.5 + 0.5, sin(time * 2.0) * 0.5 + 0.5, cos(time * 1.9) * 0.5 + 0.5);
    
    // Mix colors based on the effect
    vec3 finalColor = mix(color1, color2, effect);
    
    color = vec4(finalColor * fragmentColor.rgb, fragmentColor.a);
}