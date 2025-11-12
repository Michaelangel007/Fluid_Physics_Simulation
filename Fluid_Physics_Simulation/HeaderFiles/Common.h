// 4.152ms -> 3.844 ms
// Instead of returning a complete copy of a particle we only return its index.
#define USE_NEIGHBORS_INDEX 1
#if USE_NEIGHBORS_INDEX
	// 3.844 ms -> 1.329 ms
	// If USE_NEIGHBORS_INDEX is enabled we can further use a static array instead of a dynamic vector
	#define USE_FIXED_NEIGHBORS_SIZE 64
#endif

//#define PROFILE_NEIGHBORS 1
#define USE_CPP_IOSTREAM 1
#define _CRT_SECURE_NO_WARNINGS

// Already set in Solution properties > Configuration Properties > C/C++ > Preprocessor
// #define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>

#if USE_CPP_IOSTREAM
	#include <iostream>
	#include <iomanip>
#endif
