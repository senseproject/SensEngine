in vec2 vtexcoord;

// This unfortunate name is all Apple's fault
out vec4 gcol;

uniform sampler2D guitex;

void main(void) {
  gcol = texture(guitex, vtexcoord);
}
