uniform sampler2D texture;

void main(void) {
	gl_FragColor = texture2D(texture, gl_TexCoord[0].xy);
}
