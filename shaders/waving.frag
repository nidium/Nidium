uniform sampler2D texture;
uniform float time;

void main(void) {
    vec4 color = texture2D(
        texture,
        vec2(gl_TexCoord[0].x + cos(0.01 * time), gl_TexCoord[0].y)
    );

    gl_FragColor = color;
}
