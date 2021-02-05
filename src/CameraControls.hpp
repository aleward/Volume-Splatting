#pragma once

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "utilityCore.hpp"

namespace CameraControls {
	// Camera View
	static const float fovy = (float)(PI / 4);
	static const float zNear = 0.10f;
	static const float zFar = 10.0f;

	// Camera Placement
	static float theta = 1.22f;
	static float phi = -0.70f;
	static float zoom = 4.0f;

	static glm::vec3 lookAt = glm::vec3(0.0f, 0.0f, 0.0f);
	static glm::vec3 cameraPosition = glm::vec3();
	static glm::mat4 projection = glm::mat4();

	void centerView(float x, float y, float z);

	void rotate(float phiDif, float thetaDif);
	void zoomDist(float zoomDif);

	void updateCamera(GLuint * program, const unsigned int PROG_BOID, 
		int width, int height);
}