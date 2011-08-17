in vec3 pos;
in vec3 nor;
in vec3 tan;
in vec2 te0;

out vec2 vtex;
out vec3 vnor;
out vec3 vtan;
out vec3 vbitan;

uniform mat4 modelview[SENSE_MAX_INSTANCES];

void main(void) 
{
  vtex = te0;
  vnor = nor;
  vtan = tan;
  vbitan = cross(vnor, vtan);
  gl_Position = modelview[gl_InstanceID] * vec4(pos, 1.0);
}
