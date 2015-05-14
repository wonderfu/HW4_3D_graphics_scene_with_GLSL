
varying vec4 Position;

void main (void)
{
	vec3 dis = fract(vec3(Position))*vec3(10);

	if( dis.y > 1.0 && dis.y < 7.0)
		discard;

	gl_FragColor = vec4(0.0,0.0,1.0,1.0);
}
