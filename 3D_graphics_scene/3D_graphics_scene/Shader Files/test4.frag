
varying vec4 Position;

uniform float cy_r;
uniform float dig_r;
uniform vec3 color1;
uniform vec3 color2;
uniform vec3 point[20];

void main (void)
{
	vec4 color;

	color = vec4(-1.0);
	for(int i = 0; i < 20; ++i)
	{
		float dis = distance(vec3(Position), point[i] ) - dig_r + cy_r;
		if( dis < 0.01 )
		{
			color = vec4(mix(color1, color2, dis*100.0), mix(1.0, 0.0, dis*150.0));
		}
	}
	if( color.a < 0.0 )
		discard;

	gl_FragColor = color;
}
