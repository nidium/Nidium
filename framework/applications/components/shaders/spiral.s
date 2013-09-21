/* ------------------------------------------------------------------------- */
/* https://www.shadertoy.com/user/donfabio                                   */
/* ------------------------------------------------------------------------- */

#ifdef GL_ES
precision highp float;
#endif

uniform int itime;
uniform sampler2D tex0;

vec2 iResolution = vec2(1004, 680);
float iGlobalTime = float(itime)/100.;

/* ------------------------------------------------------------------------- */

void main(void) {
	float aspectRatio = iResolution.x / iResolution.y;
	vec2 p = 2.0 * gl_FragCoord.xy / iResolution.y - vec2(aspectRatio, 1.0);
	vec2 uv = 0.4 * p;
	float distSqr = dot(uv, uv);
	float vignette = 1.0 - distSqr;
	float angle = atan(p.y, p.x);
	float shear = sqrt(distSqr);
	float blur = 0.5;
	float stripes = smoothstep(-blur, blur, cos(8.0 * angle + 12.0 * iGlobalTime - 12.0 * shear));
	const vec3 lightblue = vec3(0.5, 0.7, 0.9);
	const vec3 blue = vec3(0.4, 0.6, 0.8);
	gl_FragColor = vec4(vignette * mix(lightblue, blue, stripes), 1.0)*0.25 + texture2D(tex0, gl_TexCoord[0].xy);
}