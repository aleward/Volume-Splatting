#pragma once

// Currently branching off from CIS565's HW's kernel structre

#include <stdio.h>
#include <thrust/sort.h>
#include <thrust/execution_policy.h>
#include <thrust/random.h>
#include <thrust/device_vector.h>
#include <glm/glm.hpp>
#include "utilityCore.hpp"
#include <cuda.h>
#include <cmath>
#include <vector>

// Width, length will be about 512 / image dimensions for our examples

// TODO DEPTH SORTING store distances to camera in a Map and use for rendering order

namespace Splats {
	static const float orig_width = 512.f;
	static const float orig_length = 512.f;

	// Overall DCM Dimensions
	static float scene_spacing = 1.f; // 1.0f; // 6.4:512 is best for all memory of cube

	static float scene_width  = floor(orig_width / scene_spacing) * scene_spacing; // 80.f;
	static float scene_length = floor(orig_length / scene_spacing) * scene_spacing; // 80.f;
	static float scene_height = std::min(floor(1.f / scene_spacing) * scene_spacing, scene_spacing);//512.f; // 80.f;
	static float scene_scale = std::max(std::max(scene_width, scene_length), 
										scene_height);

	float width();
	float length();
	float height();
	float spacing();
	float scale();

	void setDimensions(float w, float l, float h, float s);

	// To run the sim
    void initSimulation(int N);
	void simpleUpdate(float dt);
    void stepSimulationScatteredGrid(float dt);
    //void stepSimulationCoherentGrid(float dt);
	void copySplatsToVBO(float *vbodptr_positions);

    void endSimulation();

	// A test leftover from CIS 565:
    void unitTest();
}
