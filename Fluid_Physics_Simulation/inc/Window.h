#pragma once
#include "../inc/Common.h"

extern const char  *APP_NAME;

// in: normalized percent t
// out: rgb color mapping
//  t     rgb
//  0.00  0.00, 0.50, 1.00
//  0.25  0.25, 0.75, 0.75
//  0.50  0.50, 1.00, 0.50
//  0.75  0.75, 0.75, 0.25
//  1.00  1.00, 0.50, 0.00
//
// Also see:
// * https://www.shadertoy.com/view/tc2SDw
inline void utilColorMapping(const float t, glm::vec3& color) {
    color.r = t;
    color.g = 1.0f - std::abs(t - 0.5f);
    color.b = 1.0f - t;
}

// clamp((vec3(2.0) - abs(4.0*vec3(t) - vec3(4,2,0))), 0., 1.);
inline void utilColorMappingHotToCold(const float t, glm::vec3& color) {
    const glm::vec3 kZERO(0.f);
    const glm::vec3 kONE (1.f);
    const glm::vec3 kTWO (2.0f);
    const glm::vec3 k420 (4.0f, 2.0f, 0.0f);

          glm::vec3 a(4.0f * t);
          glm::vec3 b(a - k420);
          glm::vec3 c(kTWO - glm::abs(b));
    color = glm::clamp( c, kZERO, kONE );
}

class Window {
public:
	GLFWwindow* win;
	int width;
	int height;
	float aspectRatio = 1.0f; 
	static unsigned int vbo;
	static unsigned int vao;
	Window(int w, int h, bool waitVSnyc = true);
	static void drawRectangle(int object_Location, int color_Location, const std::vector<float> *p_rectangleVertices = nullptr, const glm::vec3* pColor = nullptr);
};