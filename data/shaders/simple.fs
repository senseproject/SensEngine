out vec4 fcol;
out vec4 fnor;
out vec2 fmat;

in vec3 vnor;
in vec3 vtan;
in vec3 vbitan;
in vec2 vtex;

uniform sampler2D teximg;

void main(void)
{
  fcol = vec4(texture(teximg, vtex).rgb, 1.0);
  fnor = vec4(vnor * 0.5 + 0.5, 0.0);
  fmat = vec2(0.0, 0.0);
}
