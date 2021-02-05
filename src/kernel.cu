#define GLM_FORCE_CUDA
#include <stdio.h>
#include <cuda.h>
#include <cmath>
#include <glm/glm.hpp>
#include "utilityCore.hpp"
#include "kernel.h"
//#include "device_launch_parameters.h"


#ifndef imax
#define imax( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif

#ifndef imin
#define imin( a, b ) ( ((a) < (b)) ? (a) : (b) )
#endif

#define checkCUDAErrorWithLine(msg) checkCUDAError(msg, __LINE__)

/**
* Check for CUDA errors; print and exit if there was a problem.
*/
void checkCUDAError(const char *msg, int line = -1) {
	cudaError_t err = cudaGetLastError();
	if (cudaSuccess != err) {
		if (line >= 0) {
			fprintf(stderr, "Line %d: ", line);
		}
		fprintf(stderr, "Cuda error: %s: %s.\n", msg, cudaGetErrorString(err));
		exit(EXIT_FAILURE);
	}
}


/*****************
* Configuration *
*****************/

/*! Block size used for CUDA kernel launch. */
#define blockSize 64 // 32 // 

// TODO Parameters for comparing neighboring splats - check if applies.
// These worked well in 565 reference implementation.
#define rule1Distance 5.0f
#define rule2Distance 3.0f
#define rule3Distance 5.0f

#define rule1Scale 0.01f
#define rule2Scale 0.1f
#define rule3Scale 0.1f

#define maxSpeed 1.0f

/***********************************************
* Kernel state (pointers are device pointers) *
***********************************************/

int numObjects;
dim3 threadsPerBlock(blockSize);

// Buffers to hold all the splat information.
// These get allocated in Splats::initSimulation.
glm::vec3 *dev_pos;
// Ping-pong buffer example. Unneeded as of now
glm::vec3 *dev_vel1;
glm::vec3 *dev_vel2;

// TODO - to use these buffers for neighbor search
// For efficient sorting and the uniform grid. These should always be parallel.
int *dev_particleArrayIndices; // What index in dev_pos and dev_velX represents this particle?
int *dev_particleGridIndices; // What grid cell is this particle in?
// needed for use with thrust
thrust::device_ptr<int> dev_thrust_particleArrayIndices;
thrust::device_ptr<int> dev_thrust_particleGridIndices;

int *dev_gridCellStartIndices; // What part of dev_particleArrayIndices belongs
int *dev_gridCellEndIndices;   // to this cell?

// Additional buffers for sorting data
glm::vec3 *dev_positionThrust;
glm::vec3 *dev_velocityThrust;

// Grid parameters based on simulation parameters.
// These are automatically computed in Splats::initSimulation
int gridCellCount;
int gridSideCount;
float gridCellWidth;
float gridInverseCellWidth;
glm::vec3 gridMinimum;

/*******************
* Important Getters*
*******************/

float Splats::width() {
	return scene_width;
}

float Splats::length() {
	return scene_length;
}

float Splats::height() {
	return scene_height;
}

float Splats::spacing() {
	return scene_spacing;
}

float Splats::scale() {
	return scene_scale;
}

void resetValueCheck(std::string valName, float orig, float curr, float spacing) {
	if (orig != curr) {
		std::cout << "Value for " << valName << " has changed from " << 
			orig << " to " << curr << " due to the chosen scene_spacing of " << 
			spacing << std::endl;
	}
}

void Splats::setDimensions(float w, float l, float h, float s) {
	// TODO Play with memory to get the max particle num higher:
	if (h > 102.0 && s < 1.5) {
		scene_spacing = 1.5f;
		std::cout << "Not enough VRAM for the detail of this scene. " << 
			"Scene_spacing increased to " << scene_spacing << std::endl;
	}
	else if (h <= 102.0 && s == 1.5) {
		scene_spacing = 1.f;
		std::cout << "We can get finer detail at this level so we decreased scene_spacing to " <<
			scene_spacing << std::endl;
	}
	else {
		scene_spacing = s;
	}

	scene_width  = floor(w / scene_spacing) * scene_spacing;
	scene_length = floor(l / scene_spacing) * scene_spacing;
	scene_height = floor(h / scene_spacing) * scene_spacing;

	if (h > 0.0 && scene_height < 1.0) { scene_height = scene_spacing; }

	scene_scale = std::max(std::max(scene_width, scene_length), scene_height);

	// Let the user know that some values may have changed due  to the chosen scene_spacing
	resetValueCheck("scene_width", w, scene_width, scene_spacing);
	resetValueCheck("scene_length", l, scene_length, scene_spacing);
	resetValueCheck("scene_height", h, scene_height, scene_spacing);
}

/******************
* initSimulation *
******************/

__host__ __device__ unsigned int hash(unsigned int a) {
	a = (a + 0x7ed55d16) + (a << 12);
	a = (a ^ 0xc761c23c) ^ (a >> 19);
	a = (a + 0x165667b1) + (a << 5);
	a = (a + 0xd3a2646c) ^ (a << 9);
	a = (a + 0xfd7046c5) + (a << 3);
	a = (a ^ 0xb55a4f09) ^ (a >> 16);
	return a;
}

// Splat - setting grid based on DICOM image dimensions
__global__ void kernGenerateInitPosArray(int time, int N, glm::vec3 * arr, float length, float width, float height, float size) { //float scale, float height, float size) { 
	int index = (blockIdx.x * blockDim.x) + threadIdx.x; // maybe make this correspond with images? - nah
	if (index < N) {
		arr[index].z = ((float)(index % (int)height) - height / 2.0f) * size;
		arr[index].y = ((float)((index / (int)height) % (int)length) - length / 2.0f) * size;
		arr[index].x = ((float)(index / ((int)height * (int)length)) - width / 2.0f) * size; 

		// TODO strat - maybe - check if segmented (maybe rewrite the images with alpha channel, or just roll wit black for now)
		// If choosing a threshold for irrelevant pixels, dont change anything with the position, then use stream compaction to ignore it for future kernels?
	}
}
// Replace with a read @ index thing

/**
* Initialize memory, update some globals
*/
void Splats::initSimulation(int N) {
	numObjects = N;
	dim3 fullBlocksPerGrid((N + blockSize - 1) / blockSize);

	// Don't forget to cudaFree in  Splats::endSimulation.
	cudaMalloc((void**)&dev_pos, N * sizeof(glm::vec3));
	checkCUDAErrorWithLine("cudaMalloc dev_pos failed!");

	//cudaMalloc((void**)&dev_vel1, N * sizeof(glm::vec3));
	//checkCUDAErrorWithLine("cudaMalloc dev_vel1 failed!");
	//
	//cudaMalloc((void**)&dev_vel2, N * sizeof(glm::vec3));
	//checkCUDAErrorWithLine("cudaMalloc dev_vel2 failed!");

	// TODO DEPTH SORTING only if camera has moved since last time
	kernGenerateInitPosArray << <fullBlocksPerGrid, blockSize >> > (1, numObjects,
		dev_pos, scene_length / scene_spacing, scene_width / scene_spacing, 
		scene_height / scene_spacing, scene_spacing * 2.0f);
	checkCUDAErrorWithLine("kernGenerateInitPosArray failed!");

	// TODO - check computing grid params
	//gridCellWidth = 2.0f * std::max(std::max(rule1Distance, rule2Distance), rule3Distance);
	//int halfSideCount = (int)(scene_scale / gridCellWidth) + 1;
	//gridSideCount = 2 * halfSideCount;
	//
	//gridCellCount = gridSideCount * gridSideCount * gridSideCount;
	//gridInverseCellWidth = 1.0f / gridCellWidth;
	//float halfGridWidth = gridCellWidth * halfSideCount;
	//gridMinimum.x -= halfGridWidth;
	//gridMinimum.y -= halfGridWidth;
	//gridMinimum.z -= halfGridWidth;

	// TODO - Allocate index-related buffers here.
	///cudaMalloc((void**)&dev_particleArrayIndices, N * sizeof(int));
	///checkCUDAErrorWithLine("cudaMalloc dev_particleArrayIndices failed!");

	// TODO - and the space-management needed
	//cudaMalloc((void**)&dev_particleGridIndices, N * sizeof(int));
	//checkCUDAErrorWithLine("cudaMalloc dev_particleGridIndices failed!");
	//
	//cudaMalloc((void**)&dev_gridCellStartIndices, N * sizeof(int));
	//checkCUDAErrorWithLine("cudaMalloc dev_gridCellStartIndices failed!");
	//
	//cudaMalloc((void**)&dev_gridCellEndIndices, N * sizeof(int));
	//checkCUDAErrorWithLine("cudaMalloc dev_gridCellEndIndices failed!");
	//
	//cudaMalloc((void**)&dev_positionThrust, N * sizeof(glm::vec3));
	//checkCUDAErrorWithLine("cudaMalloc dev_positionThrust failed!");
	//
	//cudaMalloc((void**)&dev_velocityThrust, N * sizeof(glm::vec3));
	//checkCUDAErrorWithLine("cudaMalloc dev_velocityThrust failed!");

	cudaDeviceSynchronize();
}


/******************
* copySplatsToVBO *
******************/

/**
* Copy the splat positions into the VBO so that they can be drawn by OpenGL.
*/
__global__ void kernCopyPositionsToVBO(int N, glm::vec3 *pos, float *vbo, float s_scale) {
	int index = threadIdx.x + (blockIdx.x * blockDim.x);

	float c_scale = -1.0f / s_scale;

	if (index < N) {
		vbo[4 * index + 0] = pos[index].x * c_scale;
		vbo[4 * index + 1] = pos[index].y * c_scale;
		vbo[4 * index + 2] = pos[index].z * c_scale;
		vbo[4 * index + 3] = 1.0f;
	}
}

/**
* Wrapper for call to the kernCopysplatsToVBO CUDA kernel.
*/

void Splats::copySplatsToVBO(float *vbodptr_positions) { // TODO add more vbos as needed
	dim3 fullBlocksPerGrid((numObjects + blockSize - 1) / blockSize);

	kernCopyPositionsToVBO << <fullBlocksPerGrid, blockSize >> > (numObjects, dev_pos, vbodptr_positions, scene_scale);
	checkCUDAErrorWithLine("copySplatsToVBO failed!");

	cudaDeviceSynchronize();
}


/******************
* stepSimulation *
******************/

/**
* TODO Use this to update splat values
* For each of the `N` splats, update its position based on its current velocity.
*/
// TODO check if should pass by value or reference
__global__ void kernUpdatePos(int N, float dt, glm::vec3 *pos, glm::vec3 *vel, float scale) {
	// Update position by velocity
	int index = threadIdx.x + (blockIdx.x * blockDim.x);
	if (index >= N) {
		return;
	}
	glm::vec3 thisPos = pos[index];
	thisPos += vel[index] * dt;

	// Wrap the splats around so we don't lose them - unneeded now TODO delete
	//thisPos.x = thisPos.x < -scale ? scale : thisPos.x;
	//thisPos.y = thisPos.y < -scale ? scale : thisPos.y;
	//thisPos.z = thisPos.z < -scale ? scale : thisPos.z;
	//
	//thisPos.x = thisPos.x > scale ? -scale : thisPos.x;
	//thisPos.y = thisPos.y > scale ? -scale : thisPos.y;
	//thisPos.z = thisPos.z > scale ? -scale : thisPos.z;

	pos[index] = thisPos;
}


__device__ int gridIndex3Dto1D(int x, int y, int z, int gridResolution) {
	return x + y * gridResolution + z * gridResolution * gridResolution;
}

__global__ void kernComputeIndices(int N, int gridResolution,
	glm::vec3 gridMin, float inverseCellWidth,
	glm::vec3 *pos, int *indices, int *gridIndices) {
	int index = (blockIdx.x * blockDim.x) + threadIdx.x;

	if (index < N) {
		// - Label each splat with the index of its grid cell.
		glm::vec3 posInGrid = glm::floor((pos[index] - gridMin) * inverseCellWidth);
		gridIndices[index] = gridIndex3Dto1D(posInGrid.x, posInGrid.y, posInGrid.z, gridResolution);

		// - Set up a parallel array of integer indices as pointers to the actual
		//   splat data in pos and vel1/vel2
		indices[index] = index;
	}
}

/*__global__ void kernComputeCoherentIndices(int N, glm::vec3 *positionThrust, 
	glm::vec3 *velocityThrust, glm::vec3 *pos, glm::vec3 *vel, int *indices) {
	int index = (blockIdx.x * blockDim.x) + threadIdx.x;

	if (index < N) {
		// - Find the index of the current splat pre sorting.
		int thisIdx = indices[index];

		// - Update the related positions and velocities
		positionThrust[index] = pos[thisIdx];
		velocityThrust[index] = vel[thisIdx];
	}
}*/

// Indicates that a cell does not enclose any splats?
__global__ void kernResetIntBuffer(int N, int *intBuffer, int value) {
	int index = (blockIdx.x * blockDim.x) + threadIdx.x;
	if (index < N) {
		intBuffer[index] = value;
	}
}

__global__ void kernIdentifyCellStartEnd(int N, int *particleGridIndices,
	int *gridCellStartIndices, int *gridCellEndIndices) {
	// TODO check - Identify the start point of each cell in the gridIndices array.
	int index = (blockIdx.x * blockDim.x) + threadIdx.x;
	if (index < N) {
		int thisIdx = particleGridIndices[index];

		// Check previous index to see if this is a start
		if (thisIdx != particleGridIndices[index - 1]) {
			gridCellStartIndices[thisIdx] = index;
		}

		// Check next index to see if this is an end
		if (thisIdx != particleGridIndices[index + 1]) {
			gridCellEndIndices[thisIdx] = index;
		}
	}
	// This is basically a parallel unrolling of a loop that goes
	// "this index doesn't match the one before it, must be a new cell!"
}

__global__ void kernUpdateVelNeighborSearchScattered(
	int N, int gridResolution, glm::vec3 gridMin,
	float inverseCellWidth, float cellWidth,
	int *gridCellStartIndices, int *gridCellEndIndices,
	int *particleArrayIndices,
	glm::vec3 *pos, glm::vec3 *vel1, glm::vec3 *vel2) {
	// Update a splat's velocity using the uniform grid to reduce
	// the number of splats that need to be checked.

	int index = (blockIdx.x * blockDim.x) + threadIdx.x;
	if (index < N) {
		// - Identify the grid cell that this particle is in
		glm::vec3 posInGrid = glm::floor((pos[index] - gridMin) * inverseCellWidth);
		int gridCell = gridIndex3Dto1D(posInGrid.x, posInGrid.y, posInGrid.z, gridResolution);

		// - Access each splat in each cell and compute velocity change from
		//   the splats rules, if this splat is within the neighborhood distance.
		glm::vec3 perceivedCenter = glm::vec3();
		float numOfNeigh1 = 0.f;

		glm::vec3 c = glm::vec3();

		glm::vec3 perceivedVel = glm::vec3();
		float numOfNeigh3 = 0.f;

		// - Identify which cells may contain neighbors. This isn't always 8.
		for (int x = imax((int) posInGrid.x - 1, 0); x <= imin((int)posInGrid.x + 1, gridResolution); x++) {
			for (int y = imax((int)posInGrid.y - 1, 0); y <= imin((int)posInGrid.y + 1, gridResolution); y++) {
				for (int z = imax((int)posInGrid.z - 1, 0); z <= imin((int)posInGrid.z + 1, gridResolution); z++) {

					int currCell = gridIndex3Dto1D(x, y, z, gridResolution);

					// - For each cell, read the start/end indices in the splat pointer array.
					int startIdx = gridCellStartIndices[currCell];
					int endIdx = gridCellEndIndices[currCell];

					for (int b = startIdx; b < endIdx; b++) {
						int splatIdx = particleArrayIndices[b];

						if (splatIdx != index) {
							float d = glm::distance(pos[index], pos[splatIdx]);
							// Rule 1: splats fly towards their local perceived center of mass, which excludes themselves
							if (d < rule1Distance) {
								perceivedCenter += pos[splatIdx];
								numOfNeigh1++;
							}

							// Rule 2: splats try to stay a distance d away from each other
							if (d < rule2Distance) {
								c -= (pos[splatIdx] - pos[index]);
							}

							// Rule 3: splats try to match the speed of surrounding splats
							if (d < rule3Distance) {
								perceivedVel += vel1[splatIdx];
								numOfNeigh3++;
							}
						}
					}
				}
			}
		}

		if (numOfNeigh1 > 0) {
			perceivedCenter /= numOfNeigh1;
			perceivedCenter = (perceivedCenter - pos[index]) * rule1Scale;
		}

		c *= rule2Scale;

		if (numOfNeigh3 > 0) {
			perceivedVel /= numOfNeigh3;
			perceivedVel *= rule3Scale;
		}

		// Compute a new velocity based on pos and vel1
		glm::vec3 velChange = vel1[index] + perceivedCenter + c + perceivedVel;

		// - Clamp the speed change before putting the new speed in vel2
		float speed = glm::length(velChange);
		if (speed > maxSpeed) {
			velChange /= speed;
		}

		vel2[index] = velChange;
	}
}

/*__global__ void kernUpdateVelNeighborSearchCoherent(
	int N, int gridResolution, glm::vec3 gridMin,
	float inverseCellWidth, float cellWidth,
	int *gridCellStartIndices, int *gridCellEndIndices,
	glm::vec3 *pos, glm::vec3 *vel1, glm::vec3 *vel2) {
	// This is very similar to kernUpdateVelNeighborSearchScattered,
	// except with one less level of indirection.
	// This should expect gridCellStartIndices and gridCellEndIndices to refer
	// directly to pos and vel1.

	int index = (blockIdx.x * blockDim.x) + threadIdx.x;
	if (index < N) {
		// - Identify the grid cell that this particle is in
		glm::vec3 thisPos = pos[index];
		glm::vec3 posInGrid = glm::floor((thisPos - gridMin) * inverseCellWidth);
		int gridCell = gridIndex3Dto1D(posInGrid.x, posInGrid.y, posInGrid.z, gridResolution);

		// - Access each splat in each cell and compute velocity change from
		//   the splats rules, if this splat is within the neighborhood distance.
		glm::vec3 perceivedCenter = glm::vec3(0.f);
		float numOfNeigh1 = 0.f;

		glm::vec3 c = glm::vec3();

		glm::vec3 perceivedVel = glm::vec3(0.f);
		float numOfNeigh3 = 0.f;

		// - Identify which cells may contain neighbors. This isn't always 8.
		for (int x = imax((int)posInGrid.x - 1, 0); x <= imin((int)posInGrid.x + 1, gridResolution); x++) {
			for (int y = imax((int)posInGrid.y - 1, 0); y <= imin((int)posInGrid.y + 1, gridResolution); y++) {
				for (int z = imax((int)posInGrid.z - 1, 0); z <= imin((int)posInGrid.z + 1, gridResolution); z++) {

					int currCell = gridIndex3Dto1D(x, y, z, gridResolution);

					// - For each cell, read the start/end indices in the splat pointer array.
					//   DIFFERENCE: For best results, consider what order the cells should be
					//   checked in to maximize the memory benefits of reordering the splats data.
					int startIdx = gridCellStartIndices[currCell];
					int endIdx = gridCellEndIndices[currCell];

					for (int b = startIdx; b <= endIdx; b++) {
						if (b != index) {
							float d = glm::distance(pos[b], thisPos);

							// Rule 1: splats fly towards their local perceived center of mass, which excludes themselves
							if (d < rule1Distance) {
								perceivedCenter += pos[b];
								numOfNeigh1++;
							}

							// Rule 2: splats try to stay a distance d away from each other
							if (d < rule2Distance) {
								c -= (pos[b] - thisPos);
							}

							// Rule 3: splats try to match the speed of surrounding splats
							if (d < rule3Distance) {
								perceivedVel += vel1[b];
								numOfNeigh3++;
							}
						}
					}
				}
			}
		}

		if (numOfNeigh1 > 0) {
			perceivedCenter /= numOfNeigh1;
			perceivedCenter = (perceivedCenter - thisPos) * rule1Scale;
		}

		c *= rule2Scale;

		if (numOfNeigh3 > 0) {
			perceivedVel /= numOfNeigh3;
			perceivedVel *= rule3Scale;
		}

		// Compute a new velocity based on pos and vel1
		glm::vec3 velChange = vel1[index] + perceivedCenter + c + perceivedVel;

		// - Clamp the speed change before putting the new speed in vel2
		float speed = glm::length(velChange);
		if (speed > maxSpeed) {
			velChange /= speed;
			velChange *= maxSpeed;
		}

		vel2[index] = velChange;
	}
}*/

// TODO - A test kernel right now, to be replaced
void Splats::simpleUpdate(float dt) {
	dim3 fullBlocksPerGrid((numObjects + blockSize - 1) / blockSize);

	kernUpdatePos << <fullBlocksPerGrid, blockSize >> > (numObjects, dt,
		dev_pos, dev_vel1, scene_scale);
	checkCUDAErrorWithLine("kernUpdatePos failed!");
}

void Splats::stepSimulationScatteredGrid(float dt) {
	// Uniform Grid Neighbor search using Thrust sort.

	// In Parallel:
	// - label each particle with its array index as well as its grid index.
	//   Use 2x width grids.
	dim3 fullBlocksPerGrid((numObjects + blockSize - 1) / blockSize);

	kernComputeIndices << <fullBlocksPerGrid, blockSize >> > (numObjects, 
		gridSideCount, gridMinimum, gridInverseCellWidth, dev_pos, 
		dev_particleArrayIndices, dev_particleGridIndices);
	checkCUDAErrorWithLine("kernComputeIndices failed!");

	// - Unstable key sort using Thrust. A stable sort isn't necessary, but you
	//   are welcome to do a performance comparison.
	dev_thrust_particleArrayIndices = thrust::device_pointer_cast(dev_particleArrayIndices);
	dev_thrust_particleGridIndices = thrust::device_pointer_cast(dev_particleGridIndices);

	thrust::sort_by_key(dev_thrust_particleGridIndices, 
		dev_thrust_particleGridIndices + numObjects, dev_thrust_particleArrayIndices);

	// - Naively unroll the loop for finding the start and end indices of each
	//   cell's data pointers in the array of splat indices
	kernResetIntBuffer << <fullBlocksPerGrid, blockSize >> > (gridCellCount, 
		dev_gridCellStartIndices, INT16_MAX);
	checkCUDAErrorWithLine("kernResetIntBuffer failed!");

	kernResetIntBuffer << <fullBlocksPerGrid, blockSize >> > (gridCellCount,
		dev_gridCellEndIndices, INT16_MAX);
	checkCUDAErrorWithLine("kernResetIntBuffer2 failed!");

	kernIdentifyCellStartEnd << <fullBlocksPerGrid, blockSize >> > (numObjects, 
		dev_particleGridIndices, dev_gridCellStartIndices, dev_gridCellEndIndices);
	checkCUDAErrorWithLine("kernIdentifyCellStartEnd failed!");

	// - Perform velocity updates using neighbor search
	/*kernUpdateVelNeighborSearchScattered << <fullBlocksPerGrid, blockSize >> > (numObjects,
		gridSideCount, gridMinimum, gridInverseCellWidth, gridCellWidth, 
		dev_gridCellStartIndices, dev_gridCellEndIndices, dev_particleArrayIndices, 
		dev_pos, dev_vel1, dev_vel2);
	checkCUDAErrorWithLine("kernUpdateVelNeighborSearchScattered failed!");*/

	// - Update positions
	kernUpdatePos << <fullBlocksPerGrid, blockSize >> > (numObjects, dt,
		dev_pos, dev_vel1, scene_scale);
	checkCUDAErrorWithLine("kernUpdatePos failed!");

	// - Ping-pong buffers as needed
	glm::vec3 *temp_vel = dev_vel1;
	dev_vel1 = dev_vel2;
	dev_vel2 = temp_vel;
}

/*void Splats::stepSimulationCoherentGrid(float dt) {
	// Uniform Grid Neighbor search using Thrust sort on cell-coherent data.

	// In Parallel:
	// - label each particle with its array index as well as its grid index.
	//   Use 2x width grids.
	dim3 fullBlocksPerGrid((numObjects + blockSize - 1) / blockSize);

	kernComputeIndices << <fullBlocksPerGrid, blockSize >> > (numObjects,
		gridSideCount, gridMinimum, gridInverseCellWidth, dev_pos,
		dev_particleArrayIndices, dev_particleGridIndices);
	checkCUDAErrorWithLine("kernComputeIndices failed!");

	// - Unstable key sort using Thrust. A stable sort isn't necessary, but you
	//   are welcome to do a performance comparison.
	dev_thrust_particleArrayIndices = thrust::device_pointer_cast(dev_particleArrayIndices);
	dev_thrust_particleGridIndices = thrust::device_pointer_cast(dev_particleGridIndices);

	thrust::sort_by_key(dev_thrust_particleGridIndices,
		dev_thrust_particleGridIndices + numObjects, dev_thrust_particleArrayIndices);

	// - Naively unroll the loop for finding the start and end indices of each
	//   cell's data pointers in the array of splat indices
	kernResetIntBuffer << <fullBlocksPerGrid, blockSize >> > (gridCellCount,
		dev_gridCellStartIndices, INT16_MAX);
	checkCUDAErrorWithLine("kernResetIntBuffer failed!");

	kernResetIntBuffer << <fullBlocksPerGrid, blockSize >> > (gridCellCount,
		dev_gridCellEndIndices, INT16_MAX);
	checkCUDAErrorWithLine("kernResetIntBuffer2 failed!");

	kernIdentifyCellStartEnd << <fullBlocksPerGrid, blockSize >> > (numObjects,
		dev_particleGridIndices, dev_gridCellStartIndices, dev_gridCellEndIndices);
	checkCUDAErrorWithLine("kernIdentifyCellStartEnd failed!");

	// - BIG DIFFERENCE: use the rearranged array index buffer to reshuffle all
	//   the particle data in the simulation array.
	//   CONSIDER WHAT ADDITIONAL BUFFERS YOU NEED
	kernComputeCoherentIndices << <fullBlocksPerGrid, blockSize >> > (numObjects,
		dev_positionThrust, dev_velocityThrust, dev_pos, dev_vel1, dev_particleArrayIndices);
	checkCUDAErrorWithLine("kernComputeCoherentIndices failed!");

	// Ping pong pos
	glm::vec3 *temp_pos = dev_positionThrust;
	dev_positionThrust = dev_pos;
	dev_pos = temp_pos;

	// - Perform velocity updates using neighbor search
	kernUpdateVelNeighborSearchCoherent << <fullBlocksPerGrid, blockSize >> > (numObjects,
		gridSideCount, gridMinimum, gridInverseCellWidth, gridCellWidth,
		dev_gridCellStartIndices, dev_gridCellEndIndices, dev_positionThrust, 
		dev_velocityThrust, dev_vel2);
	checkCUDAErrorWithLine("kernUpdateVelNeighborSearchCoherent failed!");

	// - Update positions
	kernUpdatePos << <fullBlocksPerGrid, blockSize >> > (numObjects, dt,
		dev_pos, dev_vel1, scene_scale);
	checkCUDAErrorWithLine("kernUpdatePos failed!");

	// - Ping-pong buffers as needed. THIS MAY BE DIFFERENT FROM BEFORE.
	glm::vec3 *temp_vel = dev_vel1;
	dev_vel1 = dev_vel2;
	dev_vel2 = temp_vel;
}*/

void Splats::endSimulation() {
	//cudaFree(dev_vel1);
	//cudaFree(dev_vel2);
	cudaFree(dev_pos);

	// TODO- Free any additional buffers here.
	///cudaFree(dev_particleArrayIndices);
	//cudaFree(dev_particleGridIndices);
	//cudaFree(dev_gridCellStartIndices);
	//cudaFree(dev_gridCellEndIndices);
	//
	//cudaFree(dev_positionThrust);
	//cudaFree(dev_velocityThrust);
}

void Splats::unitTest() {
	// test unstable sort
	int *dev_intKeys;
	int *dev_intValues;
	int N = 10;

	std::unique_ptr<int[]>intKeys{ new int[N] };
	std::unique_ptr<int[]>intValues{ new int[N] };

	intKeys[0] = 0; intValues[0] = 0;
	intKeys[1] = 1; intValues[1] = 1;
	intKeys[2] = 0; intValues[2] = 2;
	intKeys[3] = 3; intValues[3] = 3;
	intKeys[4] = 0; intValues[4] = 4;
	intKeys[5] = 2; intValues[5] = 5;
	intKeys[6] = 2; intValues[6] = 6;
	intKeys[7] = 0; intValues[7] = 7;
	intKeys[8] = 5; intValues[8] = 8;
	intKeys[9] = 6; intValues[9] = 9;

	cudaMalloc((void**)&dev_intKeys, N * sizeof(int));
	checkCUDAErrorWithLine("cudaMalloc dev_intKeys failed!");

	cudaMalloc((void**)&dev_intValues, N * sizeof(int));
	checkCUDAErrorWithLine("cudaMalloc dev_intValues failed!");

	dim3 fullBlocksPerGrid((N + blockSize - 1) / blockSize);

	std::cout << "before unstable sort: " << std::endl;
	for (int i = 0; i < N; i++) {
		std::cout << "  key: " << intKeys[i];
		std::cout << " value: " << intValues[i] << std::endl;
	}

	// How to copy data to the GPU
	cudaMemcpy(dev_intKeys, intKeys.get(), sizeof(int) * N, cudaMemcpyHostToDevice);
	cudaMemcpy(dev_intValues, intValues.get(), sizeof(int) * N, cudaMemcpyHostToDevice);

	// Wrap device vectors in thrust iterators for use with thrust.
	thrust::device_ptr<int> dev_thrust_keys(dev_intKeys);
	thrust::device_ptr<int> dev_thrust_values(dev_intValues);
	// Example for using thrust::sort_by_key
	thrust::sort_by_key(dev_thrust_keys, dev_thrust_keys + N, dev_thrust_values);

	// How to copy data back to the CPU side from the GPU
	cudaMemcpy(intKeys.get(), dev_intKeys, sizeof(int) * N, cudaMemcpyDeviceToHost);
	cudaMemcpy(intValues.get(), dev_intValues, sizeof(int) * N, cudaMemcpyDeviceToHost);
	checkCUDAErrorWithLine("memcpy back failed!");

	std::cout << "after unstable sort: " << std::endl;
	for (int i = 0; i < N; i++) {
		std::cout << "  key: " << intKeys[i];
		std::cout << " value: " << intValues[i] << std::endl;
	}

	// cleanup
	cudaFree(dev_intKeys);
	cudaFree(dev_intValues);
	checkCUDAErrorWithLine("cudaFree failed!");
	return;
}
