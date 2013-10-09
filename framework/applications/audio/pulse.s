#ifdef GL_ES
precision highp float;
#endif

uniform sampler2D tex0;
uniform int itime;
uniform int ipulse;

uniform vec2 n_Resolution;

float time = float(itime)/50.;
float pulse = 0.0001+float(ipulse)/2.;

const float PI = 3.1415926535897932;

//speed
const float speed = 0.1;
const float speed_x = 0.3;
const float speed_y = 0.3;

// geometry
const float intensity = 0.1;
const int steps = 6;
const float freq = 2.0;
const int angle = 7; // better when a prime

// reflection and emboss
const float delta = 10.;
const float intence = 100.;
const float emboss = 0.3;

float col(vec2 coord) {
	float delta_theta = 2.0 * PI / float(angle);
	float col = 0.0;
	float theta = 0.0;

	for (int i = 0; i < steps; i++) {
		vec2 adjc = coord;
		theta = delta_theta*float(i);
		adjc.x += cos(theta)*time*speed + time * speed_x;
		adjc.y -= sin(theta)*time*speed - time * speed_y;
		col = col + cos( (adjc.x*cos(theta) - adjc.y*sin(theta))*freq)*intensity*pulse;
	}

	return cos(col);
}

void main(void) {
	vec2 p = (gl_FragCoord.xy) / n_Resolution.xy, c1 = p, c2 = p;
	float cc1 = col(c1);

	c2.x += n_Resolution.x/delta;
	float dx = emboss*(cc1-col(c2))/delta;

	c2.x = p.x;
	c2.y += n_Resolution.y/delta;
	float dy = emboss*(cc1-col(c2))/delta;

	c1.x += dx;
	c1.y = -(c1.y+dy);

	float alpha = 1.+dot(dx, dy)*intence;
	gl_FragColor = texture2D(tex0, 0.7*(c1 - vec2(-0.2, -1.3)))*alpha;
}
