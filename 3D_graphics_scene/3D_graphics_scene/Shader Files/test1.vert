
varying vec3 Position;

void main(void)
{
	Position = vec3(gl_Vertex);
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
