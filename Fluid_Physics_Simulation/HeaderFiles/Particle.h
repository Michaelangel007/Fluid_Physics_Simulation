#pragma once
#include "../HeaderFiles/Common.h"

#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>
#include <stdint.h>
#include <unordered_map>
#include "../HeaderFiles/Window.h"

#define M_PI 3.1415926535897932384626433832f

#if PROFILE_NEIGHBORS
	extern int g_nMaxNeighbors;
#endif

struct ParticleParameters
{
	int   numOfParticles; // ONLY if generating particles with random centers
	int   numSegments;

	int   gridDim;
	float gridRadius;
	float ooGridRadius;

	glm::vec3           vCollisionMinMax;
	std::vector <float> vWorldVertices;

	float farDensityScale;  //   4.0f / (M_PI * std::powf(gridRadius, 8.0f));
	float farPressureScale; // -30.0f / (M_PI * std::powf(gridRadius, 5.0f));
	float viscosityScale;   //  40.0f / (M_PI * std::powf(gridRadius, 5.0f));

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

		const float nMin = -0.9f;
		const float nMax = +0.9f;

		// World Border
		const float aWorld[] = {
			nMin, nMax, // Top Left
			nMax, nMax, // Top Right
			nMax, nMin, // Bot Right
			nMin, nMin  // Bot Left
		};
		const size_t nWorld = sizeof( aWorld ) / sizeof(aWorld[0]);
		std::vector <float> WorldVertices(aWorld, aWorld + nWorld);
		vWorldVertices = WorldVertices;

		// Collision Border inset by particle radius
		vCollisionMinMax = glm::vec3( nMin + radius, nMax - radius, 0.0f );

		gridRadius   = 0.05f;
		ooGridRadius = 1.0f / gridRadius;
		gridDim      = (int)(2.0f / gridRadius); // (2.0f / 0.05f) -> 40
		assert(("Grid Radius not larger then Particle Radius", gridRadius > 5.0f * radius));

		farDensityScale  =   4.0f / (M_PI * std::powf(gridRadius, 8.0f));
		farPressureScale = -30.0f / (M_PI * std::powf(gridRadius, 5.0f));
		viscosityScale   =  40.0f / (M_PI * std::powf(gridRadius, 5.0f));
	}
};
extern ParticleParameters g_ParticleParameters;

inline void utilPositionToGridXY(const glm::vec3 pos, int& x, int& y)
{
    const glm::vec3 translate(1.0f, 1.0f, 0.0f);
    const float ooGridRadius = g_ParticleParameters.ooGridRadius;

    glm::vec3 cellPos = pos;
    cellPos += translate;
    cellPos *= ooGridRadius;
    x = (int)cellPos.x;
    y = (int)cellPos.y;

    assert( ("Particle Out-of-Bounds in Spatial Partitioning: left"  , x >= 0) );
    assert( ("Particle Out-of-Bounds in Spatial Partitioning: bottom", y >= 0) );
    assert( ("Particle Out-of-Bounds in Spatial Partitioning: right" , x < g_ParticleParameters.gridDim) );
    assert( ("Particle Out-of-Bounds in Spatial Partitioning: top"   , y < g_ParticleParameters.gridDim) );
}

typedef std::unordered_map<int, bool> GridOccupancy;
typedef std::vector <GridOccupancy>   GridCol;

#if USE_NEIGHBORS_INDEX
	#if USE_FIXED_NEIGHBORS_SIZE
		struct Neighbors
		{
			Neighbors()
			: arraySize(0)
			{}
			const size_t size() const    { return arraySize; }
			void push_back(uint16_t val) { arrayData[ arraySize++ ] = val; assert(arraySize <= USE_FIXED_NEIGHBORS_SIZE); }

			      uint16_t  operator[](const int index)       { return arrayData[index]; }
			const uint16_t& operator[](const int index) const { return arrayData[index]; }

			size_t  arraySize;
			int16_t arrayData[ USE_FIXED_NEIGHBORS_SIZE ];
		};
	#else
		typedef std::vector<int16_t> Neighbors;
	#endif
#else
	typedef std::vector<Particle> Neighbors;
#endif

class Particle
{
public:
	static std::vector <float>        positions;
	static std::vector <unsigned int> indices;
	static std::vector <float>        centersX;
	static std::vector <float>        centersY;
	static std::vector <Particle>     particles;
	static std::vector <GridCol>      cells;

	glm::vec3 pos;
	glm::vec3 predictedPos;
	
	glm::vec3 velocity;
	glm::vec3 acceleration;
	float density;
	float nearDensity;

	static unsigned int vao;
	static unsigned int vbo;
	static unsigned int ibo;

	static glm::vec3 calculatePressure(int idx);
	static glm::vec3 calculateViscosity(int idx, Neighbors neighbors);
	static void drawParticles(int object_Location, int color_Location);
	static void generateGridCenters(int rows, int cols);
	       void generateParticle(float aspectRatio);
	static void generateRandomCenters();
	static float kernelFarDensity(float dst);
	static float kernelFarPressure(float dst);
	static float kernelNearDensity(float dst);
	static float kernelNearPressure(float dst);
	static float kernelViscosity(float dst);
	static void populate(float aspectRatio);
	static void reset(float aspectRatio);
	       void updateBoundary();
	static void updateDensities(int idx);
	static void updateParticles();

	// Spatial Partitioning
	static Neighbors findNeighbors(int idx);
	static void updateCell(int idx, int prevRow, int prevCol);
};
