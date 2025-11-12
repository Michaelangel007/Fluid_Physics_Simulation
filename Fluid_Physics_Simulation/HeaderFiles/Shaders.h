#pragma once
#include "../HeaderFiles/Common.h"

#include <fstream>
#include <string>
#include <sstream>

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

