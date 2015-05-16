
varying vec4 Position;

uniform float vtime;
uniform float size;

void main(void)
{
	Position = gl_Vertex;

	//Position.y = Position.y + vtime.y;
	
	gl_Position = gl_ModelViewProjectionMatrix * Position;
}

