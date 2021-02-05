#include "CameraControls.hpp"

void CameraControls::centerView(float x, float y, float z) {
	lookAt.x = x;
	lookAt.y = y;
	lookAt.z = z;
}

void CameraControls::rotate(float phiDif, float thetaDif) {
	phi += phiDif;
	theta += thetaDif;
	theta = std::fmax(0.01f, std::fmin(theta, 3.14f));
}

void CameraControls::zoomDist(float zoomDif) {
	zoom += zoomDif;
	zoom = std::fmax(0.1f, std::fmin(zoom, 5.0f));
}

void CameraControls::updateCamera(GLuint* program, const unsigned int PROG_BOID, int width, int height) {
	cameraPosition.x = zoom * sin(phi) * sin(theta);
	cameraPosition.z = zoom * cos(theta);
	cameraPosition.y = zoom * cos(phi) * sin(theta);
	cameraPosition += lookAt;

	projection = glm::perspective(fovy, float(width) / float(height), zNear, zFar);
	glm::mat4 view = glm::lookAt(cameraPosition, lookAt, glm::vec3(0, 0, 1));
	projection = projection * view;

	GLint location;

	glUseProgram(program[PROG_BOID]);
	if ((location = glGetUniformLocation(program[PROG_BOID], "u_projMatrix")) != -1) {
		glUniformMatrix4fv(location, 1, GL_FALSE, &projection[0][0]);
	}
	//if ((location = glGetUniformLocation(program[PROG_BOID], "u_cameraPos")) != -1) {
	//	glUniform3fv(location, 1, &cameraPosition[0]);
	//}
}