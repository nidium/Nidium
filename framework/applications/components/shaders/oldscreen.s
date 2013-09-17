#ifdef GL_ES
precision mediump float;
#endif

uniform vec2 n_Resolution;
uniform vec2 n_Position;
uniform float n_Opacity;

uniform sampler2D text;

vec2 resolution = n_Resolution;
vec2 FragCoord = vec2(gl_FragCoord.x-n_Position.x, gl_FragCoord.y-n_Position.y);
vec2 screen = vec2(320, 280);


#define TEX2D(c) pow(texture2D(text, (c)), vec4(inputGamma))
#define FIX(c)   max(abs(c), 1e-6);
#define PI 3.141592653589

#define phase 0.0
#define inputGamma 2.2
#define outputGamma 2.0
#define distortion 0.2

vec2 radialDistortion(vec2 coord) {
	coord *= screen / screen;
	vec2 cc = coord - 0.5;
	float dist = dot(cc, cc) * distortion;
	return (coord + cc * (1.0 + dist) * dist) * screen / screen;
}
		
vec4 scanlineWeights(float distance, vec4 color) {
	vec4 wid = 2.0 + 2.0 * pow(color, vec4(4.0));
	vec4 weights = vec4(distance * 3.333333);
	return 0.51 * exp(-pow(weights * sqrt(2.0 / wid), wid)) / (0.18 + 0.06 * wid);
}
		
void main() {
	vec2 p = FragCoord.xy / resolution.xy;
	vec2 one = 1.0 / screen;

	vec2 xy = radialDistortion(p.xy);

	vec2 uv_ratio = fract(xy * screen) - vec2(0.5);
 
	xy = (floor(xy * screen) + vec2(0.5)) / screen;

	vec4 coeffs = PI * vec4(1.0 + uv_ratio.x, uv_ratio.x, 1.0 - uv_ratio.x, 2.0 - uv_ratio.x);                				

	coeffs = FIX(coeffs);
	coeffs = 2.0 * sin(coeffs) * sin(coeffs / 2.0) / (coeffs * coeffs);

	coeffs /= dot(coeffs, vec4(1.0));
				
	vec4 col  = clamp(coeffs.x * TEX2D(xy + vec2(-one.x, 0.0))   + coeffs.y * TEX2D(xy)                    + coeffs.z * TEX2D(xy + vec2(one.x, 0.0)) + coeffs.w * TEX2D(xy + vec2(2.0 * one.x, 0.0)),   0.0, 1.0);
	vec4 col2 = clamp(coeffs.x * TEX2D(xy + vec2(-one.x, one.y)) + coeffs.y * TEX2D(xy + vec2(0.0, one.y)) + coeffs.z * TEX2D(xy + one)              + coeffs.w * TEX2D(xy + vec2(2.0 * one.x, one.y)), 0.0, 1.0);

	vec4 weights  = scanlineWeights(abs(uv_ratio.y) , col);
	vec4 weights2 = scanlineWeights(1.0 - uv_ratio.y, col2);
	vec3 mul_res  = (col * weights + col2 * weights2).xyz;

	float mod_factor = p.x * vec2(1280, 960).x * screen.x / screen.x;

	vec3 dotMaskWeights = mix(
			vec3(1.05, 0.75, 1.05),
			vec3(0.75, 1.05, 0.75),
			floor(mod(mod_factor, 2.0))
		);
		
	mul_res *= dotMaskWeights;

	mul_res = pow(mul_res, vec3(1.0 / (2.0 * inputGamma - outputGamma)));
	
	gl_FragColor = vec4(mul_res, 0.9)*n_Opacity;
}