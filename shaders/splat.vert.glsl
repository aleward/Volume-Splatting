#version 330

uniform vec2 u_aRange;
uniform bool u_tint;

in vec4 Position;
in float Alpha;

out float vAlpha;
out float vTint;

void main() {
	// The value of the DICOM pixel, clamped
	vAlpha = clamp((Alpha - u_aRange.x) / (u_aRange.y - u_aRange.x), 0.0f, 1.0f);

	// Alpha values greater than 1680 get blended to white, if enabled
	if (u_tint) { vTint = max(Alpha - 1680.0f, 0.0f) / (3072.0f - 1680.0f); }
	else { vTint = 0.0f; }

    gl_Position = Position;
}
