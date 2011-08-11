out vec4 fcol;

in vec2 tex;

void main(void)
{
  vec4 black  = vec4(0.f, 0.f, 0.f, 1.f);
  vec4 white = vec4(1.f, 1.f, 1.f, 1.f);
  fcol = vec4(mix(1.f, 0.f, tex.x), mix(1.f, 0.f, tex.y), 0.f, 1.f);
}
