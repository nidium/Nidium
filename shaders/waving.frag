uniform sampler2D Texture;
uniform float NativeCanvasOpacity;
uniform float time;

void main(void)
{
    vec4 color = texture2D(
            Texture,
            vec2(gl_TexCoord[0].x+cos(gl_TexCoord[0].y+time),
            gl_TexCoord[0].y));

    gl_FragColor = color*NativeCanvasOpacity;

}
