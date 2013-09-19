uniform sampler2D Texture;
uniform float n_Opacity;

void main(void)
{
    gl_FragColor = texture2D(Texture, gl_TexCoord[0].xy)*n_Opacity;
}
