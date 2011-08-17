out vec4 gcol;
out vec4 gnor;
out vec2 gmat;

in vec3 vnor;
in vec3 vtan;
in vec3 vbitan;
in vec2 vtex;

uniform sampler2D teximg;

void main(void)
{
  gcol = vec4(texture(teximg, vtex).rgb, 1.0);
  gnor = vec4(vnor * 0.5 + 0.5, 0.0);
  gmat = vec2(0.0, 0.0);
}
