
#include "../HeaderFiles/Shaders.h"
#include "../HeaderFiles/Particle.h"
#include "../HeaderFiles/Window.h"
#include<cmath>

// Defining static variables 
std::vector <float> Window::recData = {
    -0.9f,  0.9f,
     0.9f,  0.9f,
     0.9f, -0.9f,
    -0.9f, -0.9f
};

std::vector <float> Particle::centers = {};

int Particle::segments = 16;
float Particle::spacing = 0.005f;
float Particle::stepSize = 0.0005f;
int Particle::numOfParticles = 2000;
float Particle::radius = 0.008f;
float Particle::s_Radius = 0.05f;
float Particle::targetDensity = 400.0f;
float Particle::pressureMultiplier = 200.0f;
float Particle::nearPressureMultiplier = 1000.0f;
float Particle::viscosityMultiplier = 0.0002f;

int main(void)
{
    Window window(1600, 1000);
    
    // Generating Buffers
    glGenVertexArrays(1, &Window::vao);
    glGenBuffers(1, &Window::vbo);

    glGenVertexArrays(1, &Particle::vao);
    glGenBuffers(1, &Particle::vbo);
    glGenBuffers(1, &Particle::ibo);

    Particle::generateGridCenters(20, 25); // generate grid / random particles
    Particle::populate(window.aspectRatio); // create particles using center positions

    // creating and compiling shaders
    Shader::shaderProgramSource source = Shader::parse("res/shaders/Basic.shader");
    unsigned int shader = Shader::create(source.vertexSource, source.fragmentSource);
    glUseProgram(shader);

    // Uniforms Declaration
    int color_Location = glGetUniformLocation(shader, "u_Color");
    glUniform3f(color_Location, 0.2f, 0.3f, 0.8f);

    int object_Location = glGetUniformLocation(shader, "u_pos");
    glUniform4f(object_Location, 0.0f, 0.0f, 0.0f, 0.0f);

    /* Loop until the user closes the window */

    float lastTime = 0.0f;

    while (!glfwWindowShouldClose(window.win))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        Window::drawBoundary(object_Location, color_Location);

        Particle::drawElements(window, object_Location, color_Location);

        //calculate fps
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        std::cout << "FPS: " << (1 / deltaTime) << "/" << deltaTime << std::endl;

        /* Swap front and back buffers */
        glfwSwapBuffers(window.win);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glDeleteProgram(shader);

    glfwTerminate();
    return 0;
}




/*  TODO:
    1.Particle and Bounding box interactions
        --a) Create a bounding box--
        --b) Make visible boundries--
        c) Test using different and multiple particle positions and speeds

    2.Particle - Particle interactions
    3.Fluid mechanics:
        Density
        Viscosity
        Make use of SPH model
    4.Testing fluid behaviour to different objects and containers
    5.Fluid interactions with user inputs
    6.Add color gradients to make fluid presentable
    7.Final touches and bug testing and fixing
    8.Upload to github and plan for future developments...
*/