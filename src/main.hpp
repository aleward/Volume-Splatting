#pragma once

// Useful Files
#include <iostream>
#include <cstdlib>
#include <string>
#include <sstream>
#include <fstream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cuda_runtime.h>
#include <cuda_gl_interop.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "utilityCore.hpp"
#include "glslUtility.hpp"

// Files For Variable Management
//#include "kernel.h"
//#include "CameraControls.hpp"
#include "GUI.hpp"
//#include "ITKReader.hpp"

// TODO add a VRAM check
// I have 8031 MB for NVIDIA

//====================================
// GL Stuff
//====================================

GLuint positionLocation = 0;   // Match results from glslUtility::createProgram.
GLuint alphasLocation = 1; //velocitiesLocation = 1; // Also see attribtueLocations below.
const char *attributeLocations[] = { "Position", "Alpha" };

GLuint splatVAO = 0;
GLuint splatVBO_positions = 0;
GLuint splatVBO_alphas = 0;//splatVBO_velocities = 0;
GLuint splatIBO = 0;
GLuint displayImage;
GLuint program[2];

const unsigned int PROG_BOID = 0;

// LOOK-1.2: for high DPI displays, you may want to double these settings.
int width = 1280;
int height = 720;
int pointSize;

// For camera controls
bool leftMousePressed = false;
bool rightMousePressed = false;
double lastX;
double lastY;
double lastScroll;

//====================================
// Main
//====================================

const char *projectName;

int main(int argc, char* argv[]);

//====================================
// Main loop
//====================================
void mainLoop();
void errorCallback(int error, const char *description);
void windowResizeCallback(GLFWwindow* window, int width, int height);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void mousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void mouseScrollCallback(GLFWwindow* window, double xpos, double ypos);
void runCUDA();

//====================================
// Setup/init Stuff
//====================================
bool init(int argc, char **argv);
void initVAO();
void initVAO_vecs(std::unique_ptr<GLfloat[]>* splats,
				  std::unique_ptr<GLfloat[]>* alphas,
				  std::unique_ptr<GLuint[]>* sindices);
void loadVAO_vecs(std::unique_ptr<GLfloat[]>* splats,
				  std::unique_ptr<GLfloat[]>* alphas,
				  std::unique_ptr<GLuint[]>* sindices);
void initVAO_gen();
void initVAO_bind(std::unique_ptr<GLfloat[]>* splats,
				  std::unique_ptr<GLfloat[]>* alphas,
				  std::unique_ptr<GLuint[]>* sindices);
void initShaders(GLuint *program);

void resetVAO();
void resetSim();

void setPointSize();
void setN();
