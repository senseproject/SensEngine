layout(location = SENSE_VERT_INPUT_POS) in vec3 position;
layout(location = SENSE_VERT_INPUT_TE0) in vec2 texcoord;

out vec2 vtexcoord;

uniform mat4 modelview[SENSE_MAX_INSTANCES];
uniform mat4 projection;

void main(void) {
  vtexcoord = texcoord;
  gl_Position = projection * modelview[gl_InstanceID] * vec4(position, 1.0);
}