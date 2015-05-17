
varying vec4 Position;

void main(void)
{
	Position = gl_Vertex;
	gl_Position = gl_ModelViewProjectionMatrix * Position;
}

