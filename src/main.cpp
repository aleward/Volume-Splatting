/**
* @file      main.cpp
* @brief     Branches off of a file from a CIS 565 HW:
* @authors   Liam Boone, Kai Ninomiya, Kangning (Gary) Li
* @date      2013-2017
* @copyright University of Pennsylvania
*/

#include "main.hpp"

// ================
// Configuration
// ================

// LOOK-2.1 LOOK-2.3 - toggles for UNIFORM_GRID and COHERENT_GRID
#define VISUALIZE 1
#define UNIFORM_GRID 1
#define COHERENT_GRID 0

// LOOK-1.2 - change this to adjust particle count in the simulation
int N_FOR_VIS;/* = Splats::scene_height * Splats::scene_width * Splats::scene_length /
	(Splats::scene_spacing * Splats::scene_spacing * Splats::scene_spacing);//25000;*/
const float DT = 0.2f;

/**
* C main function.
*/
int main(int argc, char* argv[]) {
    projectName = "Volume Splatting";

	setN();
	setPointSize();
	GUI::resetRanges();
    
    if (init(argc, argv)) {
        mainLoop();
        Splats::endSimulation();
        return 0;
    } else {
        return 1;
    }
}

void setPointSize() { // previously 32.f
	pointSize = std::round(55.0 * 80.0f * Splats::spacing()) / Splats::scale();
}

void setN() {
	N_FOR_VIS = Splats::height() * Splats::width() * Splats::length() /
		(Splats::spacing() * Splats::spacing() * Splats::spacing());
}

//-------------------------------
//---------RUNTIME STUFF---------
//-------------------------------

std::string deviceName;
GLFWwindow *window;

/**
* Initialization of CUDA and GLFW.
*/
bool init(int argc, char **argv) {
    // Set window title to "Volume Splatting [SM # GPU Name]"
    cudaDeviceProp deviceProp;
    int gpuDevice = 0;
    int device_count = 0;
    cudaGetDeviceCount(&device_count);
    if (gpuDevice > device_count) {
        std::cout
        << "Error: GPU device number is greater than the number of devices!"
        << " Perhaps a CUDA-capable GPU is not installed?"
        << std::endl;
        return false;
    }
    cudaGetDeviceProperties(&deviceProp, gpuDevice);
    int major = deviceProp.major;
    int minor = deviceProp.minor;
    
    std::ostringstream ss;
    ss << projectName << " [SM " << major << "." << minor << " " << deviceProp.name << "]";
    deviceName = ss.str();
    
    // Window setup stuff
    glfwSetErrorCallback(errorCallback);
    
    if (!glfwInit()) {
        std::cout
        << "Error: Could not initialize GLFW!"
        << " Perhaps OpenGL 3.3 isn't available?"
        << std::endl;
        return false;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only?
    
    window = glfwCreateWindow(width, height, deviceName.c_str(), NULL, NULL);
    if (!window) {
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);
    // IMGUI: glfwSwapInterval(1); // To enable VSync
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, mousePositionCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetWindowSizeCallback(window, windowResizeCallback);
	glfwSetScrollCallback(window, mouseScrollCallback);
    
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        return false;
    }
    
    // TODO setup Dear IMGUI context "#version 130"?
    GUI::initIMGUIcontext(window);
    
    // Initialize drawing state
    initVAO();
    
    // Default to device ID 0. If you have more than one GPU and want to test a non-default one,
    // change the device ID.
    cudaGLSetGLDevice(0);
    
    cudaGLRegisterBufferObject(splatVBO_positions);
	//cudaGLRegisterBufferObject(splatVBO_alphas);// CHECK and in reset
    //cudaGLRegisterBufferObject(splatVBO_velocities);
    
    // Initialize N-body simulation
	Splats::initSimulation(N_FOR_VIS);
    
    initShaders(program);
	CameraControls::updateCamera(program, PROG_BOID, width, height);

	// TODO maybe get rid of this for depth stuff
    glEnable(GL_DEPTH_TEST);
    
    return true;
}

void initVAO() {

	std::unique_ptr<GLfloat[]> splats{ new GLfloat[4 * (N_FOR_VIS)] };
	std::unique_ptr<GLfloat[]> alphas{ new GLfloat[N_FOR_VIS] };
    std::unique_ptr<GLuint[]> sindices{ new GLuint[N_FOR_VIS] };
    
    glm::vec4 ul(-1.0, -1.0, 1.0, 1.0);
    glm::vec4 lr(1.0, 1.0, 0.0, 0.0);

	initVAO_vecs(&splats, &alphas, &sindices);
    
	initVAO_gen();

	initVAO_bind(&splats, &alphas, &sindices);
}

void resetVAO() {
	std::unique_ptr<GLfloat[]> splats{ new GLfloat[4 * (N_FOR_VIS)] };
	std::unique_ptr<GLfloat[]> alphas{ new GLfloat[N_FOR_VIS] };
	std::unique_ptr<GLuint[]> sindices{ new GLuint[N_FOR_VIS] };

	glm::vec4 ul(-1.0, -1.0, 1.0, 1.0);
	glm::vec4 lr(1.0, 1.0, 0.0, 0.0);

	loadVAO_vecs(&splats, &alphas, &sindices);
	initVAO_gen();
	initVAO_bind(&splats, &alphas, &sindices);
}

void initVAO_vecs(std::unique_ptr<GLfloat[]>* splats, 
				  std::unique_ptr<GLfloat[]>* alphas,
				  std::unique_ptr<GLuint[]>* sindices) {
	for (int i = 0; i < N_FOR_VIS; i++) {
		(*splats)[4 * i + 0] = 0.0f;
		(*splats)[4 * i + 1] = 0.0f;
		(*splats)[4 * i + 2] = 0.0f;
		(*splats)[4 * i + 3] = 1.0f;
		(*sindices)[i] = i;
		(*alphas)[i] = 1.0f;
	}
}

void loadVAO_vecs(std::unique_ptr<GLfloat[]>* splats,
	std::unique_ptr<GLfloat[]>* alphas,
	std::unique_ptr<GLuint[]>* sindices) {

	float w = Splats::width() / Splats::spacing();
	float l = Splats::length() / Splats::spacing();
	float h = Splats::height() / Splats::spacing();

	for (int i = 0; i < N_FOR_VIS; i++) {
		(*splats)[4 * i + 0] = 0.0f;
		(*splats)[4 * i + 1] = 0.0f;
		(*splats)[4 * i + 2] = 0.0f;
		(*splats)[4 * i + 3] = 1.0f;
		(*sindices)[i] = i;


		int z = (i % (int)h) * Splats::spacing();
		int y = ((i / (int)h) % (int)l) * Splats::spacing();
		int x = (i / ((int)h * (int)l)) * Splats::spacing();

		(*alphas)[i] = ITKReader::getAlphaAt(x, y, z);
	}
}

void initVAO_gen() {
	glGenVertexArrays(1, &splatVAO); // Attach everything needed to draw a particle to this
	glGenBuffers(1, &splatVBO_positions);
	glGenBuffers(1, &splatVBO_alphas); //&splatVBO_velocities);
	glGenBuffers(1, &splatIBO);
}

void initVAO_bind(std::unique_ptr<GLfloat[]>* splats,
				  std::unique_ptr<GLfloat[]>* alphas,
				  std::unique_ptr<GLuint[]>* sindices) {
	glBindVertexArray(splatVAO);

	// Bind the positions array to the splatVAO by way of the splatVBO_positions
	glBindBuffer(GL_ARRAY_BUFFER, splatVBO_positions); // bind the buffer
	glBufferData(GL_ARRAY_BUFFER, 4 * (N_FOR_VIS) * sizeof(GLfloat), (*splats).get(), GL_DYNAMIC_DRAW); // transfer data

	glEnableVertexAttribArray(positionLocation);
	glVertexAttribPointer((GLuint)positionLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);

	// Bind the alphas array to the splatVAO by way of the splatVBO_alphas
	glBindBuffer(GL_ARRAY_BUFFER, splatVBO_alphas);
	glBufferData(GL_ARRAY_BUFFER, (N_FOR_VIS) * sizeof(GLfloat), (*alphas).get(), GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(alphasLocation);
	glVertexAttribPointer((GLuint)alphasLocation, 1, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, splatIBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (N_FOR_VIS) * sizeof(GLuint), (*sindices).get(), GL_STATIC_DRAW);

	glBindVertexArray(0);
}

void initShaders(GLuint * program) {
    GLint location;
    
    program[PROG_BOID] = glslUtility::createProgram(
      "../shaders/splat.vert.glsl",
      "../shaders/splat.geom.glsl",
      "../shaders/splat.frag.glsl", attributeLocations, 2);
    glUseProgram(program[PROG_BOID]);

    if ((location = glGetUniformLocation(program[PROG_BOID], "u_projMatrix")) != -1) {
      glUniformMatrix4fv(location, 1, GL_FALSE, &CameraControls::projection[0][0]);
    }
	if ((location = glGetUniformLocation(program[PROG_BOID], "u_cameraPos")) != -1) {
		glUniform3fv(location, 1, &CameraControls::cameraPosition[0]);
	}
	if ((location = glGetUniformLocation(program[PROG_BOID], "u_pointSize")) != -1) {
		glUniform1f(location, pointSize);
	}
	if ((location = glGetUniformLocation(program[PROG_BOID], "u_ratio")) != -1) {
		glUniform1f(location, height);
	}

	GUI::loadRanges(program, PROG_BOID);
	GUI::loadDye(program, PROG_BOID);
}

//====================================
// Main loop
//====================================
void runCUDA() {
    // Map OpenGL buffer object for writing from CUDA on a single GPU
    // No data is moved (Win & Linux). When mapped to CUDA, OpenGL should not
    // use this buffer
 
    float4 *dptr = NULL;
    float *dptrVertPositions = NULL;
    //float *dptrVertVelocities = NULL;
 
    cudaGLMapBufferObject((void**)&dptrVertPositions, splatVBO_positions);
    //cudaGLMapBufferObject((void**)&dptrVertVelocities, splatVBO_velocities);
 
    // execute the kernel
    //#if UNIFORM_GRID && COHERENT_GRID
	//Splats::stepSimulationCoherentGrid(DT);
    //#elif UNIFORM_GRID
	//Splats::stepSimulationScatteredGrid(DT);
    //#endif
	//Splats::simpleUpdate(DT);
 
    #if VISUALIZE
	Splats::copySplatsToVBO(dptrVertPositions);
    #endif
    // unmap buffer object
    cudaGLUnmapBufferObject(splatVBO_positions);
    //cudaGLUnmapBufferObject(splatVBO_velocities);
}

void mainLoop() {
    double fps = 0;
    double timebase = 0;
    int frame = 0;
 
	Splats::unitTest(); // LOOK-1.2 We run some basic example code to make sure
                       // your CUDA development setup is ready to go.
 
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
 	  
        frame++;
        double time = glfwGetTime();
 	  
        if (time - timebase > 1.0) {
            fps = frame / (time - timebase);
            timebase = time;
            frame = 0;
        }
 	  
        //runCUDA(); // TODO Fill with splat relevant stuff
 	  
        std::ostringstream ss;
        ss << "[";
        ss.precision(1);
        ss << std::fixed << fps;
        ss << " fps] " << deviceName;
        glfwSetWindowTitle(window, ss.str().c_str());
 	  
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 	  
        #if VISUALIZE
        glUseProgram(program[PROG_BOID]);
        glBindVertexArray(splatVAO);
 	  
		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
 		 
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 	  
		if (ITKReader::hasFiles()) {
			// TODO DEPTH SORTING use a map of distances determined in Kernel for drawing order - stream compaction?
			glDrawElements(GL_POINTS, N_FOR_VIS + 1, GL_UNSIGNED_INT, 0);
		}
 	  
        glUseProgram(0);
        glBindVertexArray(0);
 	  
		bool newScene = GUI::myGUI(window, program, PROG_BOID);
		//GUI::imguiFileDemo(window);

		if (newScene && ITKReader::hasFiles()) {
			resetSim();
		}
 	  
        glfwSwapBuffers(window);
        #endif
    }
    glfwDestroyWindow(window);
    glfwTerminate();
}

void resetSim() {
	Splats::endSimulation();
	//Clear the VBOs - i think initVao does it automatically

	// Recalculate N and things:
	// TODO - replace w and l with DCM image dimensions
	Splats::setDimensions(Splats::orig_width, Splats::orig_length, // changed to these for consistency for now
		ITKReader::numFiles(), Splats::spacing());
	GUI::resetRanges();
	GUI::loadRanges(program, PROG_BOID);
	setPointSize();
	setN();

	// Then these - some of the functionalities w/in might be overkill
	resetVAO(); // initVAO(); //
	cudaGLRegisterBufferObject(splatVBO_positions);
	//cudaGLRegisterBufferObject(splatVBO_velocities);
	Splats::initSimulation(N_FOR_VIS);

	// Moved to here from loop bc only needed to update positions 
	// TODO: Separate into two separate functions instead
	runCUDA();
}


void errorCallback(int error, const char *description) {
    fprintf(stderr, "error %d: %s\n", error, description);
}


void windowResizeCallback(GLFWwindow* window, int w, int h) {
	GUI::screenResize(width, w);

	width = w;
	height = h;

	GLint location;
	glUseProgram(program[PROG_BOID]);
    if ((location = glGetUniformLocation(program[PROG_BOID], "u_ratio")) != -1) {
        glUniform1f(location, height);
    }
	CameraControls::updateCamera(program, PROG_BOID, width, height);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    leftMousePressed = (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS);
    rightMousePressed = (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS);
}

void mousePositionCallback(GLFWwindow* window, double xpos, double ypos) {

    if (GUI::inWindow(xpos, ypos, program, PROG_BOID)) {
		lastX = xpos;
		lastY = ypos;
		return;
    }
    
    if (leftMousePressed) {
        // compute new camera parameters
		CameraControls::rotate((xpos - lastX) / width, -(ypos - lastY) / height);
		CameraControls::updateCamera(program, PROG_BOID, width, height);
    }
    else if (rightMousePressed) {
		CameraControls::zoomDist((ypos - lastY) / height);
		CameraControls::updateCamera(program, PROG_BOID, width, height);
    }
 
    lastX = xpos;
    lastY = ypos;
}

void mouseScrollCallback(GLFWwindow* window, double xpos, double ypos) {
	//CameraControls::zoomDist((ypos - lastScroll) / height);
	//CameraControls::updateCamera(program, PROG_BOID, width, height);
	//lastScroll = ypos;
}