/* ------------------------+------------- */
/* Dirty Lines 1.0         | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

#ifdef GL_ES
precision highp float;
#endif

uniform int itime;
uniform vec2 n_Resolution;
uniform sampler2D iChannel0;

vec2 iResolution = n_Resolution;
vec2 iMouse = iResolution*0.5;
float iGlobalTime = float(itime)/50.;
float iChannelTime = 5.0;

/* ------------------------------------------------------------------------- */
void main(void)
{	
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	float y = 
		0.7*sin((uv.y + iGlobalTime) * 4.0) * 0.038 +
		0.3*sin((uv.y + iGlobalTime) * 8.0) * 0.010 +
		0.05*sin((uv.y + iGlobalTime) * 40.0) * 0.05;

	float x = 
		0.5*sin((uv.y + iGlobalTime) * 5.0) * 0.1 +
		0.2*sin((uv.x + iGlobalTime) * 10.0) * 0.05 +
		0.2*sin((uv.x + iGlobalTime) * 30.0) * 0.02;

	gl_FragColor = texture2D(iChannel0, 0.79*(uv + vec2(y+0.11, x+0.11)));
}