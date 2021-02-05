#version 330

uniform mat4 u_projMatrix;
uniform float u_ratio;
uniform float u_pointSize;

uniform vec2 u_xRange;
uniform vec2 u_yRange;
uniform vec2 u_zRange;
uniform bool u_tint;

// have a display by-color or by-transparency option?

layout(points) in;
layout(points) out;
layout(max_vertices = 1) out;

in float vAlpha[];
in float vTint[];
out vec4 vFragColor;

void main() {
    vec3 Position = gl_in[0].gl_Position.xyz;
	gl_Position = u_projMatrix * vec4(Position, 1.0);

	gl_PointSize = u_ratio / 720.0 * u_pointSize / gl_Position.w;

	if (Position.x <= u_xRange.x || Position.x > u_xRange.y ||
		Position.y <= u_yRange.x || Position.y > u_yRange.y ||
		Position.z <= u_zRange.x || Position.z > u_zRange.y) {

		vFragColor = vec4(0.0, 0.0, 0.0, 0.0);
	}
	else {
		float xRange = u_xRange.y - u_xRange.x;
		float yRange = u_yRange.y - u_yRange.x;
		float blend = (Position.x - u_xRange.x) / xRange + 
					  1.0 - (Position.y - u_yRange.x) / yRange;
		float dim = -blend * 0.1;

		vFragColor = vec4(0.2, sqrt(1.0 - blend / 2.0) + dim, sqrt(blend) + dim, vAlpha[0]);

		if (u_tint) {
			vFragColor = mix(vFragColor, vec4(1.0, 1.0, 1.0, vAlpha[0]), vTint[0]);
		}
	}

    EmitVertex();
    EndPrimitive();
}
