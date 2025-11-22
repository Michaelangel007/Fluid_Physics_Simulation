// 4.152ms -> 3.844 ms
// Instead of returning a complete copy of a particle we only return its index.
// 3.844 ms -> 1.329 ms
// Use a static array instead of a dynamic vector
#define USE_FIXED_NEIGHBORS_SIZE 76

//#define SHOW_MAX_OCCUPANCY_ONLY 1
//#define PROFILE_OCCUPANCY       1
//#define PROFILE_NEIGHBORS       1
#define USE_CPP_IOSTREAM        1
#define _CRT_SECURE_NO_WARNINGS 1
#define USE_OPENMP 1

#if PROFILE
    #define TRACY_ENABLE 1
    #include <tracy/Tracy.hpp>
#endif

// Already set in Solution properties > Configuration Properties > C/C++ > Preprocessor
// #define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GLM/glm.hpp>

#include <vector>

#if USE_CPP_IOSTREAM
	#include <iostream>
	#include <iomanip>
#endif

#if USE_OPENMP
    #include <omp.h>
#endif
