in vec3 pos;
in vec3 nor;
in vec3 tan;
in vec2 te0;

out vec2 vtex;
out vec3 vnor;
out vec3 vtan;
out vec3 vbitan;

void main(void) 
{
  vtex = te0;
  vnor = nor;
  vtan = tan;
  vbitan = cross(vnor, vtan);
  gl_Position = vec4(pos.xy, pos.z * -1.0, 1.0);
}
