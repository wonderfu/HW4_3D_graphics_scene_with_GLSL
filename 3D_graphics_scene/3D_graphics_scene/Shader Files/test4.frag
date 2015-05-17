
varying vec4 Position;

uniform float vtime;
uniform vec3 lcolor;

void main (void)
{
	vec3 dis = fract(vec3(Position));
	vec4 color;
	float zperc = dis.z - fract(vtime);
	if ( zperc < 0.0 )
		zperc = zperc + 1.0;

	zperc = zperc * 2.0;
	if ( zperc > 1.0 )
		zperc = 2.0 - zperc;

	color = mix(vec4(lcolor, 1.0) , vec4(lcolor, 0.0), zperc);

	gl_FragColor = color;
}
