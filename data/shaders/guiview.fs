in vec2 vtexcoord;

layout(location = SENSE_FRAG_OUTPUT_COL) out vec4 fcolor;

uniform sampler2D guitex;

void main(void) {
  fcolor = texture(guitex, vtexcoord);
}
