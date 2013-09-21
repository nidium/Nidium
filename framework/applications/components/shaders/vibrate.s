#ifdef GL_ES
precision highp float;
#endif

uniform int itime;
uniform vec2 n_Resolution;
uniform sampler2D tex0;

vec2 iResolution = n_Resolution;
vec2 iMouse = vec2(iResolution)*0.5;
float iGlobalTime = float(itime)/1000.;

/* ------------------------------------------------------------------------- */
vec2 texCoord = gl_TexCoord[0].st;
vec2 pos = vec2(0.0, 0.0);

void main(){
	pos.x = sin(14500.0 * iGlobalTime)*0.002;
	pos.y = cos(14500.0 * iGlobalTime)*0.002;

	vec3 p = vec3(gl_FragCoord);

	vec4 sample = texture2D(tex0, texCoord + pos);

	gl_FragColor = vec4(sample);
}