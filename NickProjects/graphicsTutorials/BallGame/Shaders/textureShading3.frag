#version 130
//The fragment shader operates on each pixel in a given polygon

in vec2 fragmentPosition;
in vec4 fragmentColor;
in vec2 fragmentUV;

uniform vec3 uColor;

float angle = atan(-fragmentPosition.y , fragmentPosition.x) * 0.1;
float len = length(fragmentPosition - vec2(0.0, 0.0));

//This is the 3 component float vector that gets outputted to the screen
//for each pixel.
out vec4 color;

uniform sampler2D mySampler;

void main() {
	
	vec4 textureColor = texture(mySampler, fragmentUV);
    
    color = vec4(uColor,1.0) * vec4(fragmentColor.r * (tan(cos(angle * 40.0)) / 2.0), 
				 fragmentColor.g * (tan(sin(len * 40.0)) / 2.0), 
				 fragmentColor.b * (tan(angle * 80.0) / 2.0), fragmentColor.a) * textureColor;
}