uniform sampler2D Texture;
uniform float NativeCanvasOpacity;

void main(void)
{
    gl_FragColor = texture2D(Texture, gl_TexCoord[0].xy)*NativeCanvasOpacity;
}
