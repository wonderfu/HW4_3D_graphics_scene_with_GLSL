
varying vec4 Position;
uniform float vtime;

uniform vec3 color1;
uniform vec3 color2;

void main (void)
{
	vec3 dis = fract(vec3(Position));
	vec3 color;
	float yperc = dis.y + fract(vtime);
	if ( yperc > 1.0 )
		yperc = yperc - 1.0;

	yperc = yperc * 2.0;
	if ( yperc > 1.0 )
		yperc = 2.0 - yperc;

	color = mix(color1, color2, yperc);

	gl_FragColor = vec4(color, 1.0);
}
