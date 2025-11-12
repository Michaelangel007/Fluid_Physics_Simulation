#pragma once
#include "../HeaderFiles/Common.h"

#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>
#include <stdint.h>
#include <unordered_map>
#include "../HeaderFiles/Window.h"

#if PROFILE_NEIGHBORS
	extern int g_nMaxNeighbors;
#endif

struct ParticleParameters
{
	int   numOfParticles;
	int   numSegments;

	int   gridDim;
	float gridRadius;
	float ooGridRadius;

	float nearPressureMultiplier;
	float pressureMultiplier;
	float radius;
	float spacing;
	float stepSize;
	float targetDensity;
	float viscosityMultiplier;

	float GRAVITY_MAGNITUDE;
	float MAX_SPEED;

	void initParticleParameters()
	{
		GRAVITY_MAGNITUDE = 200.0f;
		MAX_SPEED = 15.0f;

		spacing = 0.005f;
		stepSize = 0.0005f;
		numOfParticles = 2000;
		radius = 0.008f;
		targetDensity = 400.0f;
		pressureMultiplier = 200.0f;
		nearPressureMultiplier = 1000.0f;
		viscosityMultiplier = 0.0002f;

		numSegments = 16;

		gridRadius = 0.05f;
		ooGridRadius = 1.0f / gridRadius;
		gridDim    = (int)(2.0f / gridRadius); // (2.0f / 0.05f) -> 40
	}
};
extern ParticleParameters g_ParticleParameters;

typedef std::unordered_map<int, bool> GridOccupancy;
typedef std::vector <GridOccupancy>   GridCol;

class Particle
{
public:
	static std::vector <float> positions;
	static std::vector <unsigned int> indices;
	static std::vector <float> centers;
	static std::vector <Particle> particles;
	static std::vector <GridCol> cells;

	glm::vec3 pos;
	glm::vec3 predictedPos;
	
	glm::vec3 velocity;
	glm::vec3 acceleration;
	float density;
	float nearDensity;

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
	static void calculateDensities(int idx);
	static float densityKernel(float dst);
	static float nearDensityKernel(float dst);
	static float pressureKernel(float dst);
	static float nearPressureKernel(float dst);
	static float viscosityKernel(float dst);
	static void updateElements(int object_Location, int color_Location);
	static void drawElements(int object_Location, int color_Location, bool bDraw, int frame);
};
