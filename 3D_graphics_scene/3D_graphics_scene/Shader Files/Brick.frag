//
// Fragment shader for procedural bricks
//
// Authors: Dave Baldwin, Steve Koren, Randi Rost
//          based on a shader by Darwyn Peachey
//
// Copyright (c) 2002-2006 3Dlabs Inc. Ltd. 
//
// See 3Dlabs-License.txt for license information
//

uniform vec3  BrickColor, MortarColor;
uniform vec2  BrickSize;
uniform vec2  BrickPct;

varying vec2  MCposition;
varying float LightIntensity;

void main(void)
{
    vec3  color;
    vec2  position, useBrick;
    
    position = MCposition / BrickSize;

    if (fract(position.y * 0.5) > 0.5)
        position.x += 0.5;

    position = fract(position);

    useBrick = step(position, BrickPct);

    color  = mix(MortarColor, BrickColor, useBrick.x * useBrick.y);
    color *= LightIntensity;
    gl_FragColor = vec4 (color, 1.0);


    if( fmod((position.x + position.y), 17) <= 2 )
	{
	    gl_FragColor = vec4 (1.0, 0.5, 0, 1.0);
	}
    else if( fmod((position.x + position.y), 13) <= 2 )
	{
	    gl_FragColor = vec4 (0.9, 0.1, 0.1, 1.0);
	} 
    else if( fmod((position.x + position.y), 9) <= 2 )
	{
	    gl_FragColor = vec4 (1.0, 0.8, 0.7, 1.0);
	}
    else if( fmod((position.x + position.y), 8) <= 5 )
	{
	    gl_FragColor = vec4 (1.0, 0.1, 0.1, 1.0);	
	}
    else if( fmod((position.x + position.y), 3) <= 2 )
	{
		gl_FragColor = vec4 (1.0, 0.9, 0.9, 1.0);
	}
    else if( fmod((position.x + position.y), 7) <= 5 )
	{
		gl_FragColor = vec4 (1.0, 0.7, 0, 1.0);	
	}
    else if( fmod((position.x + position.y), 5) <= 3 )
	{
		gl_FragColor = vec4 (1.0, 0.8, 0.8, 1.0);
	}

}