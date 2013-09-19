/* ------------------------+------------- */
/* Fast Dirty Blur 1.0     | Falcon Build */
/* ------------------------+------------- */
/* (c) 2013 nidium.com - Vincent Fontaine */
/* -------------------------------------- */

uniform int data;
uniform int param;
uniform sampler2D texture;

vec2 texCoord = gl_TexCoord[0].st;
float f = float(data)/10.;
float h = 1. - float(param)/500.;
vec2 amount = vec2(f, f);

void main(){
	vec4 c = vec4(0.0);
	float total = 0.0;

	for (float t=-5.0; t<=5.0; t++) {
		float p = (t - 0.5) / 500.0;
		float weight = 1.0 - abs(p);

		vec4 sample = texture2D(texture, texCoord + amount*p);

		sample.rgb *= sample.a;

		c += sample * weight;
		total += weight;
	}

	gl_FragColor = c / total;
	gl_FragColor.rgb /= h;
}
