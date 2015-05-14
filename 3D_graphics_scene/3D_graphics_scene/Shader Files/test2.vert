
varying vec4 Position;

uniform float xtime,ytime,ztime;
uniform float size;

void main(void)
{
	Position = gl_Vertex;

	//Position.y = Position.y + time*10.0;
	//if( Position.y > size )
		//Position.y = Position.y - size;
	
	Position.x = sin(Position.x + xtime)*size;
	Position.z = sin(Position.z + ytime)*size;
	Position.y = sin(Position.y + ztime)*size;

	gl_Position = gl_ModelViewProjectionMatrix * Position;
}

