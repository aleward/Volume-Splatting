#pragma once

// Useful Files
#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "kernel.h"
#include <glm/glm.hpp>
#include "utilityCore.hpp"
#include "CameraControls.hpp"
#include "ITKReader.hpp"

// GUI-Specific Files
#include "imgui/imgui.h"
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui/imgui_internal.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "ImGuiFileDialog/ImGuiFileDialog/ImGuiFileDialog.h"
#include "ImGuiFileDialog/CustomFont.cpp"

namespace GUI {
	// Pointer to the popup window
	static ImGuiWindow* editWindow;
	static ImGuiWindow* listWindow;
	static bool isDragging = false;
	static int selectedFile = -1;
	static bool dye = false;

	// Denoting the range of the visible scene/file space, based on the sliders
	static glm::vec2 xRange;
	static glm::vec2 yRange;
	static glm::vec2 zRange;
	static glm::vec2 aRange;
							
	static int begin_W, begin_L, begin_H, begin_A;
	static int end_W, end_L, end_H, end_A;

	// Getters not rly needed
	glm::vec2 getXRange();
	glm::vec2 getYRange();
	glm::vec2 getZRange();
	glm::vec2 getARange();

	bool getDye();

	void updateXRange(int min, int max);
	void updateYRange(int min, int max);
	void updateZRange(int min, int max);
	void updateARange(int min, int max);

	void resetRanges();
	void loadRanges(GLuint * program, const unsigned int PROG_BOID);
	void loadDye(GLuint * program, const unsigned int PROG_BOID);

	void initIMGUIcontext(GLFWwindow *window);

	void imguiFileDemo(GLFWwindow *window);

	// Overall GUI Setup
	bool myGUI(GLFWwindow *window,
		GLuint * program, const unsigned int PROG_BOID);

	// Custom Duo-Slider
	bool DragIntRangeCustom(bool* drag, const char* name,
		int* v_current_min, int* v_current_max, int v_min, int v_max,
		int color, const char* format = "%d", const char* format_max = NULL,
		ImGuiSliderFlags flags = ImGuiSliderFlags_AlwaysClamp);

	// Custom Slider for Slice Viewing
	bool SliceSlider(bool* drag, const char* name, int* value, int* toAdjust, 
		int min, int max, int color, const char* format = "",
		ImGuiSliderFlags flags = ImGuiSliderFlags_AlwaysClamp);

	// GUI-Relevant functions
	void screenResize(int prevWidth, int currWidth);
	bool inWindow(int x, int y, GLuint * program, const unsigned int PROG_BOID);
}