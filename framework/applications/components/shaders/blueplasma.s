#ifdef GL_ES
precision mediump float;
#endif

uniform int itime;
uniform vec2 mouse;
vec2 r = vec2(640, 480);
float time = float(itime)/150.;

#define PI 3.14159
#define TWO_PI (PI*2.0)
#define N 8.0

void main(void) {
	vec2 v = (gl_FragCoord.xy - r/100.0) / min(r.y, r.x) * 5.0;
	v.x = v.x-10.0;
	v.y = v.y-20.0;
	float col = 0.0;

	for (float i = 0.0; i < N; i++) {
		float a = i * (TWO_PI/N) * 1.95;
		col += cos(TWO_PI*(v.y * cos(a) - v.x * sin(a) + tan(time*0.004)*100.0 ));
	}

	col /= 3.0;

	gl_FragColor = vec4(col*0.5, col*0.5, col*4.0, 1.0);
}