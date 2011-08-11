in vec3 pos;
in vec2 te0;

out vec2 tex;

void main(void) 
{
  tex = te0;
  gl_Position = vec4(pos, 1.0);
}

