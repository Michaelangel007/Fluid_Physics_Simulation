#pragma once
#include "../HeaderFiles/Common.h"

extern const char  *APP_NAME;

class Window {
public:
	GLFWwindow* win;
	int width;
	int height;
	float aspectRatio = 1.0f; 
	static unsigned int vbo;
	static unsigned int vao;
	static std::vector<float> recData;
	Window(int w, int h, bool waitVSnyc = true);
	static void drawBoundary(int object_Location, int color_Location);
};