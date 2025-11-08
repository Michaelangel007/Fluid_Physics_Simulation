#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <string>
#include <sstream>
#include<iostream>

class Shader {
public:
    struct shaderProgramSource
    {
        std::string vertexSource;
        std::string fragmentSource;
    };
    
    static shaderProgramSource parse(const std::string filePath);
    static unsigned int compile(unsigned int type, const std::string& source);
    static unsigned int create(const std::string& vertexShader, const std::string& fragmentShader);
};

