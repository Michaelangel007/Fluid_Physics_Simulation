#pragma once
#include "../HeaderFiles/Common.h"

#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>
#include <stdint.h>
#include <unordered_map>
#include "../HeaderFiles/Window.h"

class Particle
{
public:
	static std::vector <float> positions;
	static std::vector <unsigned int> indices;
	static std::vector <float> centers;
	static std::vector <Particle> particles;
	static std::vector <std::vector<std::unordered_map<int, bool>>> cells;

	glm::vec3 pos;
	glm::vec3 predictedPos;
	
	glm::vec3 velocity;
	glm::vec3 acceleration;
	float density;
	float nearDensity;

	static int numOfParticles;
	static int segments;
	static float radius;
	static float s_Radius;
	static float targetDensity;
	static float pressureMultiplier;
	static float nearPressureMultiplier;
	static float viscosityMultiplier;
	static float stepSize;
	static float spacing;

	static unsigned int vao;
	static unsigned int vbo;
	static unsigned int ibo;

	static void generateRandomCenters();
	static void generateGridCenters(int rows, int cols);
	static void populate(float aspectRatio);
	static void updateCell(int idx, int prevRow, int prevCol);
	static std::vector<Particle> findNeighbors(int idx);
	void generateParticle(float aspectRatio);
	static glm::vec3 pressure(int idx);
	static glm::vec3 viscosity(int idx, std::vector<Particle> neighbors);
	static void calcuateDensities(int idx);
	static float densityKernel(float dst);
	static float nearDensityKernel(float dst);
	static float pressureKernel(float dst);
	static float nearPressureKernel(float dst);
	static float viscosityKernel(float dst);
//	static void drawElements(Window window, int object_Location, int color_Location);
	static void updateElements(int object_Location, int color_Location);
	static void drawElements(int object_Location, int color_Location, bool bDraw, int frame);
};
