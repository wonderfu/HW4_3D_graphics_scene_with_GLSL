
varying vec4 Position;

uniform vec3 vtime;
uniform float size;

void main(void)
{
	Position = gl_Vertex;
	
	Position.xyz = sin(Position.xyz + vtime)*size;

	gl_Position = gl_ModelViewProjectionMatrix * Position;
}

