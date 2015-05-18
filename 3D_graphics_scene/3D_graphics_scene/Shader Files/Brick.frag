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
uniform vec3  BrickCircle; 
uniform float CIRBig;

varying vec2  MCposition;
varying float LightIntensity;
varying vec3  CIRposition;



float rand(vec2 co)
{
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main(void)
{
    vec3  color;
    vec2  position, useBrick;

	if ( distance(BrickCircle,CIRposition) > CIRBig)
	{
		discard;
	}

    
    position = MCposition / BrickSize;

    if (fract(position.y * 0.5) > 0.5)
        position.x += 0.5;

    position = fract(position);

    useBrick = step(position, BrickPct);

    color  = mix(MortarColor, BrickColor, useBrick.x * useBrick.y);
    color *= LightIntensity;
    gl_FragColor = vec4 (color, 1.0);


}
