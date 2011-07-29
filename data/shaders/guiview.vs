// These unfortunate names are the result of Apple not supporting explicit_attrib_location
in vec3 pos;
in vec2 te0;

out vec2 vtexcoord;

uniform mat4 modelview[SENSE_MAX_INSTANCES];
uniform mat4 projection;

void main(void) {
  vtexcoord = te0;
  gl_Position = projection * modelview[gl_InstanceID] * vec4(pos, 1.0);
}