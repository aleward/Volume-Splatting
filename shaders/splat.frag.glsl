#version 330

in vec4 vFragColor;
out vec4 fragColor;

void main() {
	if (vFragColor.a == 0.0) {
		discard;
	}

	fragColor = vFragColor;
}
