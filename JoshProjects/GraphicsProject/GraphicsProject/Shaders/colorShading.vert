#version 130

in vec2 vertexPosition;
in vec4 vertexColor;

out vec2 fragmentPosition;
out vec4 fragmentColor;

void main() {
	//set the x,y position on the screen
	gl_Position.xy = vertexPosition;
	//the z position is zero
	gl_Position.z = 0.0;
	
	//indicates that the coordinates are normalized
	gl_Position.w = 1.0;
	
	fragmentPosition = vertexPosition;
	
	fragmentColor = vertexColor;
}