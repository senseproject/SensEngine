uniform sampler2DMS gcol;
uniform sampler2DMS gnor;
uniform sampler2DMS gmat;

out vec4 lcol;

const int num_samples=16; // TODO: pass this in

void main()
{
  vec4 color;
  for(int i = 0; i < num_samples; i++) {
    color += texelFetch(gcol, ivec2(floor(gl_FragCoord.xy)), i);
  }
  lcol = color / num_samples;
}
