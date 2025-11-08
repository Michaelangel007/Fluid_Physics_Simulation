#pragma once
#include <GL/glew.h>
#include<GLFW/glfw3.h>
#include<iostream>
#include<vector>

class Window {
public:
	GLFWwindow* win;
	int width;
	int height;
	float aspectRatio = 1.0f; 
	static unsigned int vbo;
	static unsigned int vao;
	static std::vector<float> recData;
	Window(int w, int h);
	static void drawBoundary(int object_Location, int color_Location);
};