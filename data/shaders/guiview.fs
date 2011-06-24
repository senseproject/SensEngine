in vec2 vtexcoord;

layout(location = QNT_FRAG_OUTPUT_COL) out vec4 fcolor;

uniform sampler2D guitex;

void main(void) {
  fcolor = vec4(texture(guitex, vtexcoord).rgb, 1.0);
}