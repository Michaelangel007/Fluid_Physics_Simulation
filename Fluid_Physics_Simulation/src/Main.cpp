#include "../HeaderFiles/Common.h"

#include "../HeaderFiles/Shaders.h"
#include "../HeaderFiles/Particle.h"
#include "../HeaderFiles/Window.h"
#include <cmath>
#include <limits> // MAX_INT

       const char  *APP_NAME     = "Fluid Physics Simulation";
static const char  *APP_VERSION  = "Version 1.1";

// Configuration
static bool   benchmark             = false;
static bool   verbose               = false;
static bool   vsync                 = true;
static int    numFirstRenderFrame   = 0;
static double numLastPhysicsSeconds = 0.0;
static int    width = 25;
static int    height = 20;

// Defining static variables 
std::vector <float> Window::recData = {
    -0.9f,  0.9f,
     0.9f,  0.9f,
     0.9f, -0.9f,
    -0.9f, -0.9f
};

std::vector <float> Particle::centers = {};

void usage()
{
    const char *HELP =
"-?              Display command line options and quit.\n"
"--help          Alias for -?.\n"
"-benchmark      Run simulation for 3 minutes (~10,800 frames @ 60fps), render first frame at frame number 7,200.\n"
"-benchfast      Run simulation for 10 seconds (~600 frames @ 60fps), render first frame at frame number 300.\n"
"-h              Specifiy grid height (rows).\n"
"-height         Alias for -h.\n"
"-render #       Don't render until specified frame number. -1 is never render. (Default 0).\n"
"-time   #.##    Run simulation for specified seconds.\n"
"-v              Verbose mode off (default).\n"
"+v              Verbose mode on.\n"
"-V              Display version and quit.\n"
"--version       Alias for -V.\n"
"-vsync          VSync off.\n"
"+vsync          VSync on (default).\n"
"-w              Specify grid width (columns).\n"
"-width          Alias for -w.\n"
    ;
#if USE_CPP_IOSTREAM
    std::cout << HELP;
#else
    printf( HELP );
#endif
}

void version()
{
#if USE_CPP_IOSTREAM
    std::cout
        << APP_NAME    << std::endl
        << APP_VERSION << std::endl;
#else
    printf( "%s\n%s\n", APP_NAME, APP_VERSION );
#endif
}

void parseCommandLine(int nArgs, const char* aArgs[])
{
    const char *pArg = nullptr;
    int         iArg = 1;

    while (iArg < nArgs)
    {
        pArg = aArgs[ iArg ];
        if (pArg[0] == '-')
        {
            if ((strcmp(pArg, "-?"    ) == 0)
            ||  (strcmp(pArg, "--help") == 0)) {
                usage();
                exit(0);
            }
            else
            if (strcmp(pArg, "-benchmark") == 0) {
                numFirstRenderFrame   = 2*60 * 60; // 2 min * 60 s/min * 60 frames/s = 7,200 frames
                numLastPhysicsSeconds = 3.0 * 60.0; // 3 min * 60 s/min = 180 seconds
                benchmark = true;
            }
            else
            if (strcmp(pArg, "-benchfast") == 0) {
                numFirstRenderFrame   = 5 * 60; // 5 s * 60 frames/s = 300 frames
                numLastPhysicsSeconds = 10.0; // 10 s
                benchmark = true;
            }
            else
            if ((strcmp(pArg, "-h"     ) == 0)
            ||  (strcmp(pArg, "-height") == 0)) {
                iArg++;
                if (iArg >= nArgs) {
                    const char *ERROR = "ERROR: Grid height not specified.\ni.e.\n    -height 25\n";
#if USE_CPP_IOSTREAM
                    std::cout << ERROR;
#else
                    printf( ERROR );
#endif
                    exit(1);
                }
                pArg = aArgs[ iArg ];
                height = atoi( pArg );
                if (height < 1)
                    height = 1;
            }
            else
            if ((strcmp(pArg, "-w"    ) == 0)
            ||  (strcmp(pArg, "-width") == 0)) {
                iArg++;
                if (iArg >= nArgs) {
                    const char *ERROR = "ERROR: Grid width not specified.\ni.e.\n    -width 20\n";
#if USE_CPP_IOSTREAM
                    std::cout << ERROR;
#else
                    printf( ERROR );
#endif
                    exit(1);
                }
                pArg = aArgs[ iArg ];
                width = atoi( pArg );
                if (width < 1)
                    width = 1;
            }
            else
            if (strcmp(pArg, "-render") == 0) {
                iArg++;
                if (iArg >= nArgs) {
                    const char *ERROR = "ERROR: First frame to render was not specified.\ni.e.\n    -render 300\n";
#if USE_CPP_IOSTREAM
                    std::cout << ERROR;
#else
                    printf( ERROR );
#endif
                    exit(1);
                }
                pArg = aArgs[ iArg ];

                numFirstRenderFrame = atoi( pArg );
                if (numFirstRenderFrame < 0)
                    numFirstRenderFrame = INT_MAX;
            }
            else
            if (strcmp(pArg, "-time") == 0) {
                iArg++;
                if (iArg >= nArgs) {
                    const char *ERROR = "ERROR: Time to run simulation was not specified.\ni.e.\n    -time 10.0\n";
#if USE_CPP_IOSTREAM
                    std::cout << ERROR;
#else
                    printf( ERROR );
#endif
                    exit(1);
                }
                pArg = aArgs[ iArg ];

                numLastPhysicsSeconds = atof( pArg );
                if (numLastPhysicsSeconds < 0.0)
                    numLastPhysicsSeconds = 0.0;
            }
            else
            if (strcmp(pArg, "-v") == 0) {
                verbose = false;
            }
            else
            if (strcmp(pArg, "-V") == 0) {
                version();
                exit(0);
            }
            else
            if (strcmp(pArg, "-vsync") == 0) {
                vsync = false;
            }
            else
            if (strcmp(pArg, "--version") == 0) {
                verbose = false;
            }
        }
        else
        if (pArg[0] == '+')
        {
            if (strcmp(pArg, "+v") == 0) {
                verbose = true;
            }
            else
            if (strcmp(pArg, "+vsync") == 0) {
                vsync = true;
            }
        }

        iArg++;
    }
}

int main(int numArgs, const char *aArgs[])
{
    g_ParticleParameters.initParticleParameters();
    parseCommandLine( numArgs, aArgs );

    Window window(1600, 1000, vsync);
    
    // Generating Buffers
    glGenVertexArrays(1, &Window::vao);
    glGenBuffers(1, &Window::vbo);

    glGenVertexArrays(1, &Particle::vao);
    glGenBuffers(1, &Particle::vbo);
    glGenBuffers(1, &Particle::ibo);

    Particle::generateGridCenters(height, width); // generate grid / random particles
    Particle::populate(window.aspectRatio); // create particles using center positions

    // creating and compiling shaders
    Shader::shaderProgramSource source = Shader::parse("res/shaders/Basic.shader");
    unsigned int shader = Shader::create(source.vertexSource, source.fragmentSource);
    glUseProgram(shader);

    // Uniforms Declaration
    GLint color_Location = glGetUniformLocation(shader, "u_Color");
    glUniform3f(color_Location, 0.2f, 0.3f, 0.8f);

    GLint object_Location = glGetUniformLocation(shader, "u_pos");
    glUniform4f(object_Location, 0.0f, 0.0f, 0.0f, 0.0f);

    assert( color_Location != -1 );
    assert( object_Location != -1 );

    /* Loop until the user closes the window */

    static double lastTime              = 0.0f;
    static double elapsed               = 0.0;
    static int    numFrame              = 0;

    char sFirstFrame[16] = "n/a";
    if (numFirstRenderFrame != INT_MAX)
        sprintf( sFirstFrame, "%d", numFirstRenderFrame );
    const size_t numParticles = Particle::particles.size();
    const size_t numCenters   = Particle::centers.size();
    const int    gridDim      = g_ParticleParameters.gridDim;

#if USE_CPP_IOSTREAM
    std::cout.precision(6);
    std::cout
        << "Configuration: (C++ iostream)" << std::endl
        << std::fixed
        << "    First Render Frame: # " <<                                         sFirstFrame           << std::endl
        << "    Last Physics Seconds: " << std::setw(7) << std::setprecision(3) << numLastPhysicsSeconds << std::endl
        << "    Particles: "            <<                                         width << " x " << height << std::endl
        << "    Total particles: "      <<                                         numParticles          << std::endl
        << "    Centers: "              <<                                         numCenters            << std::endl
        << "    Grid dimensions: "      <<                                         gridDim               << std::endl;
#else
    printf( "Configuration: (C printf)\n" );
    printf( "    First Render Frame: # %s\n", sFirstFrame );
    printf( "    Last Physics Seconds: %7.3f\n", numLastPhysicsSeconds );
    printf( "    Particles: %d x %d\n", width, height );
    printf( "    Total particles: %llu\n", numParticles );
    printf( "    Centers: %llu\n", numCenters );
    printf( "    Grid dimensions: %d\n", gridDim );
#endif

    while (!glfwWindowShouldClose(window.win))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        Window::drawBoundary(object_Location, color_Location);
        bool bDraw = (numFrame >= numFirstRenderFrame);
        Particle::updateElements(object_Location, color_Location);
        Particle::drawElements(object_Location, color_Location, bDraw, numFrame);

        //calculate fps
        numFrame++;
        double currentTime = glfwGetTime();
        double deltaTime = currentTime - lastTime;
               elapsed += deltaTime;
        lastTime = currentTime;

        if (verbose) {
#if USE_CPP_IOSTREAM
            std::cout
                << "FPS: "          << std::setw(7) << std::setprecision(3) << (1.f / deltaTime)
                << " / Frametime: " << std::setw(7) << std::setprecision(3) << deltaTime * 1000.f << "ms"
                << "  Frame #: "    << std::setw(7)                         << numFrame
                << "  Elapsed: "    << std::setw(7) << std::setprecision(3) << elapsed << " s"
                << std::endl;
#else
            printf( "FPS: %7.3f / Frametime: %7.3f ms  Frame #: %7d  Elapsed: %7.3f s\n", (1.f / deltaTime), deltaTime * 1000.f, numFrame, elapsed );
#endif
        }

        /* Swap front and back buffers */
        glfwSwapBuffers(window.win);

        /* Poll for and process events */
        glfwPollEvents();
        if (numLastPhysicsSeconds > 0.0 && (elapsed >= numLastPhysicsSeconds)) break;
    }

    if (elapsed < 1e-6)
        elapsed = 1e-6; // Alt.: std::numeric_limits<float>::infinity();

    double frames  = (double)numFrame; // frames
    double avgFPS  = frames / elapsed; // frames/second
    double avgFTms = (1.0 / avgFPS) * 1000.0; // ms
#if USE_CPP_IOSTREAM
    std::cout
        <<   "Total Frames: "  <<                                         numFrame << " "
        << "/ Total Elapsed: " << std::setw(7) << std::setprecision(3) << elapsed << " s "
        << "= Avg FPS: "       << std::setw(7) << std::setprecision(3) << avgFPS
        << ", Avg Frametime: " << std::setw(7) << std::setprecision(3) << avgFTms << " ms"
        << std::endl;
#else
    printf( "Total Frames: %d / Total Elapsed: %7.3f s = Avg FPS: %7.3f, Avg Frametime: %7.3f ms \n", numFrame, elapsed , avgFPS, avgFTms );

    #if PROFILE_NEIGHBORS
        printf( "Max neighbors: %d\n", g_nMaxNeighbors );
    #endif

#endif

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

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