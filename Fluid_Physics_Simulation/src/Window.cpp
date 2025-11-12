#include "../HeaderFiles/Window.h"

//Defining static members
GLuint Window::vao = 0;
unsigned int Window::vbo = 0;

Window :: Window(int w, int h, bool waitVSync) {
    glfwInit();

    GLFWwindow* window;
    width = w;
    height = h;
    aspectRatio = (float)width / (float)height;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(width, height, APP_NAME, NULL, NULL);
    if (!window)
    {
        glfwTerminate();
    }
    win = window;

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    if (waitVSync)
        glfwSwapInterval(1);
    else
        glfwSwapInterval(0);

    const char *vendor     = (const char*) glGetString( GL_VENDOR  );
    const char *version    = (const char*) glGetString( GL_VERSION );
    const char *ERROR = "ERROR: Couldn't init glew.\n";
    bool glewFail = glewInit() != GLEW_OK;
    {
#if USE_CPP_IOSTREAM
        if (glewFail)
            std::cerr << ERROR;
        else
            std::cout
                << "OpenGL Version: " << version << std::endl
                << "OpenGL Vendor: "  << vendor  << std::endl;
#else
        if (glewFail)
            fprintf( stderr, ERROR );
        else
            printf( "OpenGL Version: %s\nOpenGL Vendor: %s\n", version, vendor );
#endif
    }

}

void Window::drawBoundary(int object_Location, int color_Location) {
    const int VERTICES = 4;
    const int COMPONENTS = 2; // x, y

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, VERTICES * COMPONENTS * sizeof(float), recData.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, COMPONENTS, GL_FLOAT, GL_FALSE, COMPONENTS * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glUniform4f(object_Location, 0.0f, 0.0f, 0.0f, 0.0f);
    glUniform3f(color_Location, 1.0f, 1.0f, 1.0f);

    glDrawArrays(GL_LINE_LOOP, 0, VERTICES);
}
