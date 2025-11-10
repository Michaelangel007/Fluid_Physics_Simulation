#define _CRT_SECURE_NO_WARNINGS
#include"../HeaderFiles/Shaders.h"

struct shaderProgramSource
{
    std::string vertexSource;
    std::string fragmentSource;
};

    Shader::shaderProgramSource Shader::parse(const std::string filePath) {
    std::ifstream stream(filePath);
    if (stream.fail())
    {
#if USE_CPP_IOSTREAM
        std::cout
            << "ERROR: Failed to open shader: " << filePath.c_str() << std::endl
            << "WARN: Falling back to default shader."              << std::endl;
#else
        printf( "ERROR: Failed to open shader: %s\n", filePath.c_str() );
        printf( "WARN: Falling back to default shader.\n" );
#endif
        std::string fragmentText(
"#version 330 core\n"
"layout (location = 0) out vec4 color;\n"
"uniform vec3 u_Color;\n"
"void main ()\n"
"{\n"
"    color = vec4(u_Color, 1.0f);\n"
"};"
        );
        std::string vertexText(
"#version 330 core\n"
"layout (location = 0) in vec4 position;\n"
"uniform vec4 u_pos;\n"
"void main ()\n"
"{\n"
"    vec4 pos = position;\n"
"    pos += u_pos;\n"
"    gl_Position = pos;\n"
"};"
        );
        return { vertexText, fragmentText };
    }

    enum shaderType
    {
        NONE = -1,
        VERTEX = 0,
        FRAGMENT = 1,
        NUM_SHADER_TYPES
    };

    std::string line;
    std::stringstream ss[shaderType::NUM_SHADER_TYPES];
    shaderType type = shaderType::NONE;

    while (getline(stream, line)) {
        if (line.find("#shader") != std::string::npos) {
            if (line.find("vertex") != std::string::npos)
                type = shaderType::VERTEX;
            else if (line.find("fragment") != std::string::npos)
                type = shaderType::FRAGMENT;
        }
        else {
            ss[(int)type] << line << '\n';
        }
    }

    return { ss[shaderType::VERTEX].str(), ss[shaderType::FRAGMENT].str()};
}

    unsigned int Shader::compile(unsigned int type, const std::string& source) {
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int length;
        int size;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        size = length + 1;
        char* message = (char*)malloc(size * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        message[size] = '\0';

        const char *shaderType = (type == GL_VERTEX_SHADER ? "vertex" : "fragment");
        char ERROR[256];
        sprintf( ERROR, "ERROR: Failed to compile %s shader.\n", shaderType );
#if USE_CPP_IOSTREAM
        std::cout
            << ERROR;
            << message << std::endl;
#else
        printf(" %s%s\n", ERROR, message );
#endif

        free( message );
        glDeleteShader(id);
        return 0;
    }

    return id;
}

    unsigned int Shader::create(const std::string& vertexShader, const std::string& fragmentShader)
{
    unsigned int program = glCreateProgram();
    unsigned int vs = compile(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = compile(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
};