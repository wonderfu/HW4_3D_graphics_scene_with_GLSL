
varying vec3 Position;

void main (void)
{
	vec3 dis = fract(Position*vec3(10));

	if( dis.x > 0.4 && dis.x < 0.6)
		discard;
	if( dis.y > 0.4 && dis.y < 0.6)
		discard;
	if( dis.z > 0.4 && dis.z < 0.6)
		discard;

	gl_FragColor = vec4 ( Position+vec3(0.4) , 1.0);
}
