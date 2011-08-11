out vec4 fcol;

in vec2 tex;

uniform sampler2D teximg;

void main(void)
{
  fcol = vec4(texture(teximg, tex).rgb, 1.0);
}
