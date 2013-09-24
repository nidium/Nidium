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

void main(void) {

	float scale = 1.0 / n_Resolution.x;

	vec2 delta1 = vec2(-1.0*scale, -1.0*scale);
	vec2 delta2 = vec2( 0.0*scale, -1.0*scale);
	vec2 delta3 = vec2( 1.0*scale, -1.0*scale);

	vec2 delta4 = vec2(-1.0*scale,  0.0*scale);
	vec2 delta5 = vec2( 1.0*scale,  0.0*scale);

	vec2 delta6 = vec2(-1.0*scale,  1.0*scale);
	vec2 delta7 = vec2( 0.0*scale,  1.0*scale);
	vec2 delta8 = vec2( 1.0*scale,  1.0*scale);

	vec4 px = texture2D(iChannel0, gl_TexCoord[0].xy);
	vec4 c1 = texture2D(iChannel0, gl_TexCoord[0].xy + delta1);
	vec4 c2 = texture2D(iChannel0, gl_TexCoord[0].xy + delta2);
	vec4 c3 = texture2D(iChannel0, gl_TexCoord[0].xy + delta3);
	vec4 c4 = texture2D(iChannel0, gl_TexCoord[0].xy + delta4);
	vec4 c5 = texture2D(iChannel0, gl_TexCoord[0].xy + delta5);
	vec4 c6 = texture2D(iChannel0, gl_TexCoord[0].xy + delta6);
	vec4 c7 = texture2D(iChannel0, gl_TexCoord[0].xy + delta7);
	vec4 c8 = texture2D(iChannel0, gl_TexCoord[0].xy + delta8);

	if (mod(gl_FragCoord.y, 2.0) <= 1.0) {
		gl_FragColor =	0.35*(
							0.1*c1 + 0.5*c2 + 0.5*c3 +
							0.1*c4 + 0.0*px + 0.1*c5 +
							0.5*c6 + 0.5*c7 + 0.1*c8
						);
	} else {
		gl_FragColor =	px*0.2;
	}


}