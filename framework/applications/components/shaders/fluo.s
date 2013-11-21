/* ------------------------------------------------------------------------- */
/* https://www.shadertoy.com/user/donfabio                                   */
/* ------------------------------------------------------------------------- */

#ifdef GL_ES
precision mediump float;
#endif

uniform int itime;
uniform sampler2D tex0;

vec2 iResolution = vec2(1004, 680);
vec2 iMouse = vec2(iResolution)*0.5;
float iGlobalTime = float(itime)/100.;

/* ------------------------------------------------------------------------- */
float wave( vec2 uv, float time, float frequency){
	
	// moving sine wave
	float x=sin( time+frequency*4.0-uv.x*frequency);
	
	// set y in range -1.0 to 1.0, so that comparing distance to sin(x) is easy
	float y=uv.y*2.0-1.0;
	
	// set color = distance between sin(x) and y (intensity increases further away we are)
	float color=sin(x)-y;
	
	// vary the thickness of the wave over time
	color/=(sin(time)*0.1+0.15);
	
	// colour should be brightest for points nearest the wave
	color=1.0/abs(color);
	
	return color;
}

// The white diamond control 
vec4 control( vec2 uv, vec2 mouse){
	
	// calc distance from centre point of control, locking x co-ordinate of control at 0.9
	float x=abs(uv.x-0.9);
	float y=abs(uv.y-mouse.y);
	
	// Fix aspect ratio
	y*=iResolution.y/iResolution.x;
	
	// use the distance of the current position to create a white colour
	float d=1.0/(50.0*abs(x+y));
	return vec4( d, d, d, 1.0);
}

// split out the mixing of control and waves to help experiment
vec4 myMix( vec4 first, vec4 second){
	//return first+second;
	//return first*second;
	return vec4( mix( first.r, second.r, 0.5),
				 mix( first.g, second.g, 0.5),
				 mix( first.b, second.b, 0.5), 1.0);
}

void main(void)
{
	// set uv in range 0.0 to 1.0 (current coordinate)
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	
	// set mouse in range 0.0 to 1.0
	vec2 mouse = iMouse.xy/iResolution.xy;
	
	// put a margin at top and bottom of mouse y co-ordinate
	mouse.y=clamp( mouse.y, 0.1, 0.9);
	
	// calculate colour contribution from white control to this position
	vec4 controlColor = control( uv, mouse );
	
	// generate one wave per RGB component
	vec4 waves = vec4( wave(uv, iGlobalTime, 12.0*mouse.y),
					   wave(uv, iGlobalTime, 15.0*mouse.y),
					   wave(uv, iGlobalTime, 18.0*mouse.y), 1.0);
	
	
	// mix the colours of the waves and control into the final result
	gl_FragColor=myMix(waves, controlColor);
}
