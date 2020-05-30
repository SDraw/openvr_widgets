uniform sampler2D gTexture0;

void main()
{
    vec4 l_color = texture2D(gTexture0,gl_TexCoord[0].xy);
    l_color.rb = l_color.br;
    l_color.a = 1.0;
    gl_FragColor = l_color;
}
