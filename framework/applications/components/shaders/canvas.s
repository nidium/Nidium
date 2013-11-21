#ifdef GL_ES
precision mediump float;
#endif

uniform int itime;
uniform vec2 n_Resolution;
uniform sampler2D iChannel0;

vec2 iResolution = n_Resolution;
vec2 iMouse = vec2(iResolution)*0.5;
float iGlobalTime = float(itime)/100.;

/* ------------------------------------------------------------------------- */
#define DOTSIZE 9.08
#define D2R(d) radians(d)
#define MIN_S 2.5
#define MAX_S 19.0
#define SPEED 0.0

#define SST 0.788
#define SSQ 0.288

vec2 ORIGIN = 0.5*iResolution.xy;
float S = MIN_S+(MAX_S-MIN_S)*(0.5-0.5*cos(SPEED*iGlobalTime));
float R = SPEED*0.333*iGlobalTime;

vec4 rgb2cmyki(in vec4 c) {
	float k = max(max(c.r,c.g),c.b);
	return min(vec4(c.rgb/k,k),1.0);
}

vec4 cmyki2rgb(in vec4 c) {
	return vec4(c.rgb*c.a,1.0);
}

vec2 px2uv(in vec2 px) {
	return vec2(px/iResolution.xy);
}

vec2 grid(in vec2 px) {
	return px-mod(px,S);
	//return floor(px/S)*S; // alternate
}

vec4 ss(in vec4 v) {
	return smoothstep(SST-SSQ,SST+SSQ,v);
}

vec4 halftone(in vec2 fc,in mat2 m)
{
	vec2 smp = (grid(m*fc)+0.5*S)*m;
	float s = min(length(fc-smp)/(DOTSIZE*0.5*S),1.0);
	vec4 c = rgb2cmyki(texture2D(iChannel0,px2uv(smp+ORIGIN)));
	return c+s;
}

mat2 rotm(in float r)
{
	float cr = cos(r);
	float sr = sin(r);
	return mat2(
		cr,-sr,
		sr,cr
	);
}

void main()
{
	
	vec2 fc = (gl_FragCoord.xy-ORIGIN) * 0.9;
	
	mat2 mc = rotm(R+D2R(15.0));
	mat2 mm = rotm(R+D2R(75.0));
	mat2 my = rotm(R);
	mat2 mk = rotm(R+D2R(45.0));
	
	float k = halftone(fc,mk).a;
	vec4 c = cmyki2rgb(ss(vec4(
		halftone(fc,mc).r,
		halftone(fc,mm).g,
		halftone(fc,my).b,
		halftone(fc,mk).a
	)));
	gl_FragColor = c;
}