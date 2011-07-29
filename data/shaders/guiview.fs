in vec2 vtexcoord;

// This unfortunate name is all Apple's fault
out vec4 fcol;

uniform sampler2D guitex;

void main(void) {
  fcol = texture(guitex, vtexcoord);
}
